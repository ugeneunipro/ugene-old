#include "ETSProjectViewItemsContoller.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ProjectView.h>
#include <U2Core/SelectionModel.h>

#include <U2Core/GObjectUtils.h>

#include <U2Core/DocumentSelection.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/SelectionUtils.h>

#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>

#include "blast/FormatDBSupport.h"
#include "blast/FormatDBSupportRunDialog.h"
#include "blast/FormatDBSupportTask.h"

namespace U2 {
ETSProjectViewItemsContoller::ETSProjectViewItemsContoller(QObject* p) : QObject(p) {
    formatDBOnSelectionAction=new ExternalToolSupprotAction(tr("FormatDB..."), this, QStringList(FORMATDB_TOOL_NAME));
    makeBLASTDBOnSelectionAction=new ExternalToolSupprotAction(tr("BLAST+ make DB..."), this, QStringList(MAKEBLASTDB_TOOL_NAME));
    connect(formatDBOnSelectionAction,SIGNAL(triggered()), SLOT(sl_runFormatDBOnSelection()));
    connect(makeBLASTDBOnSelectionAction,SIGNAL(triggered()), SLOT(sl_runFormatDBOnSelection()));

    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);
    connect(pv, SIGNAL(si_onDocTreePopupMenuRequested(QMenu&)), SLOT(sl_addToProjectViewMenu(QMenu&)));
}

void ETSProjectViewItemsContoller::sl_addToProjectViewMenu(QMenu& m) {

    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);

    MultiGSelection ms; ms.addSelection(pv->getGObjectSelection()); ms.addSelection(pv->getDocumentSelection());
    QSet<GObject*> set = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    if (!set.isEmpty()) {
        QMenu* subMenu = m.addMenu(tr("BLAST"));
        subMenu->setIcon(QIcon(":external_tool_support/images/ncbi.png"));
        subMenu->addAction(formatDBOnSelectionAction);
        subMenu->addAction(makeBLASTDBOnSelectionAction);
    }
}
void ETSProjectViewItemsContoller::sl_runFormatDBOnSelection(){
    ExternalToolSupprotAction* s = qobject_cast<ExternalToolSupprotAction*>(sender());
    assert(s != NULL);
    assert((s->getToolNames().contains(FORMATDB_TOOL_NAME))||(s->getToolNames().contains(MAKEBLASTDB_TOOL_NAME)));
    //Check that formatDB and tempory directory path defined
    if (AppContext::getExternalToolRegistry()->getByName(s->getToolNames().at(0))->getPath().isEmpty()){
        QMessageBox msgBox;
        if(s->getToolNames().at(0) == FORMATDB_TOOL_NAME){
            msgBox.setWindowTitle("BLAST "+s->getToolNames().at(0));
            msgBox.setText(tr("Path for BLAST %1 tool not selected.").arg(s->getToolNames().at(0)));
        }else{
            msgBox.setWindowTitle("BLAST+ "+s->getToolNames().at(0));
            msgBox.setText(tr("Path for BLAST+ %1 tool not selected.").arg(s->getToolNames().at(0)));
        }
        msgBox.setInformativeText(tr("Do you want to select it now?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret) {
           case QMessageBox::Yes:
               AppContext::getAppSettingsGUI()->showSettingsDialog(ExternalToolSupportSettingsPageId);
               break;
           case QMessageBox::No:
               return;
               break;
           default:
               assert(NULL);
               break;
         }
    }
    if (AppContext::getExternalToolRegistry()->getByName(s->getToolNames().at(0))->getPath().isEmpty()){
        return;
    }
    ExternalToolSupportSettings::checkTemporaryDir();
    if (AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath().isEmpty()){
        return;
    }

    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);

    MultiGSelection ms; ms.addSelection(pv->getGObjectSelection()); ms.addSelection(pv->getDocumentSelection());//???
    FormatDBSupportTaskSettings settings;
    foreach(Document* doc,pv->getDocumentSelection()->getSelectedDocuments()){
        settings.inputFilesPath.append(doc->getURLString());
    }
    FormatDBSupportRunDialog formatDBRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    if(formatDBRunDialog.exec() != QDialog::Accepted){
        return;
    }
    FormatDBSupportTask* formatDBSupportTask=new FormatDBSupportTask(s->getToolNames().at(0), settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(formatDBSupportTask);
}
}//namespace
