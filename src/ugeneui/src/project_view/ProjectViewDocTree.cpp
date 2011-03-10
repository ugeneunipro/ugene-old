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
