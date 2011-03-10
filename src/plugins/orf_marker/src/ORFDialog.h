#ifndef _U2_ORFDIALOG_H_
#define _U2_ORFDIALOG_H_

#include "ui/ui_ORFDialogUI.h"

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <QtCore/QList>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QCloseEvent>
#include <QtCore/QTimer>

namespace U2 {

class ADVSequenceObjectContext;
class DNASequenceObject;
class ORFFindTask;
class Task;

class ORFDialog : public QDialog, public Ui_ORFDialogBase {
    Q_OBJECT

public:
    ORFDialog(ADVSequenceObjectContext* ctx);

public slots:
    virtual void reject();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private slots:
    
    //buttons:
    void sl_onSaveAnnotations();
    void sl_onClearList();
    void sl_onFindAll();
    void sl_onClose();
    void sl_onRangeToSelection();
    void sl_onRangeToPanView();
    void sl_onRangeToSequence();

    void sl_onTaskFinished(Task*);
    void sl_onTimer();

    void sl_onResultActivated(QTreeWidgetItem* i, int col);
    void sl_translationChanged();


private:
    void connectGUI();
    void updateState();
    void updateStatus();
    void tunePercentBox();
    
    void runTask();

    void importResults();

    U2Region getCompleteSearchRegion() const;

private:
    ADVSequenceObjectContext* ctx;
    DNASequenceObject* sequence;

    U2Region panViewSelection;
    ORFFindTask* task;
    QTimer* timer;
    U2Region initialSelection;
};

}//namespace

#endif
