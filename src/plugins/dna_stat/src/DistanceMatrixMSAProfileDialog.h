#ifndef _U2_DISTANCE_MATRIX_MSA_PROFILE_DIALOG_H_
#define _U2_DISTANCE_MATRIX_MSA_PROFILE_DIALOG_H_

#include "ui/ui_DistanceMatrixMSAProfileDialog.h"

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MAlignment.h>

#include <QtCore/QHash>
#include <QtCore/QSet>

namespace U2 {

class MSAEditor;

class DistanceMatrixMSAProfileDialog : public QDialog, public Ui_DistanceMatrixMSAProfileDialog {
    Q_OBJECT

public:
    DistanceMatrixMSAProfileDialog(QWidget* p, MSAEditor* ctx);

    virtual void accept();

private slots:
    
    void sl_selectFile();
    void sl_formatChanged(bool);

private:
    MSAEditor* ctx;
};

enum DistanceMatrixMSAProfileOutputFormat {
    DistanceMatrixMSAProfileOutputFormat_Show,
    DistanceMatrixMSAProfileOutputFormat_CSV,
    DistanceMatrixMSAProfileOutputFormat_HTML
};

class DistanceMatrixMSAProfileTaskSettings {
public:
    DistanceMatrixMSAProfileTaskSettings(){ 
        outFormat = DistanceMatrixMSAProfileOutputFormat_Show; 
        usePercents = false;
    }

    QString                         algoName;    // selected algorithm
    QString                         profileName; // usually object name
    QString                         profileURL;  // document url
    MAlignment                      ma;
    bool                            usePercents; //report percents but not counts
    DistanceMatrixMSAProfileOutputFormat   outFormat;   
    QString                         outURL;    
};

class DistanceMatrixMSAProfileTask : public Task {
    Q_OBJECT
public:
    DistanceMatrixMSAProfileTask(const DistanceMatrixMSAProfileTaskSettings& s);

    virtual void prepare();
    
    QList<Task*> onSubTaskFinished(Task* subTask);
    //void run();    
    ReportResult report();

private:
    
    DistanceMatrixMSAProfileTaskSettings   s;        
    QString                         resultText;
};

}//namespace

#endif
