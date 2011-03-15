/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "ProjectViewDocTree.h"

#include <U2Core/GObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Gui/ProjectTreeController.h>

namespace U2 {

ProjectViewDocTree::ProjectViewDocTree(QWidget* w) : QTreeWidget(w) 
{
    setDragDropMode(QAbstractItemView::DragDrop);
}

QMimeData * ProjectViewDocTree::mimeData( const QList<QTreeWidgetItem *> items ) const {
    if (items.size() != 1) {
        return NULL;
    }
    QMimeData * mime = NULL;
    ProjViewItem* item = (ProjViewItem*)items.first();
    if (item->isObjectItem()) {
        ProjViewObjectItem* objItem = (ProjViewObjectItem*)item;
        mime = new GObjectMimeData(objItem->obj);
    } else if (item->isDocumentItem()) {
        mime = new DocumentMimeData(static_cast<ProjViewDocumentItem*>(item)->doc);

    }
    return mime;
}


}//namespace
