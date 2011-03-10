#include "TreeWidgetUtils.h"

namespace U2 {

static void visitBranchDFS(QTreeWidgetItem* item, TreeWidgetVisitor* visitor) {
    if (visitor->isChildVisitRequired(item)) {
        for (int i = 0; i < item->childCount(); i++) {    
            QTreeWidgetItem* child = item->child(i);
            visitBranchDFS(child, visitor);
        }
    }
    visitor->visit(item);
}

void TreeWidgetUtils::visitDFS(QTreeWidget* tree, TreeWidgetVisitor* visitor) {
    for (int i = 0; i < tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* child = tree->topLevelItem(i);
        visitBranchDFS(child, visitor);
    }
}

} //namespace
