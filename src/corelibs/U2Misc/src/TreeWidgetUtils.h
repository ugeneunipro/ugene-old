#ifndef _U2_TREE_WIDGET_UTILS_H_
#define _U2_TREE_WIDGET_UTILS_H_

#include <U2Core/global.h>
#include <QtGui/QTreeWidget>

namespace U2{

class TreeWidgetVisitor {
public:
    virtual ~TreeWidgetVisitor(){}
    virtual bool isChildVisitRequired(QTreeWidgetItem*)  {return true;}
    virtual void visit(QTreeWidgetItem* item) = 0;
};

class U2MISC_EXPORT TreeWidgetUtils {

private:
    TreeWidgetUtils(){}

public:
    static void visitDFS(QTreeWidget* tree, TreeWidgetVisitor* visitor);
};


}

#endif
