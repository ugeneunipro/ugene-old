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

#include <QMouseEvent>
#include <QGraphicsView>
#include <QInputDialog>
#include <QMenu>
#include <QPushButton>
#include <QTabBar>
#include <QVBoxLayout>

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
    tabBar()->installEventFilter(this);

    connect(this, SIGNAL(currentChanged(int)), SLOT(sl_showDashboard(int)));
}

void WorkflowTabView::sl_showDashboard(int idx) {
    Dashboard *db = dynamic_cast<Dashboard*>(widget(idx));
    CHECK(NULL != db, );
    db->onShow();
}

int WorkflowTabView::addDashboard(Dashboard *db) {
    if (db->getName().isEmpty()) {
        db->setName(generateName());
    }
    int idx = addTab(db, db->getName());

    CloseButton *closeButton = new CloseButton(db);
    tabBar()->setTabButton(idx, QTabBar::RightSide, closeButton);
    connect(closeButton, SIGNAL(clicked()), SLOT(sl_closeTab()));
    emit si_countChanged();
    return idx;
}

void WorkflowTabView::addDashboard(WorkflowMonitor *monitor, const QString &baseName) {
    QString name = generateName(baseName);
    int idx = addDashboard(new Dashboard(monitor, name, this));
    setCurrentIndex(idx);
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
            delete db;
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
    delete db;
    emit si_countChanged();
}

void WorkflowTabView::sl_renameTab() {
    QAction *rename = dynamic_cast<QAction*>(sender());
    CHECK(NULL != rename, );
    int idx = rename->data().toInt();
    Dashboard *db = dynamic_cast<Dashboard*>(widget(idx));
    CHECK(NULL != db, );

    bool ok = false;
    QString newName = QInputDialog::getText(this, tr("Rename Dashboard"),
        tr("New dashboard name:"), QLineEdit::Normal,
        db->getName(), &ok);
    if (ok && !newName.isEmpty()) {
        db->setName(newName);
        setTabText(idx, newName);
    }
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

bool WorkflowTabView::eventFilter(QObject *watched, QEvent *event) {
    CHECK(watched == tabBar(), false);
    CHECK(QEvent::MouseButtonRelease == event->type(), false);

    QMouseEvent *me = dynamic_cast<QMouseEvent*>(event);
    CHECK(Qt::RightButton == me->button(), false);
    int idx = tabBar()->tabAt(me->pos());
    CHECK(idx >=0 && idx < count(), false);
    QMenu m(tabBar());
    QAction *rename = new QAction(tr("Rename"), this);
    rename->setData(idx);
    connect(rename, SIGNAL(triggered()), SLOT(sl_renameTab()));
    m.addAction(rename);
    m.move(tabBar()->mapToGlobal(me->pos()));
    m.exec();
    return true;
}

} // U2
