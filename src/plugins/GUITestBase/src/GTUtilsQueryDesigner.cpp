/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "GTUtilsQueryDesigner.h"
#include "api/GTWidget.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsMdi.h"

#include <U2View/MSAEditor.h>
#include <QTreeWidget>
#include <QGraphicsItem>
#include <QGraphicsView>

namespace U2 {
#define GT_CLASS_NAME "GTUtilsQueryDesigner"

#define GT_METHOD_NAME "findTreeItem"
QTreeWidgetItem* GTUtilsQueryDesigner::findAlgorithm(U2OpStatus &os,QString itemName){
    QTreeWidgetItem* foundItem = NULL;
    QTreeWidget *w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"palette"));

    QList<QTreeWidgetItem*> outerList = w->findItems("",Qt::MatchContains);

    for (int i=0;i<outerList.count();i++){
        QList<QTreeWidgetItem*> innerList;

        for(int j=0;j<outerList.value(i)->childCount();j++ ){
           innerList.append(outerList.value(i)->child(j));
        }

        foreach(QTreeWidgetItem* item, innerList){
            if(item->text(0)==itemName){
                foundItem = item;
            }
        }
    }
    CHECK_SET_ERR_RESULT(foundItem!=NULL,"Item is null", NULL);
    return foundItem;
}
#undef GT_METHOD_NAME

void GTUtilsQueryDesigner::addAlgorithm(U2OpStatus &os, QString algName){
    QTreeWidgetItem *w = findAlgorithm(os, algName);
    GTGlobals::sleep(100);
    CHECK_SET_ERR(w!=NULL,"algorithm is NULL");

    GTMouseDriver::moveTo(os,GTTreeWidget::getItemCenter(os,w));

    GTWidget::click(os, GTWidget::findWidget(os,"sceneView"));
}

QPoint GTUtilsQueryDesigner::getItemCenter(U2OpStatus &os,QString itemName){
    QRect r = getItemRect(os, itemName);
    QPoint p = r.center();
    return p;
}

int GTUtilsQueryDesigner::getItemLeft(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.left();
    return i;
}

int GTUtilsQueryDesigner::getItemRight(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.right();
    return i;
}

int GTUtilsQueryDesigner::getItemTop(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.top();
    return i;
}

int GTUtilsQueryDesigner::getItemBottom(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.bottom();
    return i;
}

QRect GTUtilsQueryDesigner::getItemRect(U2OpStatus &os,QString itemName){

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
   // return QRect();
}

#undef GT_CLASS_NAME
} // namespace
