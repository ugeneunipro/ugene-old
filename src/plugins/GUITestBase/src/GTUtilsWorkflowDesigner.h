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
#include <QtGui/QGraphicsItem>

namespace U2 {
class WorkflowProcessItem;
class WorkflowPortItem;

class GTUtilsWorkflowDesigner {
public:
    enum tab {algoriths,samples};
    enum valueType{spinValue, doubleSpinValue, comboValue, textValue};

    //opens WorkflowDesigner
    void openWorkfolwDesigner(U2OpStatus &os);

    //reterns item from samples or algorithms tab
    static QTreeWidgetItem* findTreeItem(U2OpStatus &os, QString itemName,tab t);

    //returns item form samples tab
    static QTreeWidgetItem* findSamplesTreeItem(U2OpStatus &os, QString itemName);

    //reterns item from samples or algorithms tab
    static QList<QTreeWidgetItem*> getVisibleSamples(U2OpStatus &os);

    //adds alogritm with worker with name algName to workflow scene
    static void addAlgorithm(U2OpStatus &os, QString algName);

    //adds sample scheme with name sampName to workflow scene
    static void addSample(U2OpStatus &os, QString sampName);

    //expands samples/Elements tabwidget if collapsed
    static void expandTabs(U2OpStatus &os);

    //returns center of worker on workflow scene in global coordinates
    static QPoint getItemCenter(U2OpStatus &os,QString itemName);

    //returs worker placed on workflow scene which contains item name
    static WorkflowProcessItem* getWorker(U2OpStatus &os, QString itemName,const GTGlobals::FindOptions &options = GTGlobals::FindOptions());

    //returns WorkflowPortItem of worker which with ID "id"
    static WorkflowPortItem* getPortById(U2OpStatus &os,WorkflowProcessItem* worker, QString id);

    //connects worker "from" with worker "to"
    static void connect(U2OpStatus &os, WorkflowProcessItem*from, WorkflowProcessItem*to);

    //returns all workers placed on workflow scene
    static QList<WorkflowProcessItem*> getWorkers(U2OpStatus &os);

    //sets input file with path "filePath" and name "filename" dataset
    //this method should be called after selecting worker which contains dataset on scene
    static void setDatasetInputFile(U2OpStatus &os, QString filePath, QString fileName);

    //sets input folder with path "filePath" to dataset
    static void setDatasetInputFolder(U2OpStatus &os, QString filePath);

    //sets oneparameter worker parameter
    static void setParameter(U2OpStatus& os, QString parameter, QVariant value, valueType type);

    static void setParameterScripting(U2OpStatus& os, QString parameter, QString scriptMode);

    //returns number of items in error list which contain "error"
    static int checkErrorList(U2OpStatus &os, QString error);

    static int getItemLeft(U2OpStatus &os, QString itemName);
    static int getItemRight(U2OpStatus &os, QString itemName);
    static int getItemTop(U2OpStatus &os, QString itemName);
    static int getItemBottom(U2OpStatus &os, QString itemName);
private:
    static void selectAlgorithm(U2OpStatus &os, QTreeWidgetItem *algorithm);
    static void selectSample(U2OpStatus &os, QTreeWidgetItem *sample);
    static QRect getItemRect(U2OpStatus &os,QString itemName);
};

} // namespace
Q_DECLARE_METATYPE( QAction* )
#endif // GT_UTILS_WORKFLOW_DESIGNER_H
