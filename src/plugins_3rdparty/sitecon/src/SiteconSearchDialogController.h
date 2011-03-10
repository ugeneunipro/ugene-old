#ifndef _U2_SITECON_SEARCH_DIALOG_H_
#define _U2_SITECON_SEARCH_DIALOG_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <ui/ui_SiteconSearchDialog.h>

#include <QtCore/QList>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QCloseEvent>
#include <QtCore/QTimer>

namespace U2 {

class ADVSequenceObjectContext;
class DNASequenceObject;
class SiteconSearchTask;
class Task;
class SiteconResultItem;
class DiPropertySitecon;
class SiteconModel;

class SiteconSearchDialogController : public QDialog, public Ui_SiteconSearchDialog {
    Q_OBJECT
public:
    SiteconSearchDialogController(ADVSequenceObjectContext* ctx, QWidget *p = NULL);
    ~SiteconSearchDialogController();

public slots:
    virtual void reject();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private slots:
    
    //buttons:
    void sl_selectModelFile();
    void sl_onSaveAnnotations();
    void sl_onClearList();
    void sl_onSearch();
    void sl_onClose();

    // groups
    void sl_onTaskFinished();
    void sl_onTimer();

    void sl_onResultActivated(QTreeWidgetItem* i, int col);
    
private:
    void connectGUI();
    void updateState();
    void updateStatus();
    void updateModel(const SiteconModel& m);
    
    bool checkPrevSettings();
    void savePrevSettings();

    void runTask();

    void importResults();

private:
    ADVSequenceObjectContext*   ctx;
    U2Region                     initialSelection;
    
    SiteconModel*       model;

    SiteconSearchTask* task;
    QTimer* timer;
};

}//namespace

#endif
