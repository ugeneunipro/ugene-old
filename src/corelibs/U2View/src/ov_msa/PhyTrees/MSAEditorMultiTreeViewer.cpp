/**
* UGENE - Integrated Bioinformatics Tools.
* Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
#include "MSAEditorMultiTreeViewer.h"
#include <U2View/GraphicsRectangularBranchItem.h>
#include <U2View/MSAEditorDataList.h>
#include <U2View/MSAEditor.h>
#include <U2Gui/ObjectViewModel.h>
#include <QtGui/QVBoxLayout>

namespace U2 {

MSAEditorMultiTreeViewer::MSAEditorMultiTreeViewer(QString _title, MSAEditor* _editor)
: editor(_editor) {
    treeTabs = new MSAEditorTabWidgetArea(editor, this);
    titleWidget = _editor->getUI()->createLabelWidget(_title);
    MSAWidget* title = dynamic_cast<MSAWidget*>(titleWidget);
    title->setHeightMargin(-55);
    QVBoxLayout* treeAreaLayout = new QVBoxLayout(this);
    treeAreaLayout->setMargin(0);
    treeAreaLayout->setSpacing(0);
    treeAreaLayout->addWidget(titleWidget);
    treeAreaLayout->addWidget(treeTabs);

    this->setLayout(treeAreaLayout);

    connect(treeTabs, SIGNAL(si_tabCloseRequested(QWidget*)), SLOT(sl_onTabCloseRequested(QWidget*))); 
    connect(treeTabs, SIGNAL(si_tabsCountChanged(int)), SIGNAL(si_tabsCountChanged(int)));
}

void MSAEditorMultiTreeViewer::addTreeView(GObjectViewWindow* treeView) {
    treeTabs->addTab(treeView, treeView->getViewName());
    treeViews.append(treeView);
    tabsNames.append(treeView->getViewName());
}

QWidget* MSAEditorMultiTreeViewer::getCurrentWidget() const {
    return treeTabs->getCurrentWidget();
}

UpdatedTabWidget* MSAEditorMultiTreeViewer::getCurrentTabWidget() const {
    return treeTabs->getCurrentTabWidget();
}

void MSAEditorMultiTreeViewer::sl_onTabCloseRequested(QWidget* page) {
    treeViews.removeOne(page);
    GObjectViewWindow* viewWindow = qobject_cast<GObjectViewWindow*>(page);
    if(NULL != viewWindow) {
        int i =  tabsNames.indexOf(viewWindow->getViewName());
        tabsNames.removeAt(i);
    }
}

const QStringList& MSAEditorMultiTreeViewer::getTreeNames() const {
    return tabsNames;
}

}//namespace
