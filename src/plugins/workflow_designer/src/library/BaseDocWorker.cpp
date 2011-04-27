/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Log.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentUtils.h>
#include <U2Misc/DialogUtils.h>

namespace U2 {
namespace LocalWorkflow {

/**********************************
 * BaseDocReader
 **********************************/
BaseDocReader::BaseDocReader(Actor* a, const QString& tid, const DocumentFormatId& fid) : BaseWorker(a), ch(NULL), fid(fid), 
done(false), attachDoc2Proj(false) {
    mtype = WorkflowEnv::getDataTypeRegistry()->getById(tid);
}

void BaseDocReader::init() {
    QStringList urls = WorkflowUtils::expandToUrls(actor->getParameter(BaseAttributes::URL_IN_ATTRIBUTE().getId())->getAttributeValue<QString>());
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
            assert(format);
            IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url));
            doc = new Document(format, iof, url);
        }
        //TODO lock document???
        docs.insert(doc, newDoc);
    }

    assert(ports.size() == 1);
    ch = ports.values().first();
}

bool BaseDocReader::isReady() {
    return !isDone();
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
        done = true;
        ch->setEnded();
    }
    return NULL;
}

bool BaseDocReader::isDone() {
    return done && cache.isEmpty();
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
: BaseWorker(a), ch(NULL), format(NULL), done(false), append(false), fileMode(SaveDoc_Roll) 
{
    format = AppContext::getDocumentFormatRegistry()->getFormatById(fid);
}

BaseDocWriter::BaseDocWriter( Actor * a ) 
: BaseWorker(a), ch(NULL), format(NULL), done(false), append(false), fileMode(SaveDoc_Roll) 
{
}

bool BaseDocWriter::isDone() {
    return done;
}

void BaseDocWriter::cleanup() {
}

void BaseDocWriter::init() {
    assert(ports.size() == 1);
    ch = ports.values().first();
}

bool BaseDocWriter::isReady() {
    int hasMsg = ch->hasMessage();
    bool ended = ch->isEnded();
    return hasMsg || (ended && !done);
}

#define GZIP_SUFFIX ".gz"

Task* BaseDocWriter::tick() {
    while(ch->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(ch);
        
        Attribute * formatAttr = actor->getParameter(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
        if( formatAttr != NULL ) { // user sets format
            format = AppContext::getDocumentFormatRegistry()->getFormatById( formatAttr->getAttributeValue<QString>() );
        }
        url = actor->getParameter(BaseAttributes::URL_OUT_ATTRIBUTE().getId())->getAttributeValue<QString>();
        fileMode = actor->getParameter(BaseAttributes::FILE_MODE_ATTRIBUTE().getId())->getAttributeValue<uint>();
        fileMode |= SaveDoc_DestroyAfter;
        Attribute* a = actor->getParameter(BaseAttributes::ACCUMULATE_OBJS_ATTRIBUTE().getId());
        if(a != NULL) {
            append = a->getAttributeValue<bool>();
        }
        QVariantMap data = inputMessage.getData().toMap();
        
        if(format == NULL) {
            return new FailTask(tr("Document format not set"));
        }
        
        Document* doc = NULL;
        QString anUrl = url;
        
        if (anUrl.isEmpty()) {
            anUrl = data.value(BaseSlots::URL_SLOT().getId()).toString();
        }
        
        if (anUrl.isEmpty()) {
            QString err = tr("Unspecified URL to write %1").arg(format->getFormatName());
            return new FailTask(err);
        }
        
        // set correct file extension
        GUrl path(anUrl);
        QStringList suffixList = format->getSupportedDocumentFileExtensions();
        QString suffix = /*path.completeFileSuffix();*/ path.lastFileSuffix();
        QString newSuffix = suffixList.first();
        if (suffix.contains("gz")) {
            newSuffix.append(GZIP_SUFFIX);
            suffix = path.completeFileSuffix();
            suffix.remove(GZIP_SUFFIX);
        }
        if (!suffixList.contains(suffix)) {
            path = path.dirPath() + "/" + path.baseFileName() + "." + newSuffix; 
            anUrl = path.getURLString();
        } 
        

        doc = docs.value(anUrl);
        if (!doc) {
            IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url));
            int count = ++counter[anUrl];
            if(!append && count > 1) {
                anUrl = GUrlUtils::prepareFileName(anUrl, count, format->getSupportedDocumentFileExtensions());
            } else {
                assert(count == 1);
                anUrl = GUrlUtils::ensureFileExt(anUrl, format->getSupportedDocumentFileExtensions()).getURLString();
            }
            doc = new Document(format, iof, anUrl);
            doc->setLoaded(true);
            docs.insert(anUrl, doc);
        }
        data2doc(doc, data);
        if (!append) {
            break;
        }
    }
    
    done = ch->isEnded();
    if (append && !done) {
        return NULL;
    }
    return processDocs();
}

Task* BaseDocWriter::processDocs()
{
    if(docs.isEmpty()) {
        coreLog.error( "nothing to write: no documents" );
        return NULL;
    }
    QList<Task*> tlist;
    QMapIterator<QString, Document*> it(docs);
    while (it.hasNext())
    {
        it.next();
        Document* doc = it.value();
        QString anUrl = it.key();
        //int count = ++counter[anUrl];
        /*if (!append && count != 1) {
            anUrl = GUrlUtils::prepareFileName(anUrl, count, format->getSupportedDocumentFileExtensions());
        } else {
            assert(count == 1);
            anUrl = GUrlUtils::ensureFileExt(anUrl, format->getSupportedDocumentFileExtensions()).getURLString();
        }*/
        //doc->setURL(anUrl);
        ioLog.details(tr("Writing to %1 [%2]").arg(anUrl).arg(format->getFormatName()));
        tlist << new SaveDocumentTask(doc, SaveDocFlags(fileMode), DocumentUtils::getNewDocFileNameExcludesHint());
    }
    docs.clear();

    return tlist.size() == 1 ? tlist.first() : new MultiTask(tr("Save documents"), tlist);
}

} // Workflow namespace
} // U2 namespace
