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

#include "GTTestsNiaidPipelines.h"

#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTGlobals.h"
#include "api/GTTreeWidget.h"
#include "api/GTAction.h"
#include "api/GTFile.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/plugins/workflow_designer/WizardFiller.h"
#include "runnables/ugene/plugins/workflow_designer/StartupDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/RPackageDialorFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "GTUtilsWorkflowDesigner.h"
#include "GTUtilsApp.h"

#include <QGraphicsItem>
#include <U2Core/AppContext.h>
#include <QProcess>


namespace U2 {

namespace GUITest_common_scenarios_NIAID_pipelines {
class WizardFiller0001:public WizardFiller{
public:
WizardFiller0001(U2OpStatus &_os, QString _label): WizardFiller(_os,"Call Variants Wizard"), label(_label){

}
void setParameters(U2OpStatus &os, QString lab){

    QList<QLabel*> labelList = wizard->currentPage()->findChildren<QLabel*>();
    QList<QWidget*> widgetList = wizard->currentPage()->findChildren<QWidget*>();



    foreach(QLabel* label, labelList){
        if(label->text().toLower()==lab.toLower()){
            QList<QWidget*> yList;
            foreach (QWidget* w, widgetList){
                if(abs(w->mapTo(wizard,w->rect().center()).y() - label->mapTo(wizard,label->rect().center()).y())==0){
                    yList.append(w);
                }
            }
            foreach(QWidget* w, yList){
                QToolButton* t = qobject_cast<QToolButton*>(w);
                if (t)
                    GTWidget::click(os,t);
            }
            foreach(QWidget* w, yList){
                QLineEdit* line = qobject_cast<QLineEdit*>(w);
                if (line){
                    CHECK_SET_ERR(line->text()==path, "wrong path: " + line->text());
                }
            }
        }
    }

}
virtual void run(){
    dialog = QApplication::activeModalWidget();
    CHECK_SET_ERR(dialog!=NULL, "activeModalWidget is NULL");
    wizard = qobject_cast<QWizard*>(dialog);
    CHECK_SET_ERR(wizard, "activeModalWidget is not wizard");

    path = QDir::cleanPath(QDir::currentPath() + "/" + GUITest::dataDir + "samples/CLUSTALW/COI.aln");

    QPushButton* next = getNextButton(os);
    QPushButton* finish = getFinishButton(os);

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, GUITest::dataDir + "samples/CLUSTALW/" , "COI.aln");
    GTUtilsDialog::waitForDialog(os, ob);

    for(int i=0;i<20;i++){
       if(getExpandButton(os))
           GTWidget::click(os,getExpandButton(os));

       setParameters(os,label);
       GTGlobals::sleep(500);
       if (finish->isEnabled())
           break;
       else
           GTWidget::click(os, next);
    }
    GTWidget::click(os, getFinishButton(os));
    }
private:
    QString label,path;
    QWidget* dialog; //= QApplication::activeModalWidget();
    QWizard* wizard; //= qobject_cast<QWizard*>(dialog);
};
GUI_TEST_CLASS_DEFINITION(test_0001){
    //GTUtilsDialog::waitForDialog(os,new RPackageDialorFiller(os));
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");

    GTUtilsWorkflowDesigner::addSample(os,"call variants");
    GTUtilsDialog::waitForDialog(os, new WizardFiller0001(os,"BED or position list file"));
    QAbstractButton* wiz = GTAction::button(os, "Show wizard");
    GTWidget::click(os,wiz);

    TaskScheduler* scheduller = AppContext::getTaskScheduler();

    GTGlobals::sleep(5000);
    while(!scheduller->getTopLevelTasks().isEmpty()){
        GTGlobals::sleep();
    }
}
#define GT_CLASS_NAME "GTUtilsDialog::WizardFiller0002"
#define GT_METHOD_NAME "run"
class WizardFiller0002:public WizardFiller{
public:
    WizardFiller0002(U2OpStatus &_os):WizardFiller(_os,"Tuxedo Wizard"){}
    void run(){
        QWidget* dialog = QApplication::activeModalWidget();
        GT_CHECK(dialog, "activeModalWidget is NULL");

        QList<QWidget*> list= dialog->findChildren<QWidget*>();

        QList<QWidget*> datasetList;
        foreach(QWidget* act, list){
            if(act->objectName()=="DatasetWidget")
            datasetList.append(act);
        }
        QWidget* dataset = datasetList.takeLast();

        QPushButton* cancel = WizardFiller::getCancelButton(os);

        GT_CHECK(dataset,"dataset widget not found");
        GT_CHECK(cancel, "cancel button not found");

        QPoint i = dataset->mapToGlobal(dataset->rect().bottomLeft());
        QPoint j = cancel->mapToGlobal(cancel->rect().topLeft());

        CHECK_SET_ERR(qAbs(i.y()-j.y())<50, QString("%1   %2").arg(i.y()).arg(j.y()));
        GTWidget::click(os,cancel);
    }
};
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

GUI_TEST_CLASS_DEFINITION(test_0002){
//    1. Open WD
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
//    2. Open tuxedo pipeline from samples
    GTUtilsDialog::waitForDialog(os, new WizardFiller0002(os));
    GTUtilsDialog::waitForDialog(os,new ConfigureTuxedoWizardFiller(os,ConfigureTuxedoWizardFiller::full
                                                                    ,ConfigureTuxedoWizardFiller::singleReads));
    GTUtilsWorkflowDesigner::addSample(os,"Tuxedo tools");
//    3. Open wizard


//    Expected state: dataset widget fits full height
}

} // namespace GUITest_common_scenarios_annotations_edit
} // namespace U2
