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

#include "TaskViewController.h"

#include <U2Core/Task.h>
#include <U2Core/Settings.h>
#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/Timer.h>

#include <QtGui/QVBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>

/* TRANSLATOR U2::TaskViewDockWidget */

//TODO: do not create subtask items until not expanded

namespace U2 {

#define SETTINGS_ROOT QString("task_view/")

TaskViewDockWidget::TaskViewDockWidget() {
    waitingIcon = QIcon(":ugene/images/hourglass.png");
    activeIcon = QIcon(":ugene/images/hourglass_go.png");
    wasErrorIcon = QIcon(":ugene/images/hourglass_err.png");
    finishedIcon  = QIcon(":ugene/images/hourglass_ok.png");

    setObjectName(DOCK_TASK_VIEW);
    setWindowTitle(tr("Tasks"));
    setWindowIcon(QIcon(":ugene/images/clock.png"));

    QVBoxLayout* l = new QVBoxLayout();
    l->setSpacing(0);
    l->setMargin(0);
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    tree = new QTreeWidget(this);
    l->addWidget(tree);
    
    buildTree();
    tree->setColumnWidth(0, 400); //TODO: save geometry!
    tree->setColumnWidth(1, 250);
    tree->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tree, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_onContextMenuRequested(const QPoint &)));
    connect(tree, SIGNAL(itemSelectionChanged()), SLOT(sl_onTreeSelectionChanged()));
    connect(tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(sl_itemDoubleClicked(QTreeWidgetItem*, int)));
    connect(tree, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(sl_itemExpanded(QTreeWidgetItem*)));


    initActions();
    updateState();
}

TaskViewDockWidget::~TaskViewDockWidget() {
    QByteArray headerState = tree->header()->saveState();
    AppContext::getSettings()->setValue(SETTINGS_ROOT + "tree_header", headerState);
}

void TaskViewDockWidget::initActions() {
    cancelTaskAction  = new QAction(tr("cancel_task_action"), this);
    cancelTaskAction->setIcon(QIcon(":ugene/images/cancel.png"));
    connect(cancelTaskAction, SIGNAL(triggered()), SLOT(sl_onCancelTask()));

    viewReportAction = new QAction(tr("view_task_report_action"), this);
    viewReportAction->setIcon(QIcon(":ugene/images/task_report.png"));
    connect(viewReportAction, SIGNAL(triggered()), SLOT(sl_onViewTaskReport()));

    removeReportAction = new QAction(tr("remove_task_report_action"), this);
    removeReportAction->setIcon(QIcon(":ugene/images/bin_empty.png"));
    connect(removeReportAction, SIGNAL(triggered()), SLOT(sl_onRemoveTaskReport()));


    TaskScheduler* s = AppContext::getTaskScheduler();
    connect(s, SIGNAL(si_topLevelTaskRegistered(Task*)), SLOT(sl_onTopLevelTaskRegistered(Task*)));
    connect(s, SIGNAL(si_topLevelTaskUnregistered(Task*)), SLOT(sl_onTopLevelTaskUnregistered(Task*)));
    connect(s, SIGNAL(si_stateChanged(Task*)), SLOT(sl_onStateChanged(Task*)));
}

void TaskViewDockWidget::updateState() {
    QList<QTreeWidgetItem*> selItems = tree->selectedItems();
    bool hasActiveTask = false;
    bool hasTaskWithReport = false;
    foreach(QTreeWidgetItem* i, selItems) {
        TVTreeItem* ti = static_cast<TVTreeItem*>(i);
        hasActiveTask = hasActiveTask || ti->task!=NULL;
        hasTaskWithReport = hasTaskWithReport || ti->reportButton!=NULL;
    }
    cancelTaskAction->setEnabled(hasActiveTask);
    viewReportAction->setEnabled(hasTaskWithReport && selItems.size() == 1);
    removeReportAction->setEnabled(hasTaskWithReport && selItems.size() == 1);
}

void TaskViewDockWidget::buildTree() {
    tree->setColumnCount(TVColumns_NumCols);
    QStringList labels;
    for(int i=0;i < TVColumns_NumCols; i++) {
        switch((TVColumns)i) {
            case TVColumns_Name:    labels<<tr("task_name_column");         break;
            case TVColumns_Desc:    labels<<tr("task_current_desc_column"); break;
            case TVColumns_Progress:labels<<tr("task_progress_column");     break;
            case TVColumns_Actions: labels<<tr("task_actions_column");      break;
            case TVColumns_NumCols:                                         break;
        }
    }
    tree->setColumnWidth(TVColumns_Actions, 50);
    tree->setHeaderLabels(labels);
    tree->setSortingEnabled(false);
    tree->setUniformRowHeights(true);
    QByteArray headerState = AppContext::getSettings()->getValue(SETTINGS_ROOT + "tree_header", QByteArray()).toByteArray();
    if (!headerState.isEmpty()) {
        tree->header()->restoreState(headerState);	
    }

    const QList<Task*>& topLevelTasks = AppContext::getTaskScheduler()->getTopLevelTasks();
    foreach(Task* t, topLevelTasks) {
        addTopLevelTask(t);
    }
}

void TaskViewDockWidget::addTopLevelTask(Task* t) {
    TVTreeItem* ti = createTaskItem(t);
    tree->addTopLevelItem(ti);
    QWidget* w = new QWidget();
    QHBoxLayout* l = new QHBoxLayout();
    l->addStretch(10);
    l->setMargin(0);
    l->setSpacing(10);
    l->addSpacing(10);
    w->setLayout(l);

    ti->cancelButton = new TVButton(ti);
    ti->cancelButton->setFlat(true);
    ti->cancelButton->setIcon(cancelTaskAction->icon());
    ti->cancelButton->setFixedSize(QSize(20, 20));
    connect(ti->cancelButton, SIGNAL(clicked()), SLOT(sl_cancelTaskByButton()));

    l->insertWidget(1, ti->cancelButton);

    tree->setItemWidget(ti, TVColumns_Actions, w);

}

TVTreeItem* TaskViewDockWidget::createTaskItem(Task* task) {
    TVTreeItem* ti = new TVTreeItem(this, task);
    
    connect(task, SIGNAL(si_subtaskAdded(Task*)), SLOT(sl_onSubtaskAdded(Task*)));
    connect(task, SIGNAL(si_progressChanged()), SLOT(sl_onTaskProgress()));
    connect(task, SIGNAL(si_descriptionChanged()), SLOT(sl_onTaskDescription()));
    
    return ti;
}

void TaskViewDockWidget::sl_itemDoubleClicked(QTreeWidgetItem *item, int column) {
    Q_UNUSED(column);
    TVTreeItem* ti = static_cast<TVTreeItem*>(item);
    if (ti!=NULL && ti->reportButton != NULL) {
        activateReport(ti);
    }
}

void TaskViewDockWidget::sl_cancelTaskByButton() {
    TVButton* b = qobject_cast<TVButton*>(sender());
    if (b->ti->task!=NULL) {
        b->ti->task->cancel();
    } else {
       removeReport(b->ti);
    }
}

TVTreeItem* TaskViewDockWidget::findItem(Task* t, bool topLevelOnly) const {
    for (int i=0, n = tree->topLevelItemCount(); i<n; i++) {
        QTreeWidgetItem* item = tree->topLevelItem(i);
        TVTreeItem* ti = static_cast<TVTreeItem*>(item);
        if (ti->task == t) {
            return ti;
        }
        if (ti->task != t->getTopLevelParentTask()) {
            continue;
        }
        if (!topLevelOnly) {
            TVTreeItem* res = findChildItem(ti, t);
            if (res != NULL) {
                return res;
            }
        }
    }
    return NULL;
}

TVTreeItem* TaskViewDockWidget::findChildItem(TVTreeItem* ti, Task* t) const {
    for (int i=0, n = ti->childCount(); i<n; i++) {
        QTreeWidgetItem* item = ti->child(i);
        TVTreeItem* cti = static_cast<TVTreeItem*>(item);
        if (cti->task == t) {
            return cti;
        }
        TVTreeItem* res = findChildItem(cti, t);
        if (res != NULL) {
            return res;
        }
    }
    return NULL;
}

void TaskViewDockWidget::sl_onTopLevelTaskRegistered(Task* t) {
    addTopLevelTask(t);
}

void TaskViewDockWidget::sl_onTopLevelTaskUnregistered(Task* t) {
    TVTreeItem* ti = findItem(t, true);
    assert(ti!=NULL);

    /*if (t->isReportingEnabled()) {
        log.info(tr("New report available from task: %1").arg(t->getTaskName()));
        
        ti->detachFromTask();
        ti->reportButton = new TVButton(ti);
        ti->reportButton->setFlat(true);
        ti->reportButton->setIcon(viewReportAction->icon());
        ti->reportButton->setFixedSize(QSize(20, 20));
        connect(ti->reportButton, SIGNAL(clicked()), SLOT(sl_activateReportByButton()));
        
        QWidget*  w = tree->itemWidget(ti, TVColumns_Actions);
        QHBoxLayout* l = (QHBoxLayout*)w->layout();
        l->insertWidget(1, ti->reportButton);

        ti->cancelButton->setIcon(removeReportAction->icon());

        emit si_reportsCountChanged();
    } else {*/
        delete ti;
    //}
}

void TaskViewDockWidget::sl_activateReportByButton() {
    TVButton* b = qobject_cast<TVButton*>(sender());
    activateReport(b->ti);
}

void TaskViewDockWidget::sl_onViewTaskReport() {
    QList<QTreeWidgetItem*> items = tree->selectedItems();
    assert(items.size() == 1);
    TVTreeItem* ti = static_cast<TVTreeItem*>(items.first());
    activateReport(ti);
}

int TaskViewDockWidget::countAvailableReports() const {
    int res = 0;
    for (int i=0, n = tree->topLevelItemCount(); i<n; i++) {
        QTreeWidgetItem* item = tree->topLevelItem(i);
        TVTreeItem* ti = static_cast<TVTreeItem*>(item);
        res+=ti->reportButton == NULL ? 0 : 1;
    }
    return res;
}

void TaskViewDockWidget::removeReport(TVTreeItem* ti) {
    assert(ti->reportButton!=NULL);
    delete ti;
    emit si_reportsCountChanged();
}

void TaskViewDockWidget::sl_onRemoveTaskReport() {
    QList<QTreeWidgetItem*> items = tree->selectedItems();
    assert(items.size() == 1);
    TVTreeItem* ti = static_cast<TVTreeItem*>(items.first());
    removeReport(ti);
}


void TaskViewDockWidget::activateReport(TVTreeItem* ti) {
    assert(ti->reportButton!=NULL);
    uiLog.details(tr("Activating task report: %1").arg(ti->taskName));

    MWMDIManager* mdi = AppContext::getMainWindow()->getMDIManager();
    MWMDIWindow* w = ti->reportWindowId!=-1 ? NULL : mdi->getWindowById(ti->reportWindowId);
    if (w != NULL) {
        mdi->activateWindow(w);
        return;
    } 
    w = new TVReportWindow(ti->taskName, ti->taskId, ti->taskReport);
    mdi->addMDIWindow(w);
    ti->reportWindowId =  w->getId();
}

void TaskViewDockWidget::sl_onSubtaskAdded(Task* sub) {
    Task* parent = qobject_cast<Task*>(sender());
    TVTreeItem* ti = findItem(parent, false);    
    if (ti == NULL) {
        return;
    }
    if (ti->isExpanded() || ti->childCount() > 0) {
        TVTreeItem* childItem = createTaskItem(sub);
        ti->addChild(childItem);
    }
    ti->updateVisual();
}

void TaskViewDockWidget::sl_onTaskProgress()  {
    Task* t = qobject_cast<Task*>(sender());
    TVTreeItem* ti = findItem(t, false);    
    if (ti==NULL) {
		if (t!=NULL){
			assert(!t->isTopLevelTask());
		}
        return;
    }
    ti->updateVisual();
}

void TaskViewDockWidget::sl_onTaskDescription()  {
    Task* t = qobject_cast<Task*>(sender());
    TVTreeItem* ti = findItem(t, false);    
    if (ti==NULL) {
        assert(!t->isTopLevelTask());
        return;
    }
    ti->updateVisual();
}

void TaskViewDockWidget::sl_onStateChanged(Task* t) {
    TVTreeItem* ti = findItem(t, false);
    if (ti == NULL) {
        assert(!t->isTopLevelTask());
        return;
    }
    if (t->isFinished()) {
        ti->wasError = t->hasErrors();
        ti->wasCanceled = t->isCanceled();
    }
    ti->updateVisual();
}

void TaskViewDockWidget::sl_onTreeSelectionChanged() {
    updateState();
}

void TaskViewDockWidget::sl_onContextMenuRequested(const QPoint & pos) {
    Q_UNUSED(pos);

    QMenu popup;
    popup.addAction(viewReportAction);
    if (removeReportAction->isEnabled()) {
        popup.addAction(removeReportAction);
    } else {
        popup.addAction(cancelTaskAction);
    }
    popup.exec(QCursor::pos());
}


void TaskViewDockWidget::sl_onCancelTask() {
    QList<QTreeWidgetItem*> items = tree->selectedItems();
    assert(!items.isEmpty());
    foreach(QTreeWidgetItem* item, items) {
        TVTreeItem* tv = static_cast<TVTreeItem*>(item);
        tv->task->cancel();
    }
}

void TaskViewDockWidget::sl_itemExpanded(QTreeWidgetItem* qi) {
    if (qi->childCount() > 0) {
        return;
    }
    TVTreeItem* ti = static_cast<TVTreeItem*>(qi);
    if (ti->task==NULL) {
        return;
    }
    if (qi->childIndicatorPolicy() != QTreeWidgetItem::ShowIndicator) { //all taskitems with subtasks have QTreeWidgetItem::ShowIndicator
        assert(ti->task == NULL || ti->task->getSubtasks().isEmpty());
        return;
    }

    const QList<Task*>& subs = ti->task->getSubtasks();
    assert(!subs.isEmpty());
    QList<QTreeWidgetItem*> newSubtaskItems;
    foreach(Task* sub, subs) {
        newSubtaskItems.append(createTaskItem(sub));
    }
    ti->addChildren(newSubtaskItems);
    ti->updateVisual();
}

void TaskViewDockWidget::selectTask(Task* t) {
    TVTreeItem* ti = findItem(t, true);
    if (ti == NULL) {
        assert(!t->isTopLevelTask());
        return;
    }
    tree->scrollToItem(ti);
    ti->setSelected(true);

}

TVReportWindow::TVReportWindow(const QString& taskName, qint64 tid, const QString& report) 
: MWMDIWindow(genWindowName(taskName)), taskId(tid)
{
    QVBoxLayout* l = new QVBoxLayout();
    l->setMargin(0);
    setLayout(l);

    textEdit = new QTextEdit();
    textEdit->setAcceptRichText(true);
    textEdit->setReadOnly(true);
    textEdit->setText(report);
    
    l->addWidget(textEdit);
    textEdit->viewport()->installEventFilter(this);
    textEdit->viewport()->setMouseTracking(true);
}

QString TVReportWindow::genWindowName(const QString& taskName) {
    return tr("report_for_%1").arg(taskName);
}


QString TVReportWindow::prepareReportHTML(Task* t) {
    assert( t->isReportingSupported() && t->isReportingEnabled() );
    QString report = "<table>";
    report+="<tr><td><center><h1>"+genWindowName(t->getTaskName())+"</h1></center><br><br></td></tr>";
    report+="<tr><td>";

    report+="<table>";
    QString status = t->hasErrors() ? tr("Failed") : t->isCanceled() ? tr("Canceled") : tr("Finished");
    report+="<tr><td width=200><b>"+tr("status")+"</b></td><td>" +status+ "</td></tr>";
    if (t->hasErrors()) {
    report+="<tr><td><b>"+tr("error:")+"</b></td><td>" + t->getError().replace('|',"&#124;") + "</td></tr>";
    }

    int msecs = GTimer::millisBetween(t->getTimeInfo().startTime, t->getTimeInfo().finishTime);
    QString time = QTime().addMSecs(msecs).toString("h.mm.ss.zzz");

    report+="<tr><td><b>"+tr("time")+"</b></td><td>" +time+ "</td></tr>";
    report+="</td></tr>";
    report+="</table>";

    report+="</td></tr><tr><td>";


    report+=t->generateReport().replace('|',"&#124;")+"<br>";
    report+="</td></tr></table>";
    return report;
}

bool TVReportWindow::eventFilter(QObject *o, QEvent *e) {
    if(e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if(me->button() == Qt::LeftButton) {
            QString url = textEdit->anchorAt(me->pos());
            if(!url.isEmpty()) {
                Task *t = AppContext::getProjectLoader()->openProjectTask(url, false);
                AppContext::getTaskScheduler()->registerTopLevelTask(t);
            }
        }
    } else if(e->type() == QEvent::MouseMove) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if(!textEdit->anchorAt(me->pos()).isEmpty()) {
            textEdit->viewport()->setCursor(Qt::PointingHandCursor);
        } else {
            textEdit->viewport()->setCursor(Qt::IBeamCursor);
        }
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////
// Tree Items
TVTreeItem::TVTreeItem(TaskViewDockWidget* _w, Task* t) : task(t), w(_w) {
    reportWindowId = -1;
    taskId = task->getTaskId();
    taskName = task->getTaskName();
    assert(!taskName.isEmpty());
    reportButton = NULL;
    cancelButton = NULL;
    wasCanceled = false;
    wasError = false;
    updateVisual();
}

void TVTreeItem::updateVisual() {
    setText(TVColumns_Name, taskName);
    
    if (task == NULL || task->isFinished()) {
        setIcon(TVColumns_Name, wasError ? w->wasErrorIcon : w->finishedIcon);
    } else {
        setIcon(TVColumns_Name, task->isRunning() ? w->activeIcon : w->waitingIcon);
        setChildIndicatorPolicy(task->getSubtasks().isEmpty() ? QTreeWidgetItem::DontShowIndicator : QTreeWidgetItem::ShowIndicator);
    }

    if (task != NULL) {
        QString state;
        if (task->isCanceled()) {
            if (task->isFinished()) {
                state = TaskViewDockWidget::tr("Canceled");
            } else {
                state = TaskViewDockWidget::tr("Canceling...");
            }
        } else {
            state = task->getStateInfo().getStateDesc();
            if (state.isEmpty() || task->isFinished()) {
                state = AppContext::getTaskScheduler()->getStateName(task);
            }
        }

        setText(TVColumns_Desc, state);
        if (task->hasErrors()) {
            setToolTip(TVColumns_Name, task->getStateInfo().getError());
            setToolTip(TVColumns_Desc, task->getStateInfo().getError());
        }

        int progress = task->getProgress();
        setText(TVColumns_Progress, progress < 0 ? "?" : QString::number(progress) + "%");
    } else {
        setText(TVColumns_Desc, TaskViewDockWidget::tr("finished"));
        setText(TVColumns_Progress, "100%");
    }
}

void TVTreeItem::detachFromTask() {
    if (task->isTopLevelTask()) {
        assert(task->isReportingEnabled());
        taskReport = TVReportWindow::prepareReportHTML(task);
    }
    if (childCount() == 0) {
        const QList<Task*>& subs = task->getSubtasks();
        QList<QTreeWidgetItem*> newSubtaskItems;
        foreach(Task* sub, subs) {
            newSubtaskItems.append(new TVTreeItem(w, sub));
        }
        addChildren(newSubtaskItems);
    }

    for (int i=0;i<childCount(); i++) {
        TVTreeItem* ci = static_cast<TVTreeItem*>(child(i));
        ci->detachFromTask();
    }
    task = NULL;
}

}//namespace
