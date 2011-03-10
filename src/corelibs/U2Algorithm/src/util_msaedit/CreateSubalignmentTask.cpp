#include "CreateSubalignmentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/SaveDocumentTask.h>

#include <U2Core/GObjectUtils.h>


namespace U2{

CreateSubalignmentTask::CreateSubalignmentTask(MAlignmentObject* _maObj, U2Region _window, 
                                              const QStringList& _seqNames, const GUrl& _url, 
                                                bool _saveImmediately )
: Task(tr("Extract selected as MSA task"), TaskFlags_NR_FOSCOE),
saveToAnother(true),
maObj(_maObj), window(_window), seqNames(_seqNames), url(_url), 
saveImmediately(_saveImmediately)
{
    curDoc = maObj->getDocument();
    if(url == curDoc->getURL() || url.isEmpty()) {
        saveToAnother = false;
    }
}

void CreateSubalignmentTask::prepare() {
    QString ext = url.completeFileSuffix();

    DocumentFormat *dfd = 0;
    DocumentFormatRegistry *dfr = AppContext::getDocumentFormatRegistry();

    foreach(const DocumentFormatId &dfId, dfr->getRegisteredFormats()) {
        DocumentFormat *df = dfr->getFormatById(dfId);

        foreach(const QString &dfExt, df->getSupportedDocumentFileExtensions()) {
            if(ext.endsWith(dfExt)) {
                dfd = df;
            }
        }
    }

    if(!dfd) {
        coreLog.error(tr("Unable to detect format by file extension"));
        return;
    }

    newDoc = curDoc;

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url));
    if(saveToAnother) {
        QList<GObject*> GObjList = curDoc->getObjects();
        newDoc = dfd->createNewDocument(iof, url, curDoc->getGHintsMap());
        foreach(GObject* go, GObjList){
            GObject *cl = go->clone();
            newDoc->addObject(cl);
            if(go == maObj){
                maObj = static_cast<MAlignmentObject*> (cl);
            }
        }
        foreach(GObject* o, newDoc->getObjects()) {
            GObjectUtils::updateRelationsURL(o, curDoc->getURL(), url);
        }
    }
    else {
        if(newDoc->isStateLocked()){
            coreLog.error(tr("Document is locked"));
            return;
        }
    }

    //TODO: add "remove empty rows and columns" flag to crop function
    QSet<QString> rowNames;
    foreach (const QString& name, seqNames) {
        rowNames.insert(name);
    }
    maObj->crop(window, rowNames);

    if(saveImmediately) {
        addSubTask(new SaveDocumentTask(newDoc, iof, url));
    }

    // Leak alert: if this signal isn't handled somewhere, memory allocated for this document will be lost
    emit documentCreated(newDoc);
}

Task::ReportResult CreateSubalignmentTask::report(){
    return ReportResult_Finished;
}

}
