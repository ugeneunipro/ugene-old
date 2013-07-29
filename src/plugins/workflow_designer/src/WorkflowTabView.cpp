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

#include <QGraphicsView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTabBar>

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>

#include "WorkflowTabView.h"

namespace U2 {

class CloseButton : public QPushButton {
public:
    CloseButton(QWidget *content)
        : QPushButton(QIcon(":workflow_designer/images/delete.png"), ""), _content(content)
    {
        setToolTip(tr("Close dashboard"));
        setFlat(true);
        setFixedSize(16, 16);
    }

    QWidget * content() const {
        return _content;
    }

private:
    QWidget *_content;
};

WorkflowTabView::WorkflowTabView(QWidget *parent)
: QTabWidget(parent)
{
    setTabPosition(QTabWidget::North);
    tabBar()->setShape(QTabBar::TriangularNorth);
    tabBar()->setMovable(true);

    setDocumentMode(true);
    ScanDashboardsDirTask *t = new ScanDashboardsDirTask();
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_dashboardsLoaded()));
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void WorkflowTabView::addDashboard(Dashboard *db) {
    if (db->getName().isEmpty()) {
        db->setName(generateName());
    }
    int idx = addTab(db, db->getName());
    setCurrentIndex(idx);

    CloseButton *closeButton = new CloseButton(db);
    tabBar()->setTabButton(idx, QTabBar::RightSide, closeButton);
    connect(closeButton, SIGNAL(clicked()), SLOT(sl_closeTab()));
    emit si_countChanged();
}

void WorkflowTabView::addDashboard(WorkflowMonitor *monitor, const QString &baseName) {
    QString name = generateName(baseName);
    addDashboard(new Dashboard(monitor, name, this));
}

bool WorkflowTabView::hasDashboards() const {
    return count() > 0;
}

void WorkflowTabView::updateDashboards(const QList<DashboardInfo> &dashboards) {
    QList<DashboardInfo> dbs = dashboards;
    int i = 0;
    while (i < count()) {
        Dashboard *db = dynamic_cast<Dashboard*>(widget(i));
        DashboardInfo info(db->directory());
        if (dbs.contains(info)) {
            dbs.removeOne(info);
            i++;
        } else {
            db->setClosed();
            removeTab(i);
        }
    }
    foreach (const DashboardInfo &info, dbs) {
        addDashboard(new Dashboard(info.path, this));
    }
    emit si_countChanged();
}

void WorkflowTabView::sl_closeTab() {
    CloseButton *button = dynamic_cast<CloseButton*>(sender());
    SAFE_POINT(NULL != button, "NULL close button", );
    int idx = indexOf(button->content());
    Dashboard *db = dynamic_cast<Dashboard*>(widget(idx));
    db->setClosed();
    removeTab(idx);
    emit si_countChanged();
}

void WorkflowTabView::sl_dashboardsLoaded() {
    ScanDashboardsDirTask *t = dynamic_cast<ScanDashboardsDirTask*>(sender());
    CHECK(NULL != t, );
    CHECK(t->isFinished(), );

    foreach (const QString &dbPath, t->getOpenedDashboards()) {
        addDashboard(new Dashboard(dbPath, this));
    }
}

QStringList WorkflowTabView::allNames() const {
    QStringList result;
    for (int i=0; i<count(); i++) {
        Dashboard *db = dynamic_cast<Dashboard*>(widget(i));
        result << db->getName();
    }
    return result;
}

QString WorkflowTabView::generateName(const QString &name) const {
    QString baseName = name;
    if (baseName.isEmpty()) {
        baseName = tr("Run");
    }

    QString result;
    QStringList all = allNames();
    int num = 1;
    do {
        result = baseName + QString(" %1").arg(num);
        num++;
    } while (all.contains(result));
    return result;
}

} // U2
