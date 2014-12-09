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

#ifndef GT_UTILS_WORKFLOW_DESIGNER_H
#define GT_UTILS_WORKFLOW_DESIGNER_H

#include "api/GTGlobals.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QTreeWidget>
#include <QtGui/QGraphicsItem>
#else
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QGraphicsItem>
#endif

class QTableView;

namespace U2 {
class WorkflowProcessItem;
class WorkflowPortItem;

class GTUtilsWorkflowDesigner {
public:
    enum tab {algoriths,samples};
    enum valueType{spinValue, doubleSpinValue, comboValue, textValue, ComboChecks};

    //returns algoriths or samples
    static tab currentTab(U2OpStatus &os);

    //activates the tab t
    static void setCurrentTab(U2OpStatus &os, tab t);

    //opens WorkflowDesigner
    static void openWorkflowDesigner(U2OpStatus &os);

    //opens a workflow from a file
    static void loadWorkflow(U2OpStatus &os, const QString &fileUrl);

    static void runWorkflow(U2OpStatus &os);
    static void returnToWorkflow(U2OpStatus &os);

    //reterns item from samples or algorithms tab
    static QTreeWidgetItem* findTreeItem(U2OpStatus &os, QString itemName,tab t);

    //returns item form samples tab
    static QTreeWidgetItem* findSamplesTreeItem(U2OpStatus &os, QString itemName);

    //reterns item from samples or algorithms tab
    static QList<QTreeWidgetItem*> getVisibleSamples(U2OpStatus &os);

    //adds alogritm with worker with name algName to workflow scene
    static void addAlgorithm(U2OpStatus &os, QString algName);

    static WorkflowProcessItem * addElement(U2OpStatus &os, const QString &algName);

    //adds sample scheme with name sampName to workflow scene
    static void addSample(U2OpStatus &os, QString sampName);

    //expands samples/Elements tabwidget if collapsed
    static void expandTabs(U2OpStatus &os);

    //returns center of worker on workflow scene in global coordinates
    static QPoint getItemCenter(U2OpStatus &os,QString itemName);

    static void removeItem(U2OpStatus &os,QString itemName);

    static void click(U2OpStatus &os, QString itemName,  QPoint p = QPoint(0,0));

    //returs worker placed on workflow scene which contains item name
    static WorkflowProcessItem* getWorker(U2OpStatus &os, QString itemName,const GTGlobals::FindOptions &options = GTGlobals::FindOptions());

    static bool isWorkerExtended(U2OpStatus &os, const QString &itemName);

    //returns WorkflowPortItem of worker which with ID "id"
    static WorkflowPortItem* getPortById(U2OpStatus &os,WorkflowProcessItem* worker, QString id);

    //connects worker "from" with worker "to"
    static void connect(U2OpStatus &os, WorkflowProcessItem*from, WorkflowProcessItem*to);

    //returns all workers placed on workflow scene
    static QList<WorkflowProcessItem*> getWorkers(U2OpStatus &os);

    static void createDataset(U2OpStatus& os, QString datasetName = "");
    //sets input file with path "filePath" and name "filename" dataset
    //this method should be called after selecting worker which contains dataset on scene
    static void setDatasetInputFile(U2OpStatus &os, QString filePath, QString fileName);

    static void addInputFile(U2OpStatus &os, const QString &elementName, const QString &url);

    //sets input folder with path "filePath" to dataset
    static void setDatasetInputFolder(U2OpStatus &os, QString filePath);

    //sets oneparameter worker parameter
    static void setParameter(U2OpStatus& os, QString parameter, QVariant value, valueType type, GTGlobals::UseMethod method = GTGlobals::UseMouse);
    static QStringList getAllParameters(U2OpStatus& os);

    //gets oneparameter worker parameter
    static QString getParameter(U2OpStatus& os, QString parameter);
    static bool isParameterEnabled(U2OpStatus& os, QString parameter);
    static bool isParameterRequired(U2OpStatus &os, const QString &parameter);
    static bool isParameterVisible(U2OpStatus &os, const QString &parameter);
    static void clickParameter(U2OpStatus &os, const QString &parameter);
    static QTableView *getParametersTable(U2OpStatus &os);

    static void setParameterScripting(U2OpStatus& os, QString parameter, QString scriptMode);

    //returns number of items in error list which contain "error"
    static int checkErrorList(U2OpStatus &os, QString error);

    static int getItemLeft(U2OpStatus &os, QString itemName);
    static int getItemRight(U2OpStatus &os, QString itemName);
    static int getItemTop(U2OpStatus &os, QString itemName);
    static int getItemBottom(U2OpStatus &os, QString itemName);

    // breakpoints
    static void toggleDebugMode(U2OpStatus &os, bool enable = true);
    static void toggleBreakpointManager(U2OpStatus &os);
    static void setBreakpoint(U2OpStatus &os, const QString &itemName);
    static QStringList getBreakpointList(U2OpStatus &os);

private:
    static void selectAlgorithm(U2OpStatus &os, QTreeWidgetItem *algorithm);
    static void selectSample(U2OpStatus &os, QTreeWidgetItem *sample);
    static QRect getItemRect(U2OpStatus &os,QString itemName);
};

} // namespace
Q_DECLARE_METATYPE( QAction* )
#endif // GT_UTILS_WORKFLOW_DESIGNER_H
