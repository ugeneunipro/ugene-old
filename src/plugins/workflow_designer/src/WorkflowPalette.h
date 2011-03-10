#ifndef _U2_WORKFLOW_PALETTE_H_
#define _U2_WORKFLOW_PALETTE_H_

#include <U2Lang/ActorModel.h>
#include <U2Lang/ActorPrototypeRegistry.h>

#include <QtGui/QAction>
#include <QtGui/QTreeWidget>

namespace U2 {
using namespace Workflow;
class WorkflowView;
class WorkflowScene;


class WorkflowPalette : public QTreeWidget
{
    Q_OBJECT

public:
    static const QString MIME_TYPE;

    WorkflowPalette(ActorPrototypeRegistry* reg, QWidget *parent = 0);
    QMenu* createMenu(const QString& name);

    QVariant saveState() const;
    void restoreState(const QVariant&);

public slots:
    void resetSelection();

signals:
    void processSelected(Workflow::ActorPrototype*);
    void si_protoDeleted(const QString &);
    
protected:
    void contextMenuEvent(QContextMenuEvent *e);
    void mouseMoveEvent ( QMouseEvent * event );
    void mousePressEvent ( QMouseEvent * event );
    void leaveEvent ( QEvent * event );

private slots:
    void handleItemAction();
    void sl_selectProcess(bool checked = false);
    void rebuild();
    void editElement();
    void removeElement();

private:
    QTreeWidgetItem* createItemWidget(QAction* a);
    QAction* createItemAction(Workflow::ActorPrototype* item);
    void setContent(ActorPrototypeRegistry*);
    void sortTree();
private:
    QMap<QString,QList<QAction*> > categoryMap;
    QMap<QAction*, QTreeWidgetItem*> actionMap;
    QTreeWidgetItem *overItem;
    QAction* currentAction;
    QPoint dragStartPosition;
    friend class PaletteDelegate;
};


}//namespace

Q_DECLARE_METATYPE(QAction *)
Q_DECLARE_METATYPE(U2::Workflow::ActorPrototype *)

#endif
