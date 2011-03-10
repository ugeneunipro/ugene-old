#ifndef _U2_SUBST_MATRIX_DIALOG_H_
#define _U2_SUBST_MATRIX_DIALOG_H_

#include "ui_SubstMatrixDialogBase.h"

#include <U2Core/global.h>
#include <U2Core/SMatrix.h>

#include <QtGui/QDialog>

namespace U2 {

class U2VIEW_EXPORT SubstMatrixDialog: public QDialog, public Ui::SubstMatrixDialogBase {
    Q_OBJECT
public:
    SubstMatrixDialog(const SMatrix& m, QWidget* parent);

private slots:
    void sl_closeWindow();

    void sl_mouseOnCell(int row, int column);

private:

    void connectGUI();
    void prepareTable();

    int hlBorderColumn;
    int hlBorderRow;
    
    int hlInnerColumn;
    int hlInnerRow;

    SMatrix m;
};

} // namespace

#endif
