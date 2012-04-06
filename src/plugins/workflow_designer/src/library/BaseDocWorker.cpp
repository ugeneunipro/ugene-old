/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "BaseDocWorker.h"

#include "CoreLib.h"
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/FailTask.h>
#include <U2Core/MultiTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2DbiRegistry.h>

#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Log.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Gui/DialogUtils.h>

namespace U2 {
namespace LocalWorkflow {

/**********************************
 * BaseDocReader
 **********************************/
BaseDocReader::BaseDocReader(Actor* a, const QString& tid, const DocumentFormatId& fid) : BaseWorker(a), ch(NULL), fid(fid), 
attachDoc2Proj(false) {
    mtype = WorkflowEnv::getDataTypeRegistry()->getById(tid);
}

void BaseDocReader::init() {
    QStringList urls = WorkflowUtils::expandToUrls(actor->getParameter(BaseAttributes::URL_IN_ATTRIBUTE().getId())->getAttributeValue<QString>(context));
    Project* p = AppContext::getProject();
    foreach(QString url, urls) {
        Document* doc = NULL;
        bool newDoc = true;
        if (p) {
            doc = p->findDocumentByURL(url);
            if (doc && doc->getDocumentFormatId() == fid) {
                newDoc = false;
            } else {
                doc = NULL;
            }
        }
        if (!doc) {
            DocumentFormat* format = AppContext::getDocumentFormatRegistry()->getFormatById(fid);
            IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
            U2OpStatus2Log os;
            doc = format->createNewUnloadedDocument(iof, url, os);
        }
        //TODO lock document???
        docs.insert(doc, newDoc);
    }

    assert(ports.size() == 1);
    ch = ports.values().first();
}

Task* BaseDocReader::tick() {
    if (!docs.isEmpty()) {
        Document* doc = docs.begin().key();
        if (!doc->isLoaded()) {
            return new LoadUnloadedDocumentTask(doc);
        } else {
            doc2data(doc);
            while (!cache.isEmpty()) {
                ch->put(cache.takeFirst());
            }
            if (docs.take(doc)) {
                doc->unload();
                delete doc;
            }
        }
    } 
    if (docs.isEmpty()) {
        setDone();
        ch->setEnded();
    }
    return NULL;
}

bool BaseDocReader::isDone() {
    return BaseWorker::isDone() && cache.isEmpty();
}

void BaseDocReader::cleanup() {
    QMapIterator<Document*, bool> it(docs);
    while (it.hasNext())
    {
        it.next();
        if (it.value()) {
            if (it.key()->isLoaded()) {
                it.key()->unload();
            }
            delete it.key();
        }
    }
}


/**********************************
* BaseDocWriter
**********************************/
BaseDocWriter::BaseDocWriter(Actor* a, const DocumentFormatId& fid) 
: BaseWorker(a), ch(NULL), format(NULL), append(false), fileMode(SaveDoc_Roll) 
{
    format = AppContext::getDocumentFormatRegistry()->getFormatById(fid);
}

BaseDocWriter::BaseDocWriter( Actor * a ) 
: BaseWorker(a), ch(NULL), format(NULL), append(false), fileMode(SaveDoc_Roll) 
{
}

void BaseDocWriter::cleanup() {
    foreach (IOAdapter *io, adapters.values()) {
        io->close();
    }
}

void BaseDocWriter::init() {
    assert(ports.size() == 1);
    ch = ports.values().first();
}

static bool openIOAdapter(IOAdapter *io, const QString &url, SaveDocFlags flags, const QSet<QString> &excludeList) {
    if (flags.testFlag(SaveDoc_Roll)) {
        TaskStateInfo ti;
        if (!GUrlUtils::renameFileWithNameRoll(url, ti, excludeList)) {
            return false;
        }
    }
    IOAdapterMode mode = IOAdapterMode_Write;
    if (flags.testFlag(SaveDoc_Append)) {
        mode = IOAdapterMode_Append;
    }
    return io->open(url, mode);
}

#define GZIP_SUFFIX ".gz"

Task* BaseDocWriter::tick() {
    while(ch->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(ch);
        { // get parameters
            Attribute * formatAttr = actor->getParameter(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
            if( formatAttr != NULL ) { // user sets format
                QString formatId = formatAttr->getAttributeValue<QString>(context);
                format = AppContext::getDocumentFormatRegistry()->getFormatById( formatId );
            }
            if(format == NULL) {
                return new FailTask(tr("Document format not set"));
            }

            Attribute * urlAttribute = actor->getParameter(BaseAttributes::URL_OUT_ATTRIBUTE().getId());
            url = urlAttribute->getAttributeValue<QString>(context);
            fileMode = actor->getParameter(BaseAttributes::FILE_MODE_ATTRIBUTE().getId())->getAttributeValue<uint>(context);
            fileMode |= SaveDoc_DestroyAfter;
            Attribute* a = actor->getParameter(BaseAttributes::ACCUMULATE_OBJS_ATTRIBUTE().getId());
            if(a != NULL) {
                append = a->getAttributeValue<bool>(context);
            }
        }

        QVariantMap data = inputMessage.getData().toMap();
        if (data.isEmpty()) {
            continue;
        }

        QString anUrl = url;
        if (anUrl.isEmpty()) {
            anUrl = data.value(BaseSlots::URL_SLOT().getId()).toString();
        }
        if (anUrl.isEmpty()) {
            QString err = tr("Unspecified URL to write %1").arg(format->getFormatName());
            return new FailTask(err);
        }

        // to avoid "c:/..." and "C:/..." on windows
        anUrl = QFileInfo(anUrl).absoluteFilePath();

        bool streaming = format->isStreamingSupport();
        { // create a new adapter or document
            bool createNewDoc = ( !append || !streaming ) && !docs.contains(anUrl);
            bool createNewAdapter = append && streaming && !adapters.contains(anUrl);

            IOAdapterFactory *iof = NULL;
            if (createNewAdapter || createNewDoc) {
                iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
                if (createNewAdapter) {
                    IOAdapter *io = iof->createIOAdapter();
                    openIOAdapter(io, anUrl, SaveDocFlags(fileMode), usedUrls);
                    ioLog.details(tr("Creating %1 [%2]").arg(io->getURL().getURLString()).arg(format->getFormatName()));
                    usedUrls.insert(io->getURL().getURLString());
                    adapters.insert(anUrl, io);
                }
            }
            if (createNewDoc) {
                U2OpStatus2Log os;
                QVariantMap hints;
                U2DbiRef dbiRef = context->getDataStorage()->getDbiRef();
                hints.insert(DocumentFormat::DBI_REF_HINT, qVariantFromValue(dbiRef));
                Document *doc = format->createNewLoadedDocument(iof, anUrl, os, hints);
                docs.insert(anUrl, doc);
            }
        }

        if (streaming && append) {
            IOAdapter *io = adapters.value(anUrl);
            storeEntry(io, data, ch->takenMessages());
        } else {
            Document *doc = docs.value(anUrl);
            data2doc(doc, data);
            if (!append) {
                break;
            }
        }
    }
    
    bool done = ch->isEnded() && !ch->hasMessage();
    if (append && !done) {
        return NULL;
    }
    if (done) {
        setDone();
    }
    return processDocs();
}

Task* BaseDocWriter::processDocs()
{
    if(docs.isEmpty() && adapters.isEmpty()) {
        coreLog.error(tr("nothing to write"));
    }
    if (docs.isEmpty()) {
        return NULL;
    }
    QList<Task*> tlist;
    QMapIterator<QString, Document*> it(docs);
    while (it.hasNext()) {
        it.next();
        Document* doc = it.value();
        QString anUrl = it.key();
        ioLog.details(tr("Writing to %1 [%2]").arg(anUrl).arg(format->getFormatName()));
        tlist << new SaveDocumentTask(doc, SaveDocFlags(fileMode), DocumentUtils::getNewDocFileNameExcludesHint());
    }
    docs.clear();

    return tlist.size() == 1 ? tlist.first() : new MultiTask(tr("Save documents"), tlist);
}

} // Workflow namespace
} // U2 namespace
