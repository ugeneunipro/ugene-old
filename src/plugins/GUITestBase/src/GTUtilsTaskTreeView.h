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

#ifndef _U2_GUI_TASK_TREE_VIEW_UTILS_H_
#define _U2_GUI_TASK_TREE_VIEW_UTILS_H_

#include "api/GTGlobals.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace U2 {
class Task;

class SchedulerListener : public QObject {
    Q_OBJECT
public:
    SchedulerListener();

    int getRegisteredTaskCount() const;
    void reset();

private slots:
    void sl_taskRegistered();

private:
    int registeredTaskCount;
};

class GTUtilsTaskTreeView {
public:
    static void waitTaskFinished(U2OpStatus &os, long timeout=180000);
    static void click(U2OpStatus &os, const QString &itemName, Qt::MouseButton b = Qt::LeftButton);
    static void openView(U2OpStatus& os);
    static void toggleView(U2OpStatus& os);
    static void cancelTask(U2OpStatus& os, const QString &itemName);
    static QTreeWidgetItem* getTreeWidgetItem(U2OpStatus &os, const QString &itemName);
    static QTreeWidget* getTreeWidget(U2OpStatus &os);
    static void moveToOpenedView(U2OpStatus& os, const QString &itemName);
    static QPoint getTreeViewItemPosition(U2OpStatus &os, const QString &itemName);
    static void moveTo(U2OpStatus &os,const QString &itemName);
    static int getTopLevelTasksCount(U2OpStatus &os);
    static bool checkTask(U2OpStatus& os, const QString &itemName);
    static int countTasks(U2OpStatus& os, const QString &itemName);

    static const QString widgetName;

private:
    static QTreeWidgetItem* getTreeWidgetItem(QTreeWidget* tree, const QString &itemName);
    static QList<QTreeWidgetItem*> getTaskTreeViewItems(QTreeWidgetItem* root, bool recursively = true);
    static QString getTasksInfo(QList<U2::Task *> tasks, int level);

};

} //ns

#endif
