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

#ifndef _U2_MSA_EDITOR_TREE_TAB_AREA_H_
#define _U2_MSA_EDITOR_TREE_TAB_AREA_H_

#include <QPushButton>
#include <QTabWidget>

#include "../MsaUpdatedWidgetInterface.h"

namespace U2
{

class MAlignmentRow;
class MSADistanceMatrix;
class MSAEditor;

class MsaEditorTreeTab : public QTabWidget {
    Q_OBJECT
public:
    MsaEditorTreeTab(MSAEditor* msa, QWidget* parent);
    virtual ~MsaEditorTreeTab() {}
    int addTab(QWidget *page, const QString &label);
    void deleteTab(QWidget *page);
    QWidget* getCurrentWidget() const {return currentWidget();}
    void addExistingTree();
public slots:
    void sl_addTabTriggered();
private slots:
    void sl_onTabCloseRequested(int index);
    void updateActionsState(int count);
signals:
    void si_onTabCloseRequested(QWidget*);
    void si_tabsCountChanged(int curTabsNumber);
private:
    void buildMenu();
    bool eventFilter(QObject *target, QEvent *event);

    void processMenuAction(QAction* triggeredAction);
    void closeOtherTabsTriggered();
    void closeAllTabsTriggered();
    void closeTabTriggered();

    MSAEditor*          msa;
    QPushButton*        addTabButton;
    QMenu*              tabsMenu;
    QPoint              menuPos;
    QAction*            closeOtherTabs;
    QAction*            closeAllTabs;
    QAction*            closeTab;
};

class MsaEditorTreeTabArea : public QWidget{
    Q_OBJECT
public:
    MsaEditorTreeTabArea(MSAEditor* msa, QWidget* parent);

    void addTab(QWidget *page, const QString &label);
    void deleteTab(QWidget *page);

    MsaEditorTreeTab* getCurrentTabWidget() const { return treeTabWidget; }
    QWidget*          getCurrentWidget() const { return treeTabWidget->getCurrentWidget(); }
protected:
    void paintEvent(QPaintEvent *);
    virtual MsaEditorTreeTab* createTabWidget();
    void initialize();
    
private slots:
    void sl_onTabCloseRequested(QWidget* page);
signals:
    void si_tabsCountChanged(int curTabsNumber);
private:
    QPushButton*                        addTabButton;
    MSAEditor*                          msa;
    MsaEditorTreeTab*                   treeTabWidget;
    QLayout*                            currentLayout;
};
} //namespace


#endif
