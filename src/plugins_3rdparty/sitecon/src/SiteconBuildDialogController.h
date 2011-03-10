#ifndef _U2_SITECON_BUILD_DIALOG_CONTROLLER_H_
#define _U2_SITECON_BUILD_DIALOG_CONTROLLER_H_

#include <ui/ui_SiteconBuildDialog.h>
#include "SiteconAlgorithm.h"
#include <U2Core/Task.h>
#include <U2Core/MAlignment.h>
#include <QtGui/QDialog>

namespace U2 {

class LoadDocumentTask;
class SiteconPlugin;

class SiteconBuildDialogController : public QDialog, public Ui_SiteconBuildDialog {
    Q_OBJECT

public:
    SiteconBuildDialogController(SiteconPlugin* p, QWidget* w = NULL);

public slots:
    virtual void reject();

private slots:
    void sl_inFileButtonClicked();
    void sl_outFileButtonClicked();
    void sl_okButtonClicked();

    void sl_onStateChanged();
    void sl_onProgressChanged();

private:
    Task* task;
    SiteconPlugin* plug;
};

class SiteconBuildTask : public Task {
    Q_OBJECT
public:
    SiteconBuildTask(const SiteconBuildSettings& s, const MAlignment& ma, const QString& origin = QString());
    void run();
    SiteconModel getResult() const {return m;}

private:
    SiteconBuildSettings    settings;
    MAlignment              ma;
    SiteconModel            m;
};

class SiteconBuildToFileTask : public Task {
    Q_OBJECT
public:
    SiteconBuildToFileTask(const QString& inFile, const QString& outFile, const SiteconBuildSettings& s);
    virtual QList<Task*> onSubTaskFinished(Task* subTask);

private:
    LoadDocumentTask*       loadTask;
    SiteconBuildTask*       buildTask;
    QString                 outFile;
    SiteconBuildSettings    settings;
};

} //namespace

#endif

