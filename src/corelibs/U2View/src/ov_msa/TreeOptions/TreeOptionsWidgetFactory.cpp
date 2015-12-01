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

#include <QPixmap>

#include <U2Core/U2SafePoints.h>

#include "ov_msa/MSAEditor.h"
#include "ov_phyltree/TreeViewer.h"
#include "TreeOptionsWidget.h"

#include "TreeOptionsWidgetFactory.h"

namespace U2 {

const QString MSATreeOptionsWidgetFactory::GROUP_ID = "OP_MSA_TREES_WIDGET";
const QString MSATreeOptionsWidgetFactory::GROUP_ICON_STR = ":core/images/tree.png";
const QString MSATreeOptionsWidgetFactory::GROUP_TITLE = QString(QObject::tr("Tree Settings"));
const QString MSATreeOptionsWidgetFactory::GROUP_DOC_PAGE = "17466154";


MSATreeOptionsWidgetFactory::MSATreeOptionsWidgetFactory()
    : viewSettings(new TreeOpWidgetViewSettings)
{
    objectViewOfWidget = ObjViewType_AlignmentEditor;
}

MSATreeOptionsWidgetFactory::~MSATreeOptionsWidgetFactory() {
    delete viewSettings;
}

QWidget* MSATreeOptionsWidgetFactory::createWidget(GObjectView* objView) {
    SAFE_POINT(NULL != objView,
        QString("Internal error: unable to create widget for group '%1', object view is NULL.").arg(GROUP_ID),
        NULL);

    MSAEditor* msa = qobject_cast<MSAEditor*>(objView);
    SAFE_POINT(NULL != msa,
        QString("Internal error: unable to cast object view to MSAEditor for group '%1'.").arg(GROUP_ID),
        NULL);

    SAFE_POINT(NULL != viewSettings, "Invalid tree view settings", NULL);
    TreeOptionsWidget *treeOpWidget = new TreeOptionsWidget(msa, *viewSettings);
    connect(treeOpWidget, SIGNAL(saveViewSettings(const TreeOpWidgetViewSettings&)), SLOT(sl_onWidgetViewSaved(const TreeOpWidgetViewSettings&)));
    return treeOpWidget;
}

OPGroupParameters MSATreeOptionsWidgetFactory::getOPGroupParameters(){
    return OPGroupParameters(GROUP_ID, QPixmap(GROUP_ICON_STR), GROUP_TITLE, GROUP_DOC_PAGE);
}

void MSATreeOptionsWidgetFactory::sl_onWidgetViewSaved(const TreeOpWidgetViewSettings& settings) {
    delete viewSettings;
    viewSettings = new TreeOpWidgetViewSettings(settings);
}

const QString TreeOptionsWidgetFactory::GROUP_ID = "OP_TREES_WIDGET";
const QString TreeOptionsWidgetFactory::GROUP_ICON_STR = ":core/images/tree.png";
const QString TreeOptionsWidgetFactory::GROUP_TITLE = QString(QObject::tr("Tree Settings"));
const QString TreeOptionsWidgetFactory::GROUP_DOC_PAGE = "17466154";

TreeOptionsWidgetFactory::TreeOptionsWidgetFactory()
    : viewSettings(new TreeOpWidgetViewSettings)
{
    objectViewOfWidget = ObjViewType_PhylogeneticTree;
}

TreeOptionsWidgetFactory::~TreeOptionsWidgetFactory() {
    delete viewSettings;
}

QWidget* TreeOptionsWidgetFactory::createWidget(GObjectView* objView)
{
    SAFE_POINT(NULL != objView,
        QString("Internal error: unable to create widget for group '%1', object view is NULL.").arg(GROUP_ID),
        NULL);

    TreeViewer* treeView = qobject_cast<TreeViewer*>(objView);
    SAFE_POINT(NULL != treeView,
        QString("Internal error: unable to cast object view to TreeViewer for group '%1'.").arg(GROUP_ID),
        NULL);

    SAFE_POINT(NULL != viewSettings, "Invalid tree view settings", NULL);
    TreeOptionsWidget *treeOpWidget = new TreeOptionsWidget(treeView, *viewSettings);
    connect(treeOpWidget, SIGNAL(saveViewSettings(const TreeOpWidgetViewSettings&)), SLOT(sl_onWidgetViewSaved(const TreeOpWidgetViewSettings&)));

    return treeOpWidget;
}

OPGroupParameters TreeOptionsWidgetFactory::getOPGroupParameters(){
    return OPGroupParameters(GROUP_ID, QPixmap(GROUP_ICON_STR), GROUP_TITLE, GROUP_DOC_PAGE);
}

void TreeOptionsWidgetFactory::sl_onWidgetViewSaved(const TreeOpWidgetViewSettings& settings) {
    delete viewSettings;
    viewSettings = new TreeOpWidgetViewSettings(settings);
}

const QString AddTreeWidgetFactory::GROUP_ID = "OP_MSA_ADD_TREE_WIDGET";
const QString AddTreeWidgetFactory::GROUP_ICON_STR = ":core/images/tree.png";
const QString AddTreeWidgetFactory::GROUP_TITLE = QString(QObject::tr("Tree Settings"));
const QString AddTreeWidgetFactory::GROUP_DOC_PAGE = "17466154";

AddTreeWidgetFactory::AddTreeWidgetFactory() {
    objectViewOfWidget = ObjViewType_AlignmentEditor;
}

QWidget* AddTreeWidgetFactory::createWidget(GObjectView* objView)
{
    SAFE_POINT(NULL != objView,
        QString("Internal error: unable to create widget for group '%1', object view is NULL.").arg(GROUP_ID),
        NULL);

    MSAEditor* msa = qobject_cast<MSAEditor*>(objView);
    SAFE_POINT(NULL != msa,
        QString("Internal error: unable to cast object view to MSAEditor for group '%1'.").arg(GROUP_ID),
        NULL);

    AddTreeWidget *addTreeWidget = new AddTreeWidget(msa);
    return addTreeWidget;
}

OPGroupParameters AddTreeWidgetFactory::getOPGroupParameters(){
    return OPGroupParameters(GROUP_ID, QPixmap(GROUP_ICON_STR), GROUP_TITLE, GROUP_DOC_PAGE);
}

} // namespace
