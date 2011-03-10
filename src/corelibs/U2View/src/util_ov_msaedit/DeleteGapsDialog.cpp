#include "DeleteGapsDialog.h"


namespace U2 {

DeleteGapsDialog::DeleteGapsDialog(QWidget* parent, int rowNum): QDialog(parent) {
    setupUi(this);

    allRadioButton->setChecked(true);
    absoluteSpinBox->setMinimum(1);
    absoluteSpinBox->setMaximum(rowNum);

    connect(deleteButton, SIGNAL(clicked()), SLOT(sl_onOkClicked()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(sl_onCancelClicked()));
}

void DeleteGapsDialog::sl_onOkClicked() {
    deleteMode = allRadioButton->isChecked() ? DeleteAll : (relativeRadioButton->isChecked() ? DeleteByRelativeVal : DeleteByAbsoluteVal);

    switch(deleteMode) {
        case DeleteByAbsoluteVal: value = absoluteSpinBox->value();
            break;
        case DeleteByRelativeVal: value = relativeSpinBox->value();
            break;
        default: value = 0;
    }

    accept();
}

void DeleteGapsDialog::sl_onCancelClicked() {
    reject();
}

}