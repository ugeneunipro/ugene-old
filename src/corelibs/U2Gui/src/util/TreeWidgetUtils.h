/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_TREE_WIDGET_UTILS_H_
#define _U2_TREE_WIDGET_UTILS_H_

#include <U2Core/global.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QTreeWidget>
#else
#include <QtWidgets/QTreeWidget>
#endif

namespace U2{

class TreeWidgetVisitor {
public:
    virtual ~TreeWidgetVisitor(){}
    virtual bool isChildVisitRequired(QTreeWidgetItem*)  {return true;}
    virtual void visit(QTreeWidgetItem* item) = 0;
};

class U2GUI_EXPORT TreeWidgetUtils {

private:
    TreeWidgetUtils(){}

public:
    static void visitDFS(QTreeWidget* tree, TreeWidgetVisitor* visitor);
};


}

#endif
