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

#ifndef _U2_DASHBOARDSMANAGERDIALOG_H_
#define _U2_DASHBOARDSMANAGERDIALOG_H_

#include <U2Designer/Dashboard.h>

#include <ui_DashboardsManagerDialog.h>

namespace U2 {

class ScanDashboardsDirTask;

class DashboardsManagerDialog : public QDialog, public Ui::DashboardsManagerDialog {
    Q_OBJECT
public:
    DashboardsManagerDialog(ScanDashboardsDirTask *task, QWidget *parent);

    QList<DashboardInfo> selectedDashboards();
    QList<DashboardInfo> removedDashboards();

private slots:
    void sl_check();
    void sl_uncheck();
    void sl_selectAll();
    void sl_remove();

private:
    void setupList();
    QList<QTreeWidgetItem*> allItems();
    bool confirmDashboardsRemoving( ) const;

private:
    ScanDashboardsDirTask *task;
    QList<DashboardInfo> removed;
};

} // U2

#endif // _U2_DASHBOARDSMANAGERDIALOG_H_
