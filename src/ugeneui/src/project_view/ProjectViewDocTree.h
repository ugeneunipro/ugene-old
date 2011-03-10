#ifndef _U2_PROJECT_VIEW_DOC_TREE_H_
#define _U2_PROJECT_VIEW_DOC_TREE_H_

#include <QtCore/QMimeData>
#include <QtGui/QTreeWidget>

namespace U2 {

class ProjectViewDocTree : public QTreeWidget {
public:
    ProjectViewDocTree(QWidget* w);
protected:
    virtual QMimeData * mimeData ( const QList<QTreeWidgetItem *> items ) const;
};

}//namespace

#endif
