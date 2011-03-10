#ifndef _U2_COLLOCATION_DIALOG_CONTROLLER_H_
#define _U2_COLLOCATION_DIALOG_CONTROLLER_H_

#include "CollocationsSearchAlgorithm.h"

#include <ui/ui_FindAnnotationCollocationsDialog.h>
#include <U2Core/Task.h>
#include <U2Core/U2Region.h>
#include <U2Core/AnnotationData.h>
//#include "gobjects/AnnotationTableObject.h"

#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtGui/QDialog>
#include <QtGui/QToolButton>

namespace U2 {

class ADVSequenceObjectContext;
class CollocationSearchTask;
class AnnotationTableObject;

//TODO: listen for allocation add/remove to the view
class CollocationsDialogController : public QDialog, Ui_FindAnnotationCollocationsDialog {
    Q_OBJECT
public:
    CollocationsDialogController(QStringList names, ADVSequenceObjectContext* ctx);

public slots:
    void reject();

private slots:
    void sl_searchClicked();
    void sl_cancelClicked();
    void sl_plusClicked();
    void sl_minusClicked();
    void sl_addName();
    void sl_onTaskFinished(Task*);
    void sl_onTimer();
    void sl_onResultActivated(QListWidgetItem * item );
    void sl_clearClicked();
    void sl_saveClicked();

private:
    void updateState();
    void updateStatus();
    void importResults();

    QStringList                 allNames;
    QSet<QString>               usedNames;
    ADVSequenceObjectContext*   ctx;
    QToolButton*                plusButton;
    CollocationSearchTask*      task;
    QTimer*                     timer;
};

class CDCResultItem : public QListWidgetItem {
public:
    CDCResultItem(const U2Region& _r);
    U2Region r;
};

//////////////////////////////////////////////////////////////////////////
// task

class CollocationSearchTask : public Task , public CollocationsAlgorithmListener{
    Q_OBJECT
public:
    CollocationSearchTask(const QList<AnnotationTableObject*> &table, const QSet<QString>& names, const CollocationsAlgorithmSettings& cfg);
    CollocationSearchTask(const QList<SharedAnnotationData> &table, const QSet<QString>& names, const CollocationsAlgorithmSettings& cfg);
    void run();

    QVector<U2Region> popResults();

    virtual void onResult(const U2Region& r);

private:
    CollocationsAlgorithmItem& getItem(const QString& name);
    
    QMap<QString, CollocationsAlgorithmItem> items;
    CollocationsAlgorithmSettings cfg;
    QVector<U2Region>  results;
    QMutex          lock;
};

}//namespace

#endif
