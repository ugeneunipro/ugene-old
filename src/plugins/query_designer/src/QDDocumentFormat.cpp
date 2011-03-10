#include "QDDocumentFormat.h"
#include "QDSceneIOTasks.h"
#include "QDDocument.h"
#include "QueryViewController.h"

#include <U2Core/IOAdapter.h>
#include <U2Core/AppContext.h>
#include <U2Core/SelectionUtils.h>


namespace U2 {

 //QDGobject
 //////////////////////////////////////////////////////////////////////////
const GObjectType QDGObject::TYPE("query-obj");

GObject* QDGObject::clone() const {
    QDGObject* copy = new QDGObject(getGObjectName(), serializedScene, getGHintsMap());
    return copy;
}

const DocumentFormatId QDDocFormat::FORMAT_ID = "QueryDocFormat";

//Format
//////////////////////////////////////////////////////////////////////////
QDDocFormat::QDDocFormat(QObject* p)
: DocumentFormat(p, DocumentFormatFlags_W1, QStringList(QUERY_SCHEME_EXTENSION)),
formatName(tr("Query Schema")) {
    supportedObjectTypes += QDGObject::TYPE;
}

Document* QDDocFormat::createNewDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs /*= QVariantMap()*/ ) {
    Document* d = DocumentFormat::createNewDocument(io, url, fs);
    GObject* o = new QDGObject(tr("Query Schema"), "");
    d->addObject(o);
    return d;
}

#define BUFF_SIZE 1024
Document* QDDocFormat::loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode /*= DocumentLoadMode_Whole*/ ) {
    Q_UNUSED(mode);
    QByteArray  rawData;
    QByteArray block(BUFF_SIZE, '\0');
    int blockLen = 0;
    while ((blockLen = io->readBlock(block.data(), BUFF_SIZE)) > 0) {
        rawData.append(block.data(), blockLen);
        ti.progress = io->getProgress();
    }

    if(!checkRawData(rawData)) {
        ti.setError(tr("Invalid header. %1 expected").arg(QDDocument::HEADER_LINE));
        rawData.clear();
    }

    QList<GObject*> objects;
    QString data = QString::fromUtf8(rawData.data(), rawData.size());
    objects.append(new QDGObject(tr("Query Schema"), data));
    return new Document(this, io->getFactory(), io->getURL(), objects, fs);
}

void QDDocFormat::storeDocument(Document* d, TaskStateInfo& ts, IOAdapter* io) {
    Q_UNUSED(ts)
    assert(d->getDocumentFormat() == this);
    assert(d->getObjects().size() ==1);

    QDGObject* wo = qobject_cast<QDGObject*>(d->getObjects().first());
    assert(wo && wo->getScene());

    QByteArray rawData = QDSceneSerializer::scene2doc(wo->getScene())->toByteArray();
    int nWritten = 0;
    int nTotal = rawData.size();
    while(nWritten < nTotal) {
        int d = io->writeBlock(rawData.data() + nWritten, nTotal - nWritten);
        assert(d > 0);
        nWritten+= d;
    }
    wo->setSceneRawData(rawData);
}

FormatDetectionResult QDDocFormat::checkRawData( const QByteArray& rawData, const GUrl&) const {
    const QString& data = rawData;
    if(data.trimmed().startsWith(QDDocument::HEADER_LINE)) {
        return FormatDetection_Matched;
    }
    return FormatDetection_NotMatched;
}

//Factory
//////////////////////////////////////////////////////////////////////////
const GObjectViewFactoryId QDViewFactory::ID("query-view-factory");

bool QDViewFactory::canCreateView( const MultiGSelection& multiSelection ) {
    foreach(GObject* go, SelectionUtils::findObjects(QDGObject::TYPE, &multiSelection, UOF_LoadedOnly)) {
        if (!qobject_cast<QDGObject*>(go)->getScene()) {
            return true;
        }
    }
    return false;
}

Task* QDViewFactory::createViewTask( const MultiGSelection& multiSelection, bool single /*= false*/ ) {
    QSet<Document*> documents = SelectionUtils::findDocumentsWithObjects(QDGObject::TYPE, &multiSelection, UOF_LoadedAndUnloaded, true);
    if (documents.size() == 0) {
        return NULL;
    }
    Task* result = (single || documents.size() == 1) ? NULL : new Task(tr("Open multiple views"), TaskFlag_NoRun);
    foreach(Document* d, documents) {
        Task* t = new OpenQDViewTask(d);
        if (result == NULL) {
            return t;
        } 
        result->addSubTask(t);
    }
    return result;
}

//OpenViewTask
//////////////////////////////////////////////////////////////////////////
OpenQDViewTask::OpenQDViewTask( Document* doc ) : ObjectViewTask(QDViewFactory::ID) {
    if (!doc->isLoaded()) {
        documentsToLoad.append(doc);
    } else {
        foreach(GObject* go, doc->findGObjectByType(QDGObject::TYPE)) {
            selectedObjects.append(go) ;
        }
        assert(!selectedObjects.isEmpty());
    }
}

void OpenQDViewTask::open() {
    if (stateInfo.hasErrors()) {
        return;
    }
    if (!documentsToLoad.isEmpty()) {
        foreach(GObject* go, documentsToLoad.first()->findGObjectByType(QDGObject::TYPE)) {
            selectedObjects.append(go);
        }
    }
    foreach(QPointer<GObject> po, selectedObjects) {
        QDGObject* o = qobject_cast<QDGObject*>(po);
        assert(o && !o->getScene());
        QueryViewController* view = new QueryViewController;
        view->loadScene(o->getSceneRawData());
        AppContext::getMainWindow()->getMDIManager()->addMDIWindow(view);
        AppContext::getMainWindow()->getMDIManager()->activateWindow(view);
    }
}
}//namespace
