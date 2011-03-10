#ifndef _U2_EXPORT_SEC_STRUCT_DIALOG_H_
#define _U2_EXPORT_SEC_STRUCT_DIALOG_H_

#include <U2Core/AnnotationData.h>
#include <ui/ui_SecStructDialog.h>

namespace U2 {

class ADVSequenceObjectContext;
class SecStructPredictTask;  
class SecStructPredictAlgRegistry;
class Task;

class U2VIEW_EXPORT SecStructDialog : public QDialog, private Ui::SecStructDialog    {
    Q_OBJECT

public:
    SecStructDialog(ADVSequenceObjectContext* ctx, QWidget *p = NULL);

private slots:
    void sl_spinRangeStartChanged(int val);
    void sl_spinRangeEndChanged(int val);
    void sl_onStartPredictionClicked();
    void sl_onTaskFinished(Task*);
    void sl_onSaveAnnotations();

private:
    int rangeStart;
    int rangeEnd;
    QList<SharedAnnotationData> results;
    ADVSequenceObjectContext* ctx;
    SecStructPredictTask* task;
    SecStructPredictAlgRegistry* sspr;
    void connectGUI();
    void updateState();
    void showResults();
};



} // namespace

#endif //  _U2_EXPORT_SEC_STRUCT_DIALOG_H_

