/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "MSAEditorTasks.h"
#include "MSAEditor.h"
#include "MSAEditorFactory.h"
#include "MSAEditorState.h"
#include "MSAEditorConsensusArea.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/TextObject.h>
#include <U2Core/UnloadedObject.h>

#include <U2Gui/OpenViewTask.h>

#include <U2Formats/DocumentFormatUtils.h>

#include <QtCore/QSet>

namespace U2 {

/* TRANSLATOR U2::MSAEditor */
/* TRANSLATOR U2::ObjectViewTask */

//////////////////////////////////////////////////////////////////////////
/// open new view

OpenMSAEditorTask::OpenMSAEditorTask(MAlignmentObject* _obj) 
: ObjectViewTask(MSAEditorFactory::ID), msaObject(_obj)
{
    assert(!msaObject.isNull());
}

OpenMSAEditorTask::OpenMSAEditorTask(UnloadedObject* _obj) 
: ObjectViewTask(MSAEditorFactory::ID), unloadedReference(_obj)
{
    assert(_obj->getLoadedObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT);
    documentsToLoad.append(_obj->getDocument());
}

OpenMSAEditorTask::OpenMSAEditorTask(Document* doc) 
: ObjectViewTask(MSAEditorFactory::ID), msaObject(NULL)
{
    assert(!doc->isLoaded());
    documentsToLoad.append(doc);
}

void OpenMSAEditorTask::open() {
    if (stateInfo.hasError() || (msaObject.isNull() && documentsToLoad.isEmpty())) {
        return;
    }
    if (msaObject.isNull()) {
        Document* doc = documentsToLoad.first();
        if(!doc){
            stateInfo.setError(tr("Documet removed from project"));
            return;
        }
        QList<GObject*> objects;
        if (unloadedReference.isValid()) {
            GObject* obj = doc->findGObjectByName(unloadedReference.objName);
            if (obj!=NULL && obj->getGObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT) {
                msaObject = qobject_cast<MAlignmentObject*>(obj);
            }
        } else {
            QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT, UOF_LoadedAndUnloaded);
            msaObject = objects.isEmpty() ? NULL : qobject_cast<MAlignmentObject*>(objects.first());
        }
        if (msaObject.isNull()) {
            stateInfo.setError(tr("Multiple alignment object not found"));
            return;
        }
    }
    viewName = GObjectViewUtils::genUniqueViewName(msaObject->getDocument(), msaObject);
    uiLog.details(tr("Opening MSA editor for object: %1").arg(msaObject->getGObjectName()));

    MSAEditor* v = new MSAEditor(viewName, msaObject);
    GObjectViewWindow* w = new GObjectViewWindow(v, viewName, false);
    MWMDIManager* mdiManager = AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(w);

}

void OpenMSAEditorTask::updateTitle(MSAEditor* msaEd) {
    const QString& oldViewName = msaEd->getName();
    GObjectViewWindow* w = GObjectViewUtils::findViewByName(oldViewName);
    if (w != NULL) {
        MAlignmentObject* msaObject = msaEd->getMSAObject();
        QString newViewName = GObjectViewUtils::genUniqueViewName(msaObject->getDocument(), msaObject);
        msaEd->setName(newViewName);
        w->setWindowTitle(newViewName);
    }
}

//////////////////////////////////////////////////////////////////////////
// open view from state

OpenSavedMSAEditorTask::OpenSavedMSAEditorTask(const QString& viewName, const QVariantMap& stateData) 
: ObjectViewTask(MSAEditorFactory::ID, viewName, stateData)
{
    MSAEditorState state(stateData);
    GObjectReference ref = state.getMSAObjectRef();
    Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
    if (doc == NULL) {
        doc = createDocumentAndAddToProject(ref.docUrl, AppContext::getProject(), stateInfo);
        CHECK_OP_EXT(stateInfo, stateIsIllegal = true ,);
    }
    if (!doc->isLoaded()) {
        documentsToLoad.append(doc);
    }

}

void OpenSavedMSAEditorTask::open() {
    CHECK_OP(stateInfo, );
    
    MSAEditorState state(stateData);
    GObjectReference ref = state.getMSAObjectRef();
    Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
    if (doc == NULL) {
        stateIsIllegal = true;
        stateInfo.setError(L10N::errorDocumentNotFound(ref.docUrl));
        return;
    }
    GObject* obj = doc->findGObjectByName(ref.objName);
    if (obj == NULL || obj->getGObjectType() != GObjectTypes::MULTIPLE_ALIGNMENT) {
        stateIsIllegal = true;
        stateInfo.setError(tr("Alignment object not found: %1").arg(ref.objName));
        return;
    }
    MAlignmentObject* msaObject = qobject_cast<MAlignmentObject*>(obj);
    assert(msaObject!=NULL);

    MSAEditor* v = new MSAEditor(viewName, msaObject);
    GObjectViewWindow* w = new GObjectViewWindow(v, viewName, true);
    MWMDIManager* mdiManager =     AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(w);

    updateRanges(stateData, v);
}

void OpenSavedMSAEditorTask::updateRanges(const QVariantMap& stateData, MSAEditor* ctx) {
    Q_UNUSED(ctx);
    MSAEditorState state(stateData);

    QFont f = state.getFont();
    if (!f.isCopyOf(QFont())) {
        ctx->setFont(f);
    }

    int firstPos = state.getFirstPos();
    ctx->setFirstVisibleBase(firstPos);

    float zoomFactor = state.getZoomFactor();
    ctx->setZoomFactor(zoomFactor);
}


//////////////////////////////////////////////////////////////////////////
// update
UpdateMSAEditorTask::UpdateMSAEditorTask(GObjectView* v, const QString& stateName, const QVariantMap& stateData) 
: ObjectViewTask(v, stateName, stateData)
{
}

void UpdateMSAEditorTask::update() {
    if (view.isNull() || view->getFactoryId() != MSAEditorFactory::ID) {
        return; //view was closed;
    }

    MSAEditor* msaView = qobject_cast<MSAEditor*>(view.data());
    assert(msaView!=NULL);

    OpenSavedMSAEditorTask::updateRanges(stateData, msaView);
}


ExportMSAConsensusTask::ExportMSAConsensusTask(const ExportMSAConsensusTaskSettings& s )
: DocumentProviderTask(tr("Export consensus to MSA")
, (TaskFlags(TaskFlag_NoRun) | TaskFlag_FailOnSubtaskError | TaskFlag_CancelOnSubtaskCancel))
, settings(s){
    setVerboseLogMode(true);
    SAFE_POINT_EXT(s.msa != NULL, setError("Given msa pointer is NULL"), );
}

void ExportMSAConsensusTask::prepare(){
    extractConsensus = new ExtractConsensusTask(settings.keepGaps, settings.policy, settings.msa);
    addSubTask(extractConsensus);
}

QList<Task*> ExportMSAConsensusTask::onSubTaskFinished( Task* subTask ){
    QList<Task*> result;
    if(subTask == extractConsensus && !isCanceled() && !hasError()) {
        Document *takenDoc = createDocument();
        CHECK_OP(stateInfo, result);
        SaveDocumentTask *t = new SaveDocumentTask(takenDoc, takenDoc->getIOAdapterFactory(), takenDoc->getURL());
        if (settings.addToProjectFlag){
            AppContext::getTaskScheduler()->registerTopLevelTask(new AddDocumentAndOpenViewTask(takenDoc));
        }else{
            t->addFlag(SaveDoc_DestroyAfter);
        }
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
    return result;
}

Document *ExportMSAConsensusTask::createDocument(){
    filteredConsensus = extractConsensus->getExtractedConsensus();
    CHECK_EXT(!filteredConsensus.isEmpty(), setError("Consensus is empty!"), NULL);
    QString fullPath = GUrlUtils::prepareFileLocation(settings.url, stateInfo);
    CHECK_OP(stateInfo, NULL);
    GUrl url(fullPath);

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(settings.url));
    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(settings.format);
    CHECK_EXT(df, setError("Document format is NULL!"), NULL);
    GObject *obj = NULL;
    QScopedPointer<Document> doc(df->createNewLoadedDocument(iof, fullPath, stateInfo));
    CHECK_OP(stateInfo, NULL);
    if (df->getFormatId() == BaseDocumentFormats::PLAIN_TEXT){
        obj = TextObject::createInstance(filteredConsensus, settings.name, doc->getDbiRef(), stateInfo);
    }else{
        obj = DocumentFormatUtils::addSequenceObject(doc->getDbiRef(), settings.name, filteredConsensus, false, QVariantMap(), stateInfo);
    }
    CHECK_OP(stateInfo, NULL);
    doc->addObject(obj);
    return doc.take();
}

ExtractConsensusTask::ExtractConsensusTask( bool keepGaps_, NonAlphabetSymbolsPolicy policy_, MSAEditor* msa_ )
: Task(tr("Export consensus to MSA"), TaskFlags(TaskFlag_None)), 
keepGaps(keepGaps_), policy(policy_), msa(msa_){
    setVerboseLogMode(true);
    SAFE_POINT_EXT(msa != NULL, setError("Given msa pointer is NULL"), );
}

void ExtractConsensusTask::run() {
    const DNAAlphabet *alphabet = msa->getMSAObject()->getAlphabet();
    CHECK(msa->getUI(), );
    CHECK(msa->getUI()->getConsensusArea(), );
    CHECK(msa->getUI()->getConsensusArea()->getConsensusCache(),)
    foreach(QChar c, msa->getUI()->getConsensusArea()->getConsensusCache()->getConsensusLine(true)){
        if(c == '-' && !keepGaps){
            continue;
        }
        if (policy == AllowAllSymbols){
            filteredConsensus.append(c);
        }else{
            if(alphabet->getAlphabetChars(true).contains(c.toLatin1())){
                switch (policy)
                {
                case Skip:
                    break;
                case ReplaceWithGap:
                    filteredConsensus.append('-');
                    break;
                case ReplaceWithDefault:
                    filteredConsensus.append(alphabet->getDefaultSymbol());
                    break;
                default:
                    setError("Got unknown policy in settings");
                    break;
                }
            }else{
                filteredConsensus.append(c);
            }            
        }
    }
}

const QByteArray& ExtractConsensusTask::getExtractedConsensus() const {
    return filteredConsensus;
}


ExportMSAConsensusTaskSettings::ExportMSAConsensusTaskSettings(): keepGaps(true), policy(Skip), msa(NULL), 
format(BaseDocumentFormats::PLAIN_TEXT), addToProjectFlag(false)
{}

} // namespace
