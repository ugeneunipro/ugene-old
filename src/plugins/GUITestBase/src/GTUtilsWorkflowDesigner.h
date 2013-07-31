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

#ifndef GT_UTILS_WORKFLOW_DESIGNER_H
#define GT_UTILS_WORKFLOW_DESIGNER_H

#include "api/GTGlobals.h"

#include <QTreeWidget>

class QRectF;

namespace U2 {
class WorkflowProcessItem;
class WorkflowPortItem;
class GTUtilsWorkflowDesigner {
public:
    enum tab {algoriths,samples};
    static QTreeWidgetItem* findTreeItem(U2OpStatus &os, QString itemName,tab t);
    static QTreeWidgetItem* findSamplesTreeItem(U2OpStatus &os, QString itemName);
    static void addAlgorithm(U2OpStatus &os, QString algName);
    static void addSample(U2OpStatus &os, QString sampName);
    static void selectAlgorithm(U2OpStatus &os, QTreeWidgetItem *algorithm);
    static void selectSample(U2OpStatus &os, QTreeWidgetItem *sample);
    static void expandTabs(U2OpStatus &os);

    static QPoint getItemCenter(U2OpStatus &os,QString itemName);
    static QRect getItemRect(U2OpStatus &os,QString itemName);
    static int getItemLeft(U2OpStatus &os, QString itemName);
    static int getItemRight(U2OpStatus &os, QString itemName);
    static int getItemTop(U2OpStatus &os, QString itemName);
    static int getItemBottom(U2OpStatus &os, QString itemName);
    static WorkflowProcessItem* getWorker(U2OpStatus &os, QString itemName,const GTGlobals::FindOptions &options = GTGlobals::FindOptions());
    static WorkflowPortItem* getPortById(U2OpStatus &os,WorkflowProcessItem* worker, QString id);
};

} // namespace
Q_DECLARE_METATYPE( QAction* )
#endif // GT_UTILS_WORKFLOW_DESIGNER_H
