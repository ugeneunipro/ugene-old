#ifndef _U2_FINDDIALOG_H_
#define _U2_FINDDIALOG_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <ui/ui_FindDialogUI.h>

#include <QtCore/QList>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QCloseEvent>
#include <QtCore/QTimer>

namespace U2 {

class ADVSequenceObjectContext;
class DNASequenceObject;
class FindAlgorithmTask;
class Task;
class FRListItem;

class U2VIEW_EXPORT FindDialog : public QDialog, public Ui_FindDialogBase {
    Q_OBJECT
    friend class FindAlgorithmTask;

public:
    static bool runDialog(ADVSequenceObjectContext* ctx);

public slots:
    virtual void reject();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private:
    FindDialog(ADVSequenceObjectContext* c);
        
private slots:
    
    //buttons:
    void sl_onSaveAnnotations();
    void sl_onClearList();
    void sl_onFindNext();
    void sl_onFindAll();
    void sl_onClose();

    //line ed
    void sl_onSearchPatternChanged(const QString&);

    // groups
    void sl_onSequenceTypeChanged();
    void sl_onStrandChanged();
    void sl_onAlgorithmChanged();

    //spin box
    void sl_onMatchPercentChanged(int);

    void sl_onTaskFinished(Task*);
    void sl_onTimer();

    //range
    void sl_onRangeStartChanged(int);
    void sl_onCurrentPosChanged(int);
    void sl_onRangeEndChanged(int);

    void sl_onResultActivated(QListWidgetItem* i, bool setPos = true);
    void sl_currentResultChanged(QListWidgetItem*, QListWidgetItem*);

    void sl_onRangeToSelection();
    void sl_onRangeToSequence();

    void sl_onRemoveOverlaps();

private:
    void connectGUI();
    void updateState();
    void updateStatus();
    void tunePercentBox();
    
    bool checkState(bool forSingleShot);
    bool checkPrevSettings();
    void savePrevSettings();

    void runTask(bool singleShot);

    void importResults();

    int getMaxErr() const;
    U2Region getCompleteSearchRegion() const;

private:
    ADVSequenceObjectContext*   ctx;
    DNASequenceObject*          sequence;

    QString prevSearchString;
    int prevAlgorithm; //0 - 100%, 1-mismatch, 2-insdel
    int prevMatch;
    FindAlgorithmTask* task;
    QTimer* timer;
    U2Region initialSelection;
};

}//namespace

#endif
