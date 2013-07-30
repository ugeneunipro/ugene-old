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

#ifndef _U2_WORKFLOWTABVIEW_H_
#define _U2_WORKFLOWTABVIEW_H_

#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>

#include <U2Designer/Dashboard.h>

#include <U2Lang/WorkflowMonitor.h>

class QGraphicsView;

namespace U2 {
using namespace Workflow;

class Dashboard;

class WorkflowTabView : public QTabWidget {
    Q_OBJECT
public:
    WorkflowTabView(QWidget *parent);

    void addDashboard(WorkflowMonitor *monitor, const QString &name = QString());
    bool hasDashboards() const;
    void updateDashboards(const QList<DashboardInfo> &dashboards);

    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void si_countChanged();

private slots:
    void sl_closeTab();
    void sl_dashboardsLoaded();
    void sl_renameTab();

private:
    void addDashboard(Dashboard *db);
    QString generateName(const QString &baseName = "") const;
    QStringList allNames() const;
};

} // U2

#endif // _U2_WORKFLOWTABVIEW_H_
