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

#include <U2View/MSAEditor.h>
#include <QTreeWidget>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <U2Core/AppContext.h>

namespace U2 {
#define GT_CLASS_NAME "GTUtilsWorkflowDesigner"

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
    GTWidget::click(os, GTWidget::findWidget(os,"sceneView"));
    GTGlobals::sleep(1000);
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

QRect GTUtilsWorkflowDesigner::getItemRect(U2OpStatus &os,QString itemName){
//TODO: support finding items when there are several similar workers in scheme
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    QList<QGraphicsItem *> items = sceneView->items();

    foreach(QGraphicsItem* it, items) {
        QGraphicsObject *itObj = it->toGraphicsObject();

        QGraphicsTextItem* textItemO = qobject_cast<QGraphicsTextItem*>(itObj);
        if (textItemO) {
            QString text = textItemO->toPlainText();
            if (text.contains(itemName)) {
                QRectF r = it->parentItem()->boundingRect();
                QRect sceneRect = it->parentItem()->mapRectToScene(r).toRect();

                QPoint globalTopLeftPos = sceneView->mapToGlobal(sceneRect.topLeft());
                QPoint globalBottomRightPos = sceneView->mapToGlobal(sceneRect.bottomRight());
                QRect globalRect(globalTopLeftPos,globalBottomRightPos);

                return globalRect;
            }
        }
    }
   return QRect();
}

#undef GT_CLASS_NAME
} // namespace
