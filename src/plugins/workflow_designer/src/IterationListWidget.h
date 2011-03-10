#ifndef _U2_WORKFLOW_ITERATION_LIST_H_
#define _U2_WORKFLOW_ITERATION_LIST_H_

#include <U2Lang/Schema.h>
//#include <ui/ui_WorkflowEditorWidget.h>
#include <QtGui/QListView>

namespace U2 {
using namespace Workflow;
class WorkflowView;

class IterationListWidget : public QWidget
{
    Q_OBJECT
public:

    IterationListWidget(QWidget *parent);

    QList<Iteration>& list() const;
    void setList(const QList<Iteration>& lst);
    int current() const;
    QItemSelection selection() const;
    void selectIteration(int id);
    bool expandList(const ActorId& id, const QString& key, const QVariant& val);

signals:
    void iterationListAboutToChange();
    void selectionChanged();
    void listChanged();
    
private slots:
    void sl_addIteration();
    void sl_removeIteration();
    void sl_cloneIteration();
    void sl_selectIterations();
    void sl_iterationSelected();
    void sl_updateActions();
    
private:
    void setupIterationUI(QWidget*);
    void updateIterationState();

    QListView* iterationList;
    QAction * cloneAction;
    QAction * removeAction;
};


}//namespace

#endif
