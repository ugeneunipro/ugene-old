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

#include <U2Core/U2SafePoints.h>

#include <U2Designer/Dashboard.h>

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
: QTabWidget(parent), runsCounter(0)
{
    setTabPosition(QTabWidget::North);
    tabBar()->setShape(QTabBar::TriangularNorth);
    tabBar()->setMovable(true);

    setDocumentMode(true);
}

void WorkflowTabView::addDashboard(WorkflowMonitor *monitor) {
    runsCounter++;
    Dashboard *db = new Dashboard(monitor, this);
    int idx = addTab(db, tr("Run %1").arg(runsCounter));
    setCurrentIndex(idx);

    CloseButton *closeButton = new CloseButton(db);
    tabBar()->setTabButton(idx, QTabBar::RightSide, closeButton);
    connect(closeButton, SIGNAL(clicked()), SLOT(sl_closeTab()));
    emit si_countChanged();
}

bool WorkflowTabView::hasDashboards() const {
    return count() > 0;
}

void WorkflowTabView::sl_closeTab() {
    CloseButton *button = dynamic_cast<CloseButton*>(sender());
    SAFE_POINT(NULL != button, "NULL close button", );
    int idx = indexOf(button->content());
    removeTab(idx);
    emit si_countChanged();
}

} // U2
