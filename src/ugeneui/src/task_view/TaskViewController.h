#ifndef _U2_TASK_VIEW_CONTROLLER_H_
#define _U2_TASK_VIEW_CONTROLLER_H_

#include <U2Core/PluginModel.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/MainWindow.h>

#include <QtGui/QTreeWidget>
#include <QtGui/QAction>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QMouseEvent>


namespace U2 {

class Task;
class TaskViewController;


//////////////////////////////////////////////////////////////////////////
// controller
class TVTreeItem;

enum TVColumns {
    TVColumns_Name,
    TVColumns_Desc,
    TVColumns_Progress,
    TVColumns_Actions,
    TVColumns_NumCols
};

class TaskViewDockWidget: public QWidget {
	Q_OBJECT
public:
	TaskViewDockWidget();
	~TaskViewDockWidget();

    void selectTask(Task* t);
    
    int countAvailableReports() const;

    QIcon waitingIcon;
    QIcon activeIcon;
    QIcon finishedIcon;
    QIcon wasErrorIcon;

signals:
    void si_reportsCountChanged();

private slots:
	void sl_onTopLevelTaskRegistered(Task*);
	void sl_onTopLevelTaskUnregistered(Task* t);
	void sl_onStateChanged(Task* t);
    void sl_onSubtaskAdded(Task* sub);
    void sl_onTaskProgress();
    void sl_onTaskDescription();
    void sl_onContextMenuRequested(const QPoint & pos);
    void sl_onTreeSelectionChanged();
    void sl_onCancelTask();
    void sl_onViewTaskReport();
    void sl_onRemoveTaskReport();
    void sl_cancelTaskByButton();
    void sl_activateReportByButton();
    void sl_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void sl_itemExpanded(QTreeWidgetItem* i);

private:
    void activateReport(TVTreeItem* i);
    void removeReport(TVTreeItem* i);
	void initActions();
	void updateState();
	void buildTree();
	TVTreeItem* createTaskItem(Task* t);
    void addTopLevelTask(Task* t);

	TVTreeItem* findItem(Task* t, bool topLevelOnly) const ;
	TVTreeItem* findChildItem(TVTreeItem* i, Task* t) const;


	//actual widget
	QTreeWidget*    tree;
    QAction*        viewReportAction;
    QAction*        cancelTaskAction;
    QAction*        removeReportAction;
};

class TVReportWindow : public MWMDIWindow {
    Q_OBJECT
public:
    TVReportWindow(const QString& taskName, qint64 taskId, const QString& report);

    static QString genWindowName(const QString& taskName);
    static QString prepareReportHTML(Task* t);

    qint64 taskId;
    QTextEdit* textEdit;
    bool eventFilter(QObject *o, QEvent *e);
};


class TVButton;
class TVTreeItem : public QTreeWidgetItem {
public:
	TVTreeItem(TaskViewDockWidget* w, Task* t);
	
	void updateVisual();

	Task*       task;
    TaskViewDockWidget* w;

    qint64      taskId;
    QString     taskName;

    QString     taskReport;
    TVButton*   reportButton;
    TVButton*   cancelButton;
    int         reportWindowId;
    bool        wasCanceled;
    bool        wasError;
    
    void detachFromTask();
};

class TVButton : public QPushButton {
    Q_OBJECT
public:
    TVButton(TVTreeItem* t)  : ti(t){};
    TVTreeItem* ti;    
};


}//namespace

#endif
