#ifndef _U2_HMMCALIBRATE_DIALOG_CONTROLLER_H_
#define _U2_HMMCALIBRATE_DIALOG_CONTROLLER_H_

#include "uhmmcalibrate.h"

#include <ui/ui_HMMCalibrateDialog.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

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
    QPushButton* okButton;
    QPushButton* cancelButton;
};

} //namespace

#endif

