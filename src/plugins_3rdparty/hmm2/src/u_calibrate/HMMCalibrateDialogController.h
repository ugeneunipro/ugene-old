#ifndef _U2_HMMCALIBRATE_DIALOG_CONTROLLER_H_
#define _U2_HMMCALIBRATE_DIALOG_CONTROLLER_H_

#include "uhmmcalibrate.h"

#include <ui/ui_HMMCalibrateDialog.h>

#include <QtGui/QDialog>

namespace U2 {
class Task;
class HMMCalibrateDialogController : public QDialog, public Ui_HMMCalibrateDialog {
    Q_OBJECT

public:
    HMMCalibrateDialogController(QWidget* w = NULL);

public slots:
    virtual void reject();

private slots:
    void sl_hmmFileButtonClicked();
    void sl_outFileButtonClicked();
    void sl_okButtonClicked();

    void sl_onStateChanged();
    void sl_onProgressChanged();

private:
    Task* task;
};

} //namespace

#endif

