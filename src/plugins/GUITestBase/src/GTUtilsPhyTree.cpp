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

#include "GTUtilsPhyTree.h"
#include "api/GTWidget.h"
#include "math.h"
#include "QtGui/QGraphicsItem"
namespace U2
{
const int widthMin = 6;
const int widthMax = 8;
#define GT_CLASS_NAME "GTUtilsPhyTree"
#define GT_METHOD_NAME "getNodes"
QList<QGraphicsItem*> GTUtilsPhyTree::getNodes(U2OpStatus &os, int width){
    QList<QGraphicsItem*> result;
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    GT_CHECK_RESULT(treeView, "treeView not found", result);

    QList<QGraphicsItem*> list = treeView->scene()->items();
    foreach(QGraphicsItem* item, list){
        if(item->boundingRect().width()==width){
            result.append(item);
        }
    }
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedNodes"
QList<QGraphicsItem*> GTUtilsPhyTree::getSelectedNodes(U2OpStatus &os){
    return getNodes(os, widthMax);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getUnselectedNodes"
QList<QGraphicsItem*> GTUtilsPhyTree::getUnselectedNodes(U2OpStatus &os){
    return getNodes(os, widthMin);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getNodes"
QList<QGraphicsItem*> GTUtilsPhyTree::getNodes(U2OpStatus &os){
    QList<QGraphicsItem*> result;
    result.append(getSelectedNodes(os));
    result.append(getUnselectedNodes(os));
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getLabels"
QList<QGraphicsSimpleTextItem*> GTUtilsPhyTree::getLabels(U2OpStatus &os){
    QList<QGraphicsSimpleTextItem*> result;
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    GT_CHECK_RESULT(treeView, "treeView not found", result);
    QList<QGraphicsItem*> list = treeView->scene()->items();

    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem* textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem*>(item);
        if(textItem){
            bool ok;
            textItem->text().toDouble(&ok);
            if(!ok){
                result<<textItem;
            }
        }
    }
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getDistances"
QList<QGraphicsSimpleTextItem*> GTUtilsPhyTree::getDistances(U2OpStatus &os){
    QList<QGraphicsSimpleTextItem*> result;
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    GT_CHECK_RESULT(treeView, "treeView not found", result);
    QList<QGraphicsItem*> list = treeView->scene()->items();

    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem* textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem*>(item);
        if(textItem){
            bool ok;
            textItem->text().toDouble(&ok);
            if(ok){
                result<<textItem;
            }
        }
    }
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getDistancesValues"
QList<double> GTUtilsPhyTree::getDistancesValues(U2OpStatus &os){
    QList<double> result;
    QList<QGraphicsSimpleTextItem*> distList = getDistances(os);

    foreach(QGraphicsSimpleTextItem* item, distList){
        bool ok;
        QString s = item->text();
        double d = s.toDouble(&ok);
        if(ok){
            result<<d;
        }
    }

    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getLabelsText"
QStringList GTUtilsPhyTree::getLabelsText(U2OpStatus &os){
    QStringList result;
    QList<QGraphicsSimpleTextItem*> labelList = getLabels(os);

    foreach(QGraphicsSimpleTextItem* item, labelList){
        result<<item->text();
    }

    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getGlobalCoord"
QPoint GTUtilsPhyTree::getGlobalCoord(U2OpStatus& os,QGraphicsItem *item){
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    GT_CHECK_RESULT(treeView, "treeView not found", QPoint());

    QPointF sceneCoord = item->mapToScene(item->boundingRect().center());
    QPoint viewCord = treeView->mapFromScene(sceneCoord);
    QPoint globalCoord = treeView->mapToGlobal(viewCord);

    return globalCoord;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME
}
