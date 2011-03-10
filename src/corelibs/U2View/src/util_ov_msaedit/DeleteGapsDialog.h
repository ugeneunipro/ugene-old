#ifndef _DELETE_GAPS_DIALOG_
#define _DELETE_GAPS_DIALOG_

#include "ui/ui_DeleteGapsDialog.h"


namespace U2 {

enum DeleteMode {
    DeleteByAbsoluteVal,
    DeleteByRelativeVal,
    DeleteAll
};

class DeleteGapsDialog: public QDialog, public Ui_DeleteGapsDialog {
    Q_OBJECT
public:
    DeleteGapsDialog(QWidget* parent, int alignmentLen);
    DeleteMode getDeleteMode() const {return deleteMode;}
    int getValue() const {return value;}
private slots:
    void sl_onOkClicked();
    void sl_onCancelClicked();

private:
    DeleteMode deleteMode;
    int value;
};

}

#endif