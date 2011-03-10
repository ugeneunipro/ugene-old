#ifndef _U2_QD_GROUPS_EDITOR_H_
#define _U2_QD_GROUPS_EDITOR_H_

#include <QtGui/QTreeWidget>


namespace U2 {

class QueryViewController;

class QDGroupsEditor : public QTreeWidget {
    Q_OBJECT
public:
    QDGroupsEditor(QueryViewController* p);
protected:
    virtual void mousePressEvent(QMouseEvent *event);
private slots:
    void sl_addGroup();
    void sl_removeGroup();
    void sl_addActor();
    void sl_removeActor();
    void sl_setReqNum();
    void rebuild();
private:
    void initActions();
private:
    QueryViewController* view;

    QAction* addGroupAction;
    QAction* removeGroupAction;
    QAction* addActorAction;
    QAction* removeActorAction;
    QAction* setGroupReqNumAction;
};

}//namespace

#endif
