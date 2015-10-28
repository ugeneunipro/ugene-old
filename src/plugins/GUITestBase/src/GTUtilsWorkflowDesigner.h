/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "GTGlobals.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QTreeWidget>
#include <QtGui/QGraphicsItem>
#else
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QGraphicsItem>
#endif

class QTableView;
class QTableWidget;

namespace U2 {
class WorkflowProcessItem;
class WorkflowPortItem;
class WorkflowBusItem;

class GTUtilsWorkflowDesigner {
public:
    enum tab {algoriths,samples};
    enum valueType { spinValue, doubleSpinValue, comboValue, textValue, ComboChecks, comboWithFileSelector, lineEditWithFileSelector };

    //returns algoriths or samples
    static tab currentTab(U2OpStatus &os);

    //activates the tab t
    static void setCurrentTab(U2OpStatus &os, tab t);

    //opens WorkflowDesigner
    static void openWorkflowDesigner(U2OpStatus &os);

    static void loadWorkflow(U2OpStatus &os, const QString &fileUrl);
    static void saveWorkflow(U2OpStatus &os);
    static void saveWorkflowAs(U2OpStatus &os, const QString &fileUrl, const QString &workflowName);

    static void runWorkflow(U2OpStatus &os);
    static void returnToWorkflow(U2OpStatus &os);

    //returns item from samples or algorithms tab
    static QTreeWidgetItem* findTreeItem(U2OpStatus &os, QString itemName, tab t, bool exactMatch = false, bool failIfNULL = true);

    //returns item from samples tab
    static QTreeWidgetItem* findSamplesTreeItem(U2OpStatus &os, QString itemName);

    //returns item from samples or algorithms tab
    static QList<QTreeWidgetItem*> getVisibleSamples(U2OpStatus &os);

    //expands samples/Elements tabwidget if collapsed
    static void expandTabs(U2OpStatus &os);

    static void clickOnPalette(U2OpStatus &os, const QString &itemName, Qt::MouseButton mouseButton = Qt::LeftButton);

    static QTreeWidgetItem * getPaletteGroup(U2OpStatus &os, const QString &groupName);
    static QList<QTreeWidgetItem *> getPaletteGroups(U2OpStatus &os);
    static QStringList getPaletteGroupNames(U2OpStatus &os);
    static QList<QTreeWidgetItem *> getPaletteGroupEntries(U2OpStatus &os, QTreeWidgetItem *groupItem);
    static QList<QTreeWidgetItem *> getPaletteGroupEntries(U2OpStatus &os, const QString &groupName);

    //add to scene
    static void addSample(U2OpStatus &os, const QString &sampName);
    static void addAlgorithm(U2OpStatus &os, QString algName, bool exactMatch = false);
    static WorkflowProcessItem * addElement(U2OpStatus &os, const QString &algName, bool exactMatch = false);

    //returns center of worker on workflow scene in global coordinates
    static QPoint getItemCenter(U2OpStatus &os,QString itemName);

    static void removeItem(U2OpStatus &os,QString itemName);

    static void click(U2OpStatus &os, QString itemName,  QPoint p = QPoint(0,0), Qt::MouseButton button = Qt::LeftButton);
    static void click(U2OpStatus &os, QGraphicsItem *item,  QPoint p = QPoint(0,0), Qt::MouseButton button = Qt::LeftButton);

    //returs worker placed on workflow scene which contains item name
    static WorkflowProcessItem* getWorker(U2OpStatus &os, QString itemName,const GTGlobals::FindOptions &options = GTGlobals::FindOptions());

    static QString getWorkerText(U2OpStatus &os, QString itemName, const GTGlobals::FindOptions &options = GTGlobals::FindOptions());

    static void clickLink(U2OpStatus &os, QString itemName, Qt::MouseButton button = Qt::LeftButton, int step = 10);

    static bool isWorkerExtended(U2OpStatus &os, const QString &itemName);

    //returns WorkflowPortItem of worker which with ID "id"
    static WorkflowPortItem* getPortById(U2OpStatus &os,WorkflowProcessItem* worker, QString id);

    static QList<WorkflowPortItem*> getPorts(U2OpStatus &os,WorkflowProcessItem* worker);

    //connects worker "from" with worker "to"
    static void connect(U2OpStatus &os, WorkflowProcessItem* from, WorkflowProcessItem* to);
    static void disconect(U2OpStatus &os, WorkflowProcessItem* from, WorkflowProcessItem* to);

    static QList<WorkflowBusItem*> getAllConnectionArrows(U2OpStatus &os);

    static WorkflowBusItem* getConnectionArrow(U2OpStatus &os, WorkflowProcessItem* from, WorkflowProcessItem* to);
    static QGraphicsTextItem* getArrowHint(U2OpStatus &os, WorkflowBusItem* arrow);

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
    static void setTableValue(U2OpStatus& os, QString parameter, QVariant value, valueType type, QTableWidget* table, GTGlobals::UseMethod method = GTGlobals::UseMouse);
    static QString getCellValue(U2OpStatus& os, QString parameter, QTableWidget* table);
    static void setCellValue(U2OpStatus& os, QWidget* parent, QVariant value, valueType type, GTGlobals::UseMethod method);
    static QStringList getAllParameters(U2OpStatus& os);

    static QTableWidget* getInputPortsTable(U2OpStatus &os, int index);

    //gets oneparameter worker parameter
    static QString getParameter(U2OpStatus& os, QString parameter, bool exactMatch = false);
    static bool isParameterEnabled(U2OpStatus& os, QString parameter);
    static bool isParameterRequired(U2OpStatus &os, const QString &parameter);
    static bool isParameterVisible(U2OpStatus &os, const QString &parameter);
    static void clickParameter(U2OpStatus &os, const QString &parameter);
    static QTableView *getParametersTable(U2OpStatus &os);

    static void setParameterScripting(U2OpStatus& os, QString parameter, QString scriptMode, bool exactMatch = false);

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

    static void removeCmdlineWorkerFromPalette(U2OpStatus &os, const QString &workerName);

private:
    static void selectAlgorithm(U2OpStatus &os, QTreeWidgetItem *algorithm);
    static void selectSample(U2OpStatus &os, QTreeWidgetItem *sample);
    static QRect getItemRect(U2OpStatus &os,QString itemName);
    static QTreeWidget * getCurrentTabTreeWidget(U2OpStatus &os);

    static const int verticalShift;
};

} // namespace
Q_DECLARE_METATYPE( QAction* )
#endif // GT_UTILS_WORKFLOW_DESIGNER_H
