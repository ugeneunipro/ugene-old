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

#include "GTTestsWorkflowDesigner.h"

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
#include "api/GTSpinBox.h"
#include "api/GTTableView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WizardFiller.h"
#include "runnables/ugene/plugins/workflow_designer/StartupDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/AliasesDialogFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "GTUtilsWorkflowDesigner.h"
#include "GTUtilsMdi.h"
#include "GTUtilsApp.h"
#include "GTUtilsLog.h"

#include <QGraphicsItem>
#include <QtGui/QTextEdit>
#include <U2Core/AppContext.h>
#include <QProcess>
#include "../../workflow_designer/src/WorkflowViewItems.h"
#include <U2Lang/WorkflowSettings.h>

#include "runnables/ugene/plugins/workflow_designer/CreateElementWithScriptDialogFiller.h"
namespace U2 {

//8 - text
//65536 - frame without ports
//65537 - frame with ports
//65538 - ports


namespace GUITest_common_scenarios_workflow_designer {
GUI_TEST_CLASS_DEFINITION(test_0001){


    GTUtilsWorkflowDesigner::openWorkfolwDesigner(os);
    GTUtilsWorkflowDesigner::addAlgorithm(os, "File List");
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "File List"));
    GTMouseDriver::click(os);

    GTUtilsWorkflowDesigner::createDataset(os, "some name");
    //GTWidget::getAllWidgetsInfo(os, table);
    //    QAbstractButton* edit = GTAction::button(os, GTAction::findActionByText(os, "Edit script of the element..."));
//    GTWidget::click(os, edit);

    /*WizardFiller::pairValList list;
    list.append(WizardFiller::pairValLabel("bowtie version",new WizardFiller::ComboBoxValue(0)));
    list.append(WizardFiller::pairValLabel("bowtie index directory", new WizardFiller::lineEditValue
                                           (testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data/index",true)));
    list.append(WizardFiller::pairValLabel("bowtie index basename",new WizardFiller::lineEditValue("chr6",false)));
    list.append(WizardFiller::pairValLabel("tophat",new WizardFiller::lineEditValue(testDir + "_common_data/scenarios/sandbox",true)));
    list.append(WizardFiller::pairValLabel("cufflinks",new WizardFiller::lineEditValue(testDir + "_common_data/scenarios/sandbox",true)));
    list.append(WizardFiller::pairValLabel("cuffmerge",new WizardFiller::lineEditValue(testDir + "_common_data/scenarios/sandbox",true)));
    list.append(WizardFiller::pairValLabel("cuffdiff",new WizardFiller::lineEditValue(testDir + "_common_data/scenarios/sandbox",true)));


    GTUtilsDialog::waitForDialog(os,new ConfigureTuxedoWizardFiller(os,ConfigureTuxedoWizardFiller::full
                                                                    ,ConfigureTuxedoWizardFiller::singleReads));
    GTUtilsDialog::waitForDialog(os,new TuxedoWizardFiller(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data","lymph_aln.fastq",
                                                           testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data","wbc_aln.fastq",list));




    GTUtilsWorkflowDesigner::addSample(os,"tuxedo tools");



    GTKeyboardDriver::keyClick(os, 'r' ,GTKeyboardDriver::key["ctrl"]);
    TaskScheduler* scheduller = AppContext::getTaskScheduler();

    GTGlobals::sleep(5000);
    while(!scheduller->getTopLevelTasks().isEmpty()){
        GTGlobals::sleep();
    }

    bool eq;
//*****************************************
    //tophat1
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0001/tophat_out/junctions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out/junctions.bed");
    CHECK_SET_ERR(eq,"junctions.bed files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0001/tophat_out/deletions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out/deletions.bed");
    CHECK_SET_ERR(eq,"deletions.bed files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0001/tophat_out/insertions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out/insertions.bed");
    CHECK_SET_ERR(eq,"insertions.bed files are not equal");

    //tophat2
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0002/tophat_out/junctions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out_1/junctions.bed");
    CHECK_SET_ERR(eq,"junctions.bed2 files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0002/tophat_out/deletions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out_1/deletions.bed");
    CHECK_SET_ERR(eq,"deletions.bed2 files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0002/tophat_out/insertions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out_1/insertions.bed");
    CHECK_SET_ERR(eq,"insertions.bed2 files are not equal");
//*****************************************
/*    //cufflinks1
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0001/cufflinks_out/skipped.gtf",
                             testDir + "_common_data/scenarios/sandbox/cufflinks_out/skipped.gtf");
    CHECK_SET_ERR(eq,"skipped.gtf files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0001/cufflinks_out/genes.fpkm_tracking",
                             testDir + "_common_data/scenarios/sandbox/cufflinks_out/genes.fpkm_tracking");
    CHECK_SET_ERR(eq,"genes.fpkm_tracking files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0001/cufflinks_out/isoforms.fpkm_tracking",
                             testDir + "_common_data/scenarios/sandbox/cufflinks_out/isoforms.fpkm_tracking");
    CHECK_SET_ERR(eq,"isoforms.fpkm_tracking files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0001/cufflinks_out/transcripts.gtf",
                             testDir + "_common_data/scenarios/sandbox/cufflinks_out/transcripts.gtf");
    CHECK_SET_ERR(eq,"transcripts.gtf files are not equal");

    //cufflinks2
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0002/cufflinks_out_1/skipped.gtf",
                             testDir + "_common_data/scenarios/sandbox/cufflinks_out_1/skipped.gtf");
    CHECK_SET_ERR(eq,"skipped.gtf2 files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0002/cufflinks_out_1/genes.fpkm_tracking",
                             testDir + "_common_data/scenarios/sandbox/cufflinks_out_1/genes.fpkm_tracking");
    CHECK_SET_ERR(eq,"genes.fpkm_tracking2 files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0002/cufflinks_out_1/isoforms.fpkm_tracking",
                             testDir + "_common_data/scenarios/sandbox/cufflinks_out_1/isoforms.fpkm_tracking");
    CHECK_SET_ERR(eq,"isoforms.fpkm_tracking2 files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0002/cufflinks_out_1/transcripts.gtf",
                             testDir + "_common_data/scenarios/sandbox/cufflinks_out_1/transcripts.gtf");
    CHECK_SET_ERR(eq,"transcripts.gtf2 files are not equal");*/
//*****************************************

}

GUI_TEST_CLASS_DEFINITION(test_0002){
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    //1. Start UGENE. Open workflow schema file from data\cmdline\pfm-build.uws
    GTFileDialog::openFile(os,dataDir + "cmdline/","pwm-build.uwl");
    GTGlobals::sleep(1000);
//  Expected state: workflow schema opened in Workflow designer
//    2. Change item style (Minimal - Extended - Minimal - Extended)
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    CHECK_SET_ERR(sceneView,"scene not found");
    QList<QGraphicsItem *> items = sceneView->items();
    QList<QPointF> posList;

    foreach(QGraphicsItem* item,items){
        posList.append(item->pos());
    }
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Minimal"));
    GTWidget::click(os, GTWidget::findWidget(os,"Element style"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Extended"));
    GTWidget::click(os, GTWidget::findWidget(os,"Element style"));
//  Expected state: all arrows in schema still unbroken
    items = sceneView->items();
    foreach(QGraphicsItem* item,items){
        QPointF p = posList.takeFirst();
        CHECK_SET_ERR(p==item->pos(),QString("some item changed position from %1, %2 to %3, %4")
                      .arg(p.x()).arg(p.y()).arg(item->pos().x()).arg(item->pos().y()));
    }

}

GUI_TEST_CLASS_DEFINITION(test_0002_1){
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    //1. Start UGENE. Open workflow schema file from data\cmdline\pfm-build.uws
    GTFileDialog::openFile(os,dataDir + "cmdline/","pwm-build.uwl");
    GTGlobals::sleep(1000);
//  Expected state: workflow schema opened in Workflow designer
//    2. Change item style (Minimal - Extended - Minimal - Extended)
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    CHECK_SET_ERR(sceneView,"scene not found");
    QList<QGraphicsItem *> items = sceneView->items();
    QList<QPointF> posList;

    foreach(QGraphicsItem* item,items){
        posList.append(item->pos());
    }

    GTMouseDriver::moveTo(os,GTUtilsWorkflowDesigner::getItemCenter(os, "Write Weight Matrix"));
    GTMouseDriver::doubleClick(os);

    GTGlobals::sleep();
    GTMouseDriver::moveTo(os,QPoint(GTUtilsWorkflowDesigner::getItemLeft(os, "Write Weight Matrix")+25,
                                    GTUtilsWorkflowDesigner::getItemTop(os,"Write Weight Matrix")+25));
    GTMouseDriver::doubleClick(os);

//  Expected state: all arrows in schema still unbroken
        items = sceneView->items();
        foreach(QGraphicsItem* item,items){
            QPointF p = posList.takeFirst();
            CHECK_SET_ERR(p==item->pos(),QString("some item changed position from %1, %2 to %3, %4")
                          .arg(p.x()).arg(p.y()).arg(item->pos().x()).arg(item->pos().y()));
        }
}

GUI_TEST_CLASS_DEFINITION(test_0003){
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
//    1. Start UGENE. Open workflow schema file from \common data\workflow\remoteDBReaderTest.uws
    GTFileDialog::openFile(os,testDir + "_common_data/workflow/","remoteDBReaderTest.uws");
//    Expected state: workflow schema opened in Workflow designer
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    CHECK_SET_ERR(table,"tableView not found");
    GTMouseDriver::moveTo(os,GTUtilsWorkflowDesigner::getItemCenter(os,"Write Genbank"));
    GTMouseDriver::click(os);
    GTMouseDriver::moveTo(os,GTTableView::getCellPosition(os,table,1,3));
    GTMouseDriver::click(os);
    QString s = QDir().absoluteFilePath(testDir + "_common_data/scenarios/sandbox/");
    GTKeyboardDriver::keySequence(os,s+"T1.gb");
    GTWidget::click(os,GTUtilsMdi::activeWindow(os));

    GTWidget::click(os,GTAction::button(os,"Run workflow"));

    GTGlobals::sleep();
//    2. If you don't want result file (T1.gb) in UGENE run directory, change this property in write genbank worker.Run schema.
//    Expected state: T1.gb file is saved to your disc
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/sandbox/","T1.gb");
}

GUI_TEST_CLASS_DEFINITION(test_0005){
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
//1. Open WD
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
//2. Press button Validate schema
    GTUtilsDialog::waitForDialog(os,new MessageBoxDialogFiller(os, QMessageBox::Ok,"Nothing to run: empty workflow"));
    GTWidget::click(os,GTAction::button(os,"Validate workflow"));
//Expected state: message box which warns of validating empty schema has appeared
}

GUI_TEST_CLASS_DEFINITION(test_0006){
#ifndef Q_OS_LINUX
    //GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
#endif
//1. Do menu Settings->Prefrences
    GTUtilsDialog::waitForDialog(os,new AppSettingsDialogFiller(os,AppSettingsDialogFiller::minimal));
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_SETTINGS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "action__settings");
//2. Open WD settings
//3. Change Default visualization Item style from Extended to Minimal.
//4. Click OK button

//5. Open WD
    menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
//6. Load any scheme from samples tab
    GTUtilsWorkflowDesigner::addAlgorithm(os,"read alignment");
//Expected state: item style on loaded schema must be Minimal
    StyleId id;
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    QList<QGraphicsItem *> items = sceneView->items();
    foreach(QGraphicsItem* item, items){
        WorkflowProcessItem* s = qgraphicsitem_cast<WorkflowProcessItem*>(item);
        if(s){
            id = s->getStyle();
            CHECK_SET_ERR(id=="simple","items style is not minimal");
        }
    }
}

GUI_TEST_CLASS_DEFINITION(test_0006_1){
    //GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
//1. Do menu Settings->Prefrences
    GTUtilsDialog::waitForDialog(os,new AppSettingsDialogFiller(os,AppSettingsDialogFiller::extended));
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_SETTINGS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "action__settings");
//2. Open WD settings
//3. Change Default visualization Item style from Extended to Minimal.
//4. Click OK button

//5. Open WD
    menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
//6. Load any scheme from samples tab
    GTUtilsWorkflowDesigner::addAlgorithm(os,"read alignment");
//Expected state: item style on loaded schema must be Minimal
    StyleId id;
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    QList<QGraphicsItem *> items = sceneView->items();
    foreach(QGraphicsItem* item, items){
        WorkflowProcessItem* s = qgraphicsitem_cast<WorkflowProcessItem*>(item);
        if(s){
            id = s->getStyle();
            CHECK_SET_ERR(id=="ext","items style is not minimal");
        }
    }
}

GUI_TEST_CLASS_DEFINITION(test_0007){
    //GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
//1. Do menu {Settings->Prefrences}
    GTUtilsDialog::waitForDialog(os,new AppSettingsDialogFiller(os,255,0,0));
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_SETTINGS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "action__settings");
//2. Activate WD prefrences page. Change Backgrounf color for workers.

//3. Open WD and place any worker on working area.
    menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
//Expected state: workers background color must be same as in prefrences
    GTUtilsWorkflowDesigner::addAlgorithm(os,"read alignment");
    QPoint p(GTUtilsWorkflowDesigner::getItemLeft(os,"read alignment")+20,
             GTUtilsWorkflowDesigner::getItemTop(os,"read alignment")+20);

    QPixmap pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
    QImage img = pixmap.toImage();
    QRgb rgb = img.pixel(p);
    QColor c(rgb);

    CHECK_SET_ERR(c.name()=="#ff0000", QString("Expected: #ffbfbf, found: %1").arg(c.name()));

}

GUI_TEST_CLASS_DEFINITION(test_0009){
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
//    1. Open schema from examples
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
    GTUtilsWorkflowDesigner::addSample(os, "call variants");
//    2. Clear dashboard (select all + del button)
    GTGlobals::sleep(500);
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    CHECK_SET_ERR(sceneView,"scene not found");
    QList<QGraphicsItem *> items = sceneView->items();
    QList<QPointF> posList;

    foreach(QGraphicsItem* item,items){
        if(qgraphicsitem_cast<WorkflowProcessItem*>(item))
            posList.append(item->pos());
    }

    GTWidget::setFocus(os,GTWidget::findWidget(os,"sceneView"));
    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
//    3. Open this schema from examples
    GTUtilsWorkflowDesigner::addSample(os, "call variants");
//    Expected state: items and links between them painted correctly
    GTGlobals::sleep(500);
    QList<QGraphicsItem *> items1 = sceneView->items();
    QList<QPointF> posList1;

    foreach(QGraphicsItem* item,items1){
        if(qgraphicsitem_cast<WorkflowProcessItem*>(item))
            posList1.append(item->pos());
    }

    CHECK_SET_ERR(posList==posList1,"some workers changed positions");
}

GUI_TEST_CLASS_DEFINITION(test_0010){
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
//    1. Open WD
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
//    2. Place 3 HMM build workflow elements on scheme
    GTUtilsWorkflowDesigner::addAlgorithm(os, "read sequence");
    GTUtilsWorkflowDesigner::addAlgorithm(os,"write sequence");

    WorkflowProcessItem* read = GTUtilsWorkflowDesigner::getWorker(os,"read sequence");
    WorkflowProcessItem* write = GTUtilsWorkflowDesigner::getWorker(os,"write sequence");
    GTUtilsWorkflowDesigner::connect(os,read,write);
    GTGlobals::sleep();
    /*GTUtilsWorkflowDesigner::addAlgorithm(os,"hmm build");

    GTUtilsWorkflowDesigner::addAlgorithm(os,"hmm build");

    GTUtilsWorkflowDesigner::addAlgorithm(os,"hmm build");


//    Expected state: there 3 element with names "HMM build" "HMM build 1" "HMM build 2"
    QGraphicsItem* hmm = GTUtilsWorkflowDesigner::getWorker(os,"hmm build");
    CHECK_SET_ERR(hmm,"hmm not found");
    hmm = GTUtilsWorkflowDesigner::getWorker(os,"hmm build 1");
    CHECK_SET_ERR(hmm,"hmm 1 not found");
    hmm = GTUtilsWorkflowDesigner::getWorker(os,"hmm build 2");
    CHECK_SET_ERR(hmm,"hmm 2 not found");*/
}

GUI_TEST_CLASS_DEFINITION(test_0013){
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
//    1. Load any sample in WD
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
    GTUtilsWorkflowDesigner::addSample(os, "call variants");
//    2. Select output port.
    WorkflowProcessItem* gr = GTUtilsWorkflowDesigner::getWorker(os,"call variants with");
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    QList<WorkflowPortItem*> list = gr->getPortItems();
    foreach(WorkflowPortItem* p, list){
        if(p&&p->getPort()->getId()=="out-variations"){
            QPointF scenePButton = p->mapToScene(p->boundingRect().center());
            QPoint viewP = sceneView->mapFromScene(scenePButton);
            QPoint globalBottomRightPos = sceneView->viewport()->mapToGlobal(viewP);
            GTMouseDriver::moveTo(os, globalBottomRightPos);
            GTMouseDriver::click(os);
            GTGlobals::sleep(2000);
        }
    }
    QTextEdit* doc = qobject_cast<QTextEdit*>(GTWidget::findWidget(os,"doc"));
    CHECK_SET_ERR(doc->document()->toPlainText().contains("Output port \"Output variations"),"expected text not found");

//    Expected state: in property editor 'Output port' item appears

//    3. Select input port.
    WorkflowPortItem* in = GTUtilsWorkflowDesigner::getPortById(os, gr, "in-assembly");
    QPointF scenePButton = in->mapToScene(in->boundingRect().center());
    QPoint viewP = sceneView->mapFromScene(scenePButton);
    QPoint globalBottomRightPos = sceneView->viewport()->mapToGlobal(viewP);
    GTMouseDriver::moveTo(os, globalBottomRightPos);
    GTMouseDriver::click(os);

    doc = qobject_cast<QTextEdit*>(GTWidget::findWidget(os,"doc"));
    CHECK_SET_ERR(doc->document()->toPlainText().contains("Input port \"Input assembly"),"expected text not found");
//    Expected state: in property editor 'Input port' item appears
}

GUI_TEST_CLASS_DEFINITION(test_0015){
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
//    1. open WD.
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
//    2. Select any worker on palette.
    GTUtilsWorkflowDesigner::addSample(os,"call variants");
    GTMouseDriver::moveTo(os,GTUtilsWorkflowDesigner::getItemCenter(os,"call variants with"));
    GTMouseDriver::click(os);
    GTGlobals::sleep(500);
    CHECK_SET_ERR(GTWidget::findWidget(os,"table"),"parameters table not found");
    CHECK_SET_ERR(GTWidget::findWidget(os,"doc"),"element documentation widget not found");
    CHECK_SET_ERR(GTWidget::findWidget(os,"table2"),"input data table not found");
    CHECK_SET_ERR(GTWidget::findWidget(os,"propDoc"),"property documentation widget not found");

//    Expected state: Actor info (parameters, input data ...) will be displayed at the right part of window
}

GUI_TEST_CLASS_DEFINITION(test_0015_1){//DIFFERENCE:file is loaded
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
//    1. open WD.
    GTFileDialog::openFile(os,dataDir + "cmdline/","pwm-build.uwl");
    GTGlobals::sleep(1000);
//    2. Select any worker on palette.
    GTMouseDriver::moveTo(os,GTUtilsWorkflowDesigner::getItemCenter(os,"Write Weight Matrix"));
    GTMouseDriver::click(os);
    GTGlobals::sleep(500);
    CHECK_SET_ERR(GTWidget::findWidget(os,"table"),"parameters table not found");
    CHECK_SET_ERR(GTWidget::findWidget(os,"doc"),"element documentation widget not found");
    CHECK_SET_ERR(GTWidget::findWidget(os,"table2"),"input data table not found");
    CHECK_SET_ERR(GTWidget::findWidget(os,"propDoc"),"property documentation widget not found");

//    Expected state: Actor info (parameters, input data ...) will be displayed at the right part of window
}
GUI_TEST_CLASS_DEFINITION(test_0016){
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
//    1. open WD.
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");

//    2. Place Read align element on schema
    GTUtilsWorkflowDesigner::addAlgorithm(os, "read alignment");
//    3. Press button "Configure command line aliases"
    QMap<QPoint*,QString> map;
    QPoint p(1,0);
    //map.i
    map[&p] ="qqq";
    //map.insert(p,QString("qqq"));
    GTUtilsDialog::waitForDialog(os, new AliasesDialogFiller(os,map));
    GTWidget::click(os, GTAction::button(os,"Configure parameter aliases"));
//    4. Add command line alias 'qqq' for schema parameter 'Input files'

//    5. Save schema.

//    6. Press button "Configure command line aliases"

//    7. Change command line alias from 'qqq' to 'zzz'

//    8. Save schema.

//    9 Close and open this schema again.

//    10. Press button "Configure command line aliases"
//    Expected state: alias must be named 'zzz'
}

GUI_TEST_CLASS_DEFINITION(test_0017){
    //Test for UGENE-2202
    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os, true, testDir + "_common_data/scenarios/sandbox/somedir"));
    //1. Open Workflow Designer
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
    //2. Write the path to the directory which does not exist(in the StartupDialogFiller).
    //3. Click OK(in the StartupDialogFiller).
    CHECK_SET_ERR(!l.hasError(), "There are error messages about write access in WD directory");
}

} // namespace GUITest_common_scenarios_workflow_designer

} // namespace U2
