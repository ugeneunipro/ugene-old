#ifndef _U2_WEIGHT_MATRIX_VIEW_MATRIX_DIALOG_CONTROLLER_H_
#define _U2_WEIGHT_MATRIX_VIEW_MATRIX_DIALOG_CONTROLLER_H_

#include <ui/ui_ViewMatrixDialog.h>

#include <QtGui/QDialog>

#include <U2Core/PFMatrix.h>
#include <U2Core/PWMatrix.h>

#include <U2View/AlignmentLogo.h>

#include "WeightMatrixPlugin.h"

namespace U2 {

class ViewMatrixDialogController : public QDialog, public Ui_ViewMatrixDialog {
    Q_OBJECT

public:
    ViewMatrixDialogController(QWidget* w = NULL);
    ViewMatrixDialogController(PFMatrix matrix, QWidget* w = NULL);
    ViewMatrixDialogController(PWMatrix matrix, QWidget* w = NULL);

private:
    AlignmentLogoRenderArea*    logoArea;
    
private slots:
    void sl_onCloseButton();
};

} //namespace

#endif