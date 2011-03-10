#ifndef _U2_WEIGHT_MATRIX_SET_PARAMETERS_DIALOG_CONTROLLER_H_
#define _U2_WEIGHT_MATRIX_SET_PARAMETERS_DIALOG_CONTROLLER_H_

#include <ui/ui_SetDefaultParametersDialog.h>

#include <QtGui/QDialog>

#include "WeightMatrixPlugin.h"

namespace U2 {

class SetParametersDialogController : public QDialog, public Ui_SetDefaultParametersDialog {
    Q_OBJECT

public:
    SetParametersDialogController(QWidget* w = NULL);

private slots:
    void sl_onOkButton();
    void sl_onSliderMoved(int);
};

} //namespace

#endif