#ifndef _U2_HMMBUILD_DIALOG_CONTROLLER_H_
#define _U2_HMMBUILD_DIALOG_CONTROLLER_H_

#include "uhmmbuild.h"

#include <ui/ui_HMMBuildDialog.h>

#include <U2Core/MAlignment.h>
#include <U2Core/Task.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

struct plan7_s;

namespace U2 {

class LoadDocumentTask;


class HMMBuildDialogController : public QDialog, public Ui_HMMBuildDialog {
    Q_OBJECT
public:
    HMMBuildDialogController(const QString& profileName, const MAlignment& ma, QWidget* p = NULL);
    
public slots:
    void reject();

private slots:
    void sl_msaFileClicked();
    void sl_resultFileClicked();
    void sl_okClicked();

    void sl_onStateChanged();
    void sl_onProgressChanged();


private:
    MAlignment  ma;
    QString     profileName;
    Task*       task;
    QPushButton* okButton;
    QPushButton* cancelButton;

};


//////////////////////////////////////////////////////////////////////////
// tasks

class HMMBuildTask: public Task {
    Q_OBJECT
public:
    HMMBuildTask(const UHMMBuildSettings& s, const MAlignment& ma);
    ~HMMBuildTask();

    void run();
    void _run();
    plan7_s* getHMM() const {return hmm;}
private:
    MAlignment ma;
    UHMMBuildSettings settings;
    plan7_s* hmm;
};

class HMMBuildToFileTask : public Task {
    Q_OBJECT
public:
    HMMBuildToFileTask(const QString& inFile, const QString& outFile, const UHMMBuildSettings& s);

    HMMBuildToFileTask(const MAlignment& ma, const QString& outFile, const UHMMBuildSettings& s);

    virtual QList<Task*> onSubTaskFinished(Task* subTask);

    QString generateReport() const;
    
    void run();
    void _run();

private:
    UHMMBuildSettings   settings;
    QString             outFile;
    MAlignment          ma;
    LoadDocumentTask*   loadTask;
    HMMBuildTask*       buildTask;
};



}//namespace
#endif
