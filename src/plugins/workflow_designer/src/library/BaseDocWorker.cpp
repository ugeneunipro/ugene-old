/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "CoreLib.h"

#include <U2Gui/DialogUtils.h>

#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowUtils.h>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/FailTask.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/Log.h>
#include <U2Core/MultiTask.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "BaseDocWorker.h"

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
: BaseWorker(a), format(NULL), ch(NULL), append(true), fileMode(SaveDoc_Roll)
{
    format = AppContext::getDocumentFormatRegistry()->getFormatById(fid);
}

BaseDocWriter::BaseDocWriter(Actor *a)
: BaseWorker(a), format(NULL), ch(NULL), append(true), fileMode(SaveDoc_Roll)
{
}

void BaseDocWriter::cleanup() {
    foreach (IOAdapter *io, adapters.values()) {
        if (io->isOpen()) {
            io->close();
        }
    }
}

void BaseDocWriter::init() {
    assert(ports.size() == 1);
    ch = ports.values().first();
}

QStringList BaseDocWriter::getOutputFiles(){
    QStringList files = BaseWorker::getOutputFiles();
    QStringList urls;
    foreach (const QString &url, files) {
        urls << GUrl(url).getURLString();
    }
    foreach (const QString &url, usedUrls) {
        urls << GUrl(url).getURLString();
    }

    QSet<QString> urlSet = urls.toSet();
    return urlSet.toList();
}

#define GZIP_SUFFIX ".gz"

void BaseDocWriter::takeParameters(U2OpStatus &os) {
    Attribute *formatAttr = actor->getParameter(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
    if (NULL != formatAttr) { // user sets format
        QString formatId = formatAttr->getAttributeValue<QString>(context);
        format = AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
    }
    if (NULL == format) {
        os.setError(tr("Document format not set"));
        return;
    }

    fileMode = getValue<uint>(BaseAttributes::FILE_MODE_ATTRIBUTE().getId());
    Attribute *a = actor->getParameter(BaseAttributes::ACCUMULATE_OBJS_ATTRIBUTE().getId());
    if(NULL != a) {
        append = a->getAttributeValue<bool>(context);
    } else {
        append = true;
    }
}

QStringList BaseDocWriter::takeUrlList(const QVariantMap &data, U2OpStatus &os) {
    QString anUrl = getValue<QString>(BaseAttributes::URL_OUT_ATTRIBUTE().getId());;
    {
        if (anUrl.isEmpty()) {
            anUrl = data.value(BaseSlots::URL_SLOT().getId()).toString();
        }
        if (anUrl.isEmpty()) {
            QString err = tr("Unspecified URL to write %1").arg(format->getFormatName());
            os.setError(err);
            return QStringList();
        }

        // to avoid "c:/..." and "C:/..." on windows
        anUrl = QFileInfo(anUrl).absoluteFilePath();
    }

    QStringList urls;
    urls << anUrl;

    return urls;
}

bool BaseDocWriter::isSupportedSeveralMessages() const {
    // if the format can contain only one object then adapters must be created for each message
    if (format->checkFlags(DocumentFormatFlag_SingleObjectFormat)) {
        return false;
    }
    if (format->checkFlags(DocumentFormatFlag_OnlyOneObject)) {
        return false;
    }
    return true;
}

bool BaseDocWriter::ifCreateAdapter(const QString &url) const {
    if (!isSupportedSeveralMessages()) {
        return true;
    }

    // if not accumulate object in one file
    if (!append) {
        return true;
    }

    return (!adapters.contains(url));
}

void BaseDocWriter::openAdapter(IOAdapter *io, const QString &aUrl, const SaveDocFlags &flags, U2OpStatus &os) {
    QString url = aUrl;
    if (counters.contains(aUrl)) {
        url = GUrlUtils::insertSuffix(aUrl, "_" + QString::number(counters[aUrl]));
    }

    if (flags.testFlag(SaveDoc_Roll)) {
        TaskStateInfo ti;
        if (!GUrlUtils::renameFileWithNameRoll(url, ti, usedUrls)) {
            os.setError(ti.getError());
            return;
        }
    }
    IOAdapterMode mode = flags.testFlag(SaveDoc_Append) ? IOAdapterMode_Append : IOAdapterMode_Write;
    bool opened = io->open(url, mode);
    if (!opened) {
        os.setError(tr("Can not open a file for writing: %1").arg(url));
    }

    int old = counters.value(aUrl, 0);
    counters[aUrl] = old + 1;
}

IOAdapter * BaseDocWriter::getAdapter(const QString &url, U2OpStatus &os) {
    if (!ifCreateAdapter(url)) {
        return adapters[url];
    }

    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    QScopedPointer<IOAdapter> io(iof->createIOAdapter());
    openAdapter(io.data(), url, SaveDocFlags(fileMode), os);
    CHECK_OP(os, NULL);

    adapters[io->getURL().getURLString()] = io.data();
    usedUrls << io->getURL().getURLString();

    return io.take();
}

Document * BaseDocWriter::getDocument(IOAdapter *io, U2OpStatus &os) {
    if (docs.contains(io)) {
        return docs[io];
    }

    QVariantMap hints;
    U2DbiRef dbiRef = context->getDataStorage()->getDbiRef();
    hints.insert(DocumentFormat::DBI_REF_HINT, qVariantFromValue(dbiRef));
    Document *doc = format->createNewLoadedDocument(io->getFactory(), io->getURL(), os, hints);
    CHECK_OP(os, NULL);

    docs[io] = doc;
    return doc;
}

bool BaseDocWriter::isStreamingSupport() const {
    return format->isStreamingSupport();
}

Task * BaseDocWriter::tick() {
    U2OpStatusImpl os;
    while(ch->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(ch);
        QVariantMap data = inputMessage.getData().toMap();
        if (data.isEmpty()) {
            continue;
        }

        this->takeParameters(os);
        CHECK_OP(os, new FailTask(os.getError()));
        QStringList urls = this->takeUrlList(data, os);
        CHECK_OP(os, new FailTask(os.getError()));

        foreach (const QString &anUrl, urls) {
            IOAdapter *io = getAdapter(anUrl, os);
            CHECK_OP(os, new FailTask(os.getError()));
            if (isStreamingSupport()) {
                // TODO: make it in separate thread!
                storeEntry(io, data, ch->takenMessages());
            } else {
                Document *doc = getDocument(io, os);
                CHECK_OP(os, new FailTask(os.getError()));
                data2doc(doc, data);
            }
        }
        if (!append) {
            break;
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
    if(adapters.isEmpty()) {
        coreLog.error(tr("nothing to write"));
    }
    if (docs.isEmpty()) {
        return NULL;
    }
    QList<Task*> tlist;
    foreach (IOAdapter *io, docs.keys()) {
        Document *doc = docs[io];
        ioLog.details(tr("Writing to %1 [%2]").arg(io->getURL().getURLString()).arg(format->getFormatName()));
        io->close();
        tlist << getWriteDocTask(doc, getDocFlags());
    }
    docs.clear();

    return tlist.size() == 1 ? tlist.first() : new MultiTask(tr("Save documents"), tlist);
}

Task * BaseDocWriter::getWriteDocTask(Document *doc, const SaveDocFlags &flags) {
    return new SaveDocumentTask(doc, flags, DocumentUtils::getNewDocFileNameExcludesHint());
}

SaveDocFlags BaseDocWriter::getDocFlags() const {
    SaveDocFlags flags(fileMode);
    flags |= SaveDoc_DestroyAfter;
    if (flags.testFlag(SaveDoc_Roll)) {
        flags ^= SaveDoc_Roll;
    }
    return flags;
}

QString BaseDocWriter::getUniqueObjectName(const Document *doc, const QString &name) {
    QString result = name;
    int num = 0;
    bool found = false;
    while (NULL != doc->findGObjectByName(result)) {
        found = true;
        num++;
        result = name + QString("_%1").arg(num);
    }

    return found ? result : name;
}

} // Workflow namespace
} // U2 namespace
