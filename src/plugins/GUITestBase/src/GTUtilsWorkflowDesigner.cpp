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

#include "GTUtilsWorkflowDesigner.h"
#include "api/GTWidget.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsMdi.h"
#include "api/GTTabWidget.h"
#include "api/GTGraphicsItem.h"
#include "api/GTFileDialog.h"
#include "api/GTTableView.h"
#include "api/GTSpinBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTMenu.h"
#include "runnables/ugene/plugins/workflow_designer/DatasetNameEditDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/StartupDialogFiller.h"

#include <U2View/MSAEditor.h>
#include <QTreeWidget>
#include <QtGui/QToolButton>
#include <QtGui/QGraphicsView>
#include <QtGui/QListWidget>
#include <QtGui/QTableView>
#include <QtGui/QSpinBox>
#include <U2Core/AppContext.h>
#include "../../workflow_designer/src/WorkflowViewItems.h"

namespace U2 {
#define GT_CLASS_NAME "GTUtilsWorkflowDesigner"

#define GT_METHOD_NAME "openWorkflowDesigner"
void GTUtilsWorkflowDesigner::openWorkflowDesigner(U2OpStatus &os){
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new StartupDialogFiller(os));

    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findTreeItem"
QTreeWidgetItem* GTUtilsWorkflowDesigner::findTreeItem(U2OpStatus &os,QString itemName, tab t){

    QTreeWidgetItem* foundItem=NULL;
    QTreeWidget *w;
    if(t==algoriths){
        w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"WorkflowPaletteElements"));
    }
    else{
        w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"samples"));
    }
    GT_CHECK_RESULT(w!=NULL,"WorkflowPaletteElements is null", NULL);

    QList<QTreeWidgetItem*> outerList = w->findItems("",Qt::MatchContains);

    for (int i=0;i<outerList.count();i++){
        QList<QTreeWidgetItem*> innerList;

        for(int j=0;j<outerList.value(i)->childCount();j++ ){
           innerList.append(outerList.value(i)->child(j));
        }

        foreach(QTreeWidgetItem* item, innerList){
            if(t==algoriths){
                QString s = item->data(0,Qt::UserRole).value<QAction*>()->text();
                if(s.toLower().contains(itemName.toLower())){
                    GT_CHECK_RESULT(foundItem==NULL,"several items have this discription",item);
                    foundItem=item;
                }
            }
            else{
                QString s = item->text(0);
                if(s.toLower().contains(itemName.toLower())){
                    GT_CHECK_RESULT(foundItem==NULL,"several items have this discription",item);
                    foundItem=item;
                }
            }
        }
    }
    GT_CHECK_RESULT(foundItem!=NULL,"Item \"" + itemName + "\" not found in treeWidget",NULL);
    return foundItem;//added to fix a warning
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getVisibleSamples"
QList<QTreeWidgetItem*> GTUtilsWorkflowDesigner::getVisibleSamples(U2OpStatus &os){
    QTreeWidget* w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"samples"));
    GT_CHECK_RESULT(w!=NULL,"WorkflowPaletteElements is null", QList<QTreeWidgetItem*>());

    QList<QTreeWidgetItem*> outerList = w->findItems("",Qt::MatchContains);
    QList<QTreeWidgetItem*> resultList;
    for (int i=0;i<outerList.count();i++){
        QList<QTreeWidgetItem*> innerList;

        for(int j=0;j<outerList.value(i)->childCount();j++ ){
           innerList.append(outerList.value(i)->child(j));
        }

        foreach(QTreeWidgetItem* item, innerList){
            if(!item->isHidden()){
                resultList.append(item);
            }
        }
    }
    return resultList;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addAlgorithm"
void GTUtilsWorkflowDesigner::addAlgorithm(U2OpStatus &os, QString algName){
    expandTabs(os);
    QTabWidget* tabs = qobject_cast<QTabWidget*>(GTWidget::findWidget(os,"tabs"));
    GT_CHECK(tabs!=NULL, "tabs widget not found");

    GTTabWidget::setCurrentIndex(os,tabs,0);

    QTreeWidgetItem *alg = findTreeItem(os, algName,algoriths);
    GTGlobals::sleep(100);
    GT_CHECK(alg!=NULL,"algorithm is NULL");

    selectAlgorithm(os,alg);
    QWidget* w = GTWidget::findWidget(os,"sceneView");

    int workerNum = getWorkers(os).size();
    QPoint p(w->rect().topLeft() + QPoint(100+300*(workerNum-(workerNum/2)*2),100 + 200*(workerNum/2)));//shifting workers position
    GTWidget::click(os, w,Qt::LeftButton, p);
    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "selectAlgorithm"
void GTUtilsWorkflowDesigner::selectAlgorithm(U2OpStatus &os, QTreeWidgetItem* algorithm){
    GT_CHECK(algorithm!=NULL, "algorithm is NULL");
    GTGlobals::sleep(500);

    QTreeWidget *w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"WorkflowPaletteElements"));
    QList<QTreeWidgetItem*> childrenList = w->findItems("",Qt::MatchContains);
    foreach(QTreeWidgetItem* child,childrenList){
        child->setExpanded(false);
    }

    algorithm->parent()->setExpanded(true);
    GTMouseDriver::moveTo(os,GTTreeWidget::getItemCenter(os,algorithm));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addSample"
void GTUtilsWorkflowDesigner::addSample(U2OpStatus &os, QString sampName){
    expandTabs(os);
    QTabWidget* tabs = qobject_cast<QTabWidget*>(GTWidget::findWidget(os,"tabs"));
    GT_CHECK(tabs!=NULL, "tabs widget not found");

    GTTabWidget::setCurrentIndex(os,tabs,1);

    QTreeWidgetItem *samp = findTreeItem(os, sampName,samples);
    GTGlobals::sleep(100);
    GT_CHECK(samp!=NULL,"sample is NULL");

    selectSample(os,samp);
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "selectSample"
void GTUtilsWorkflowDesigner::selectSample(U2OpStatus &os, QTreeWidgetItem* sample){
    GT_CHECK(sample!=NULL, "sample is NULL");
    GTGlobals::sleep(500);

    QTreeWidget *w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"samples"));
    QList<QTreeWidgetItem*> childrenList = w->findItems("",Qt::MatchContains);
    foreach(QTreeWidgetItem* child,childrenList){
        child->setExpanded(false);
    }

    sample->parent()->setExpanded(true);
    GTMouseDriver::moveTo(os,GTTreeWidget::getItemCenter(os,sample));
    GTMouseDriver::doubleClick(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "expandTabs"
void GTUtilsWorkflowDesigner::expandTabs(U2OpStatus &os){
    QSplitter* splitter = qobject_cast<QSplitter*>(GTWidget::findWidget(os,"splitter"));
    GT_CHECK(splitter, "splitter not found");
    QList<int> s;
    s  = splitter->sizes();

    if(s.first()==0){//expands tabs if collapsed
        QPoint p;
        p.setX(splitter->geometry().left()+2);
        p.setY(splitter->geometry().center().y());
        GTMouseDriver::moveTo(os, p);
        GTGlobals::sleep(300);
        GTMouseDriver::press(os);
        p.setX(p.x()+200);
        GTMouseDriver::moveTo(os,p);
        GTMouseDriver::release(os);
    }
}
#undef GT_METHOD_NAME

QPoint GTUtilsWorkflowDesigner::getItemCenter(U2OpStatus &os,QString itemName){
    QRect r = getItemRect(os, itemName);
    QPoint p = r.center();
    return p;
}

int GTUtilsWorkflowDesigner::getItemLeft(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.left();
    return i;
}

int GTUtilsWorkflowDesigner::getItemRight(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.right();
    return i;
}

int GTUtilsWorkflowDesigner::getItemTop(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.top();
    return i;
}

int GTUtilsWorkflowDesigner::getItemBottom(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.bottom();
    return i;
}

#define GT_METHOD_NAME "getWorker"
WorkflowProcessItem* GTUtilsWorkflowDesigner::getWorker(U2OpStatus &os,QString itemName,const GTGlobals::FindOptions &options){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    GT_CHECK_RESULT(sceneView, "sceneView not found", NULL);
    QList<QGraphicsItem *> items = sceneView->items();

    foreach(QGraphicsItem* it, items) {
        QGraphicsObject *itObj = it->toGraphicsObject();

        QGraphicsTextItem* textItemO = qobject_cast<QGraphicsTextItem*>(itObj);
        if (textItemO) {
            QString text = textItemO->toPlainText();
            if (text.contains(itemName,Qt::CaseInsensitive)) {
                if(qgraphicsitem_cast<WorkflowProcessItem*>(it->parentItem()->parentItem()))
                    return (qgraphicsitem_cast<WorkflowProcessItem*>(it->parentItem()->parentItem()));
            }
        }
    }
    GT_CHECK_RESULT(options.failIfNull == false, "Item " + itemName + " not found at scene", NULL);

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPortById"
WorkflowPortItem* GTUtilsWorkflowDesigner::getPortById(U2OpStatus &os, WorkflowProcessItem *worker, QString id){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    GT_CHECK_RESULT(sceneView,"sceneView not found",NULL)
    QList<WorkflowPortItem*> list = worker->getPortItems();
    foreach(WorkflowPortItem* p, list){
        if(p&&p->getPort()->getId()==id){
            return p;
        }
    }
    GT_CHECK_RESULT(false, "port with id " + id + "not found",NULL);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemRect"
QRect GTUtilsWorkflowDesigner::getItemRect(U2OpStatus &os,QString itemName){
//TODO: support finding items when there are several similar workers in scheme
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    QList<QGraphicsItem *> items = sceneView->items();

    foreach(QGraphicsItem* it, items) {
        QGraphicsObject *itObj = it->toGraphicsObject();

        QGraphicsTextItem* textItemO = qobject_cast<QGraphicsTextItem*>(itObj);
        if (textItemO) {
            QString text = textItemO->toPlainText();
            if (text.contains(itemName,Qt::CaseInsensitive)) {
                return GTGraphicsItem::getGraphicsItemRect(os, it);
            }
        }
    }
   return QRect();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "connect"
void GTUtilsWorkflowDesigner::connect(U2OpStatus &os, WorkflowProcessItem * from , WorkflowProcessItem * to){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(from->scene()->views().at(0));
    GT_CHECK(sceneView,"sceneView not found")
    QList<WorkflowPortItem*> fromList = from->getPortItems();
    QList<WorkflowPortItem*> toList = to->getPortItems();

    foreach(WorkflowPortItem* fromPort, fromList){
        foreach(WorkflowPortItem* toPort, toList){
            if(fromPort->getPort()->canBind(toPort->getPort())){
                GTMouseDriver::moveTo(os,GTGraphicsItem::getItemCenter(os,fromPort));
                GTMouseDriver::press(os);
                GTMouseDriver::moveTo(os,GTGraphicsItem::getItemCenter(os,toPort));
                GTMouseDriver::release(os);
                return;
            }
        }
    }

    GT_CHECK(false,"no suitable ports to connect");
}
#undef GT_METHOD_NAME

QList<WorkflowProcessItem*> GTUtilsWorkflowDesigner::getWorkers(U2OpStatus &os){
    QList<WorkflowProcessItem*> result;
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    QList<QGraphicsItem *> items = sceneView->items();
    foreach(QGraphicsItem* it, items){
        WorkflowProcessItem* worker = qgraphicsitem_cast<WorkflowProcessItem*>(it);
        if(worker)
            result.append(worker);
    }
    return result;
}

#define GT_METHOD_NAME "setDatasetInputFile"
void GTUtilsWorkflowDesigner::setDatasetInputFile(U2OpStatus &os, QString filePath, QString fileName){
    QWidget* DatasetWidget = GTWidget::findWidget(os, "DatasetWidget");
    GT_CHECK(DatasetWidget, "DatasetWidget not found");

    QWidget* addFileButton = GTWidget::findWidget(os, "addFileButton", DatasetWidget);
    GT_CHECK(addFileButton, "addFileButton not found");

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, filePath, fileName, GTFileDialogUtils::Open, GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, ob);

    GTWidget::click(os, addFileButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "createDataset"
void GTUtilsWorkflowDesigner::createDataset(U2OpStatus &os, QString datasetName){
    QWidget* DatasetWidget = GTWidget::findWidget(os, "DatasetWidget");
    GT_CHECK(DatasetWidget, "DatasetWidget not found");

    QWidget* plusButton = GTWidget::findButtonByText(os, "+");
    GT_CHECK(plusButton, "plusButton not found");

    GTUtilsDialog::waitForDialog(os, new DatasetNameEditDialogFiller(os, datasetName));

    GTWidget::click(os, plusButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setDatasetInputFolder"
void GTUtilsWorkflowDesigner::setDatasetInputFolder(U2OpStatus &os, QString filePath){
    QWidget* DatasetWidget = GTWidget::findWidget(os, "DatasetWidget");
    GT_CHECK(DatasetWidget, "DatasetWidget not found");

    QWidget* addDirButton = GTWidget::findWidget(os, "addDirButton", DatasetWidget);
    GT_CHECK(addDirButton, "addFileButton not found");

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, filePath, "", GTFileDialogUtils::Choose, GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, ob);

    GTWidget::click(os, addDirButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setParameter"
void GTUtilsWorkflowDesigner::setParameter(U2OpStatus &os, QString parameter, QVariant value, valueType type){
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    CHECK_SET_ERR(table,"tableView not found");

    //FIND CELL
    QAbstractItemModel* model = table->model();
    int iMax = model->rowCount();
    int row = -1;
    for(int i = 0; i<iMax; i++){
        QString s = model->data(model->index(i,0)).toString();
        if (s.contains(parameter,Qt::CaseInsensitive))
            row = i;
    }
    GT_CHECK(row != -1, "parameter not found");
    table->scrollTo(model->index(row,1));
    GTMouseDriver::moveTo(os,GTTableView::getCellPosition(os,table,1,row));
    GTMouseDriver::click(os);
    GTGlobals::sleep(500);

    //SET VALUE
    bool ok = true;
    switch(type){
    case(spinValue):{
        int spinVal = value.toInt(&ok);
        GT_CHECK(ok,"Wrong input. Int requaered for GTUtilsWorkflowDesigner::spinValue")
        QSpinBox* box = qobject_cast<QSpinBox*>(table->findChild<QSpinBox*>());
        GT_CHECK(box, "spinBox not found. Widget in this cell might be not QSpinBox");
        GTSpinBox::setValue(os, box, spinVal, GTGlobals::UseKeyBoard);
        break;
    }
    case(doubleSpinValue):{
        double spinVal = value.toDouble(&ok);
        GT_CHECK(ok,"Wrong input. Double requaered for GTUtilsWorkflowDesigner::doubleSpinValue")
        QDoubleSpinBox* box = qobject_cast<QDoubleSpinBox*>(table->findChild<QDoubleSpinBox*>());
        GT_CHECK(box, "QDoubleSpinBox not found. Widget in this cell might be not QDoubleSpinBox");
        GTDoubleSpinbox::setValue(os, box, spinVal, GTGlobals::UseKeyBoard);
        break;
    }
    case(comboValue):{
        int comboVal = value.toInt(&ok);
        GT_CHECK(ok,"Wrong input. Int requaered for GTUtilsWorkflowDesigner::ComboValue")
        QComboBox* box = qobject_cast<QComboBox*>(table->findChild<QComboBox*>());
        GT_CHECK(box, "QComboBox not found. Widget in this cell might be not QComboBox");
        GTComboBox::setCurrentIndex(os, box, comboVal);
        break;
    }
    case(textValue):{
        QString lineVal = value.toString();
        QLineEdit* line = qobject_cast<QLineEdit*>(table->findChild<QLineEdit*>());
        GT_CHECK(line, "QLineEdit not found. Widget in this cell might be not QLineEdit");
        GTLineEdit::setText(os, line, lineVal);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    }
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getParameter"
QString GTUtilsWorkflowDesigner::getParameter(U2OpStatus &os, QString parameter){
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    GT_CHECK_RESULT(table,"tableView not found", "");

    QAbstractItemModel* model = table->model();
    GT_CHECK_RESULT(model,"model not found", "");
    int iMax = model->rowCount();
    int row = -1;
    for(int i = 0; i<iMax; i++){
        QString s = model->data(model->index(i,0)).toString();
        if (s.contains(parameter,Qt::CaseInsensitive))
            row = i;
    }
    GT_CHECK_RESULT(row != -1, "parameter not found","");

    QVariant var = model->data(model->index(row,1));
    return var.toString();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setParameterScripting"
void GTUtilsWorkflowDesigner::setParameterScripting(U2OpStatus &os, QString parameter, QString scriptMode){
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    CHECK_SET_ERR(table,"tableView not found");

    //FIND CELL
    QAbstractItemModel* model = table->model();
    int iMax = model->rowCount();
    int row = -1;
    for(int i = 0; i<iMax; i++){
        QString s = model->data(model->index(i,0)).toString();
        if (s.contains(parameter,Qt::CaseInsensitive))
            row = i;
    }
    GT_CHECK(row != -1, "parameter not found");
    table->scrollTo(model->index(row,1));
    GTMouseDriver::moveTo(os,GTTableView::getCellPosition(os,table,2,row));
    GTMouseDriver::click(os);
    GTGlobals::sleep(500);

    //SET VALUE
    QComboBox* box = qobject_cast<QComboBox*>(table->findChild<QComboBox*>());
    GT_CHECK(box, "QComboBox not found. Scripting might be unavaluable for this parameter");
    GTComboBox::setIndexWithText(os, box, scriptMode);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkErrorList"
int GTUtilsWorkflowDesigner::checkErrorList(U2OpStatus &os, QString error){
    QListWidget* w = qobject_cast<QListWidget*>(GTWidget::findWidget(os,"infoList"));
    GT_CHECK_RESULT(w, "ErrorList widget not found", 0);

    QList<QListWidgetItem *> list =  w->findItems(error,Qt::MatchContains);
    return list.size();
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
} // namespace
