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

#include "MsaEditorTreeTabArea.h"
#include "../MSAEditor.h"
#include "MSAEditorTreeManager.h"

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/PhyTreeObject.h>

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QTabBar>
#include <QVBoxLayout>


namespace U2
{
MsaEditorTreeTab::MsaEditorTreeTab(MSAEditor* msa, QWidget* parent)
    : QTabWidget(parent), msa(msa), addTabButton(NULL){
    addTabButton = new QPushButton(QIcon(":/core/images/add_tree.png"), "", this);
    addTabButton->setToolTip(tr("Add existing tree"));
    setCornerWidget(addTabButton);
    connect(addTabButton, SIGNAL(clicked(bool)), this, SLOT(sl_addTabTriggered()));
    connect(this, SIGNAL(si_tabsCountChanged(int)), SLOT(updateActionsState(int)));

    tabBar()->installEventFilter(this);
    setTabsClosable(true);
    buildMenu();
    connect(this, SIGNAL(tabCloseRequested(int)), SLOT(sl_onTabCloseRequested(int)));
}
void MsaEditorTreeTab::sl_onTabCloseRequested(int index) {
    emit si_onTabCloseRequested(widget(index));
}

void MsaEditorTreeTab::buildMenu() {
    tabsMenu = new QMenu(this);
    closeOtherTabs = tabsMenu->addAction(tr("Close other tabs"));
    closeAllTabs = tabsMenu->addAction(tr("Close all tabs"));
    closeTab = tabsMenu->addAction(tr("Close tab"));
}


void MsaEditorTreeTab::updateActionsState(int tabsCount) {
    closeOtherTabs->setEnabled(tabsCount > 1);
}


int MsaEditorTreeTab::addTab(QWidget *page, const QString &label) {
    int tabIndex = QTabWidget::addTab(page, label);
    emit si_tabsCountChanged(count());
    return tabIndex;
}

bool MsaEditorTreeTab::eventFilter(QObject *target, QEvent *event)
{
    if (target == tabBar()) {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if(NULL == mouseEvent) {
            return QTabWidget::eventFilter(target, event);
        }
        if (mouseEvent->button() == Qt::RightButton) {
            menuPos = mouseEvent->pos();
            tabsMenu->popup(mouseEvent->globalPos());
            QAction* action = tabsMenu->exec(mouseEvent->globalPos());
            processMenuAction(action);
            return true;
        }
    }
    return QTabWidget::eventFilter(target, event);
}

void MsaEditorTreeTab::sl_addTabTriggered() {
    msa->getTreeManager()->openTreeFromFile();
    emit si_tabsCountChanged(count());
}

void MsaEditorTreeTab::addExistingTree() {
    msa->getTreeManager()->openTreeFromFile();
    emit si_tabsCountChanged(count());
}

void MsaEditorTreeTab::processMenuAction(QAction* triggeredAction) {
    if (triggeredAction == closeAllTabs) {
        closeAllTabsTriggered();
    } else if (triggeredAction == closeOtherTabs) {
        closeOtherTabsTriggered();
    } else if (triggeredAction == closeTab) {
        closeTabTriggered();
    }
}

void MsaEditorTreeTab::closeOtherTabsTriggered() {
    QWidget* selectedWidget = widget(tabBar()->tabAt(menuPos));
    int nextToDelete = 0;
    while(1 < count()) {
        QWidget* curWidget = widget(nextToDelete);
        if (curWidget != selectedWidget) {
            removeTab(nextToDelete);
        } else {
            nextToDelete = 1;
        }
    }
    emit si_tabsCountChanged(count());
}
void MsaEditorTreeTab::closeAllTabsTriggered() {
    while(count()) {
        removeTab(0);
    }
    emit si_tabsCountChanged(count());
}
void MsaEditorTreeTab::closeTabTriggered() {
    int index = tabBar()->tabAt(menuPos);
    removeTab(index);
    emit si_tabsCountChanged(count());
}

MsaEditorTreeTabArea::MsaEditorTreeTabArea(MSAEditor* msa, QWidget* parent)
    : QWidget(parent), addTabButton(NULL), msa(msa), treeTabWidget(NULL), currentLayout(NULL) {
    initialize();
}
void MsaEditorTreeTabArea::initialize() {
    treeTabWidget = createTabWidget();
    currentLayout = new QVBoxLayout();
    currentLayout->setMargin(0);
    currentLayout->setSpacing(0);
    currentLayout->addWidget(treeTabWidget);
    setLayout(currentLayout);
}
MsaEditorTreeTab* MsaEditorTreeTabArea::createTabWidget() {
    MsaEditorTreeTab* widget = new MsaEditorTreeTab(msa, this);
    connect(widget, SIGNAL(si_onTabCloseRequested(QWidget*)), SLOT(sl_onTabCloseRequested(QWidget*)));
    connect(widget, SIGNAL(si_tabsCountChanged(int)), SIGNAL(si_tabsCountChanged(int)));
    connect(widget, SIGNAL(si_addSplitterTriggered(Qt::Orientation, QWidget*, const QString &)), SLOT(sl_addSplitter(Qt::Orientation, QWidget*, const QString &)));
    return widget;
}
void MsaEditorTreeTabArea::sl_onTabCloseRequested(QWidget* page) {
    deleteTab(page);
}
void MsaEditorTreeTabArea::deleteTab(QWidget *page) {
    GObjectViewWindow* win = qobject_cast<GObjectViewWindow*>(page);
    const GObject* obj = win->getObjectView()->getObjects().at(0);
    Document* doc = obj->getDocument();
    GObjectReference treeRef(doc->getURLString(), "", GObjectTypes::PHYLOGENETIC_TREE);
    treeRef.objName = obj->getGObjectName();
    msa->getMSAObject()->removeObjectRelation(GObjectRelation(treeRef, ObjectRole_PhylogeneticTree));

    CHECK(-1 != treeTabWidget->indexOf(page), );
    if(NULL != page) {
        treeTabWidget->removeTab(treeTabWidget->indexOf(page));
        emit si_tabsCountChanged(treeTabWidget->count());
    }
}
void MsaEditorTreeTabArea::addTab(QWidget *page, const QString &label) {
    treeTabWidget->addTab(page, label);
    emit si_tabsCountChanged(treeTabWidget->count());
}
void MsaEditorTreeTabArea::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::white);
}

} //namespace


