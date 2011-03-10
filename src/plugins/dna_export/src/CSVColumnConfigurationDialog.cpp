#include "CSVColumnConfigurationDialog.h"

#include <U2Core/L10n.h>
#include <U2Core/AnnotationTableObject.h>

#include "QtGui/QMessageBox"

namespace U2 {

CSVColumnConfigurationDialog::CSVColumnConfigurationDialog(QWidget* w, const ColumnConfig& _config) 
: QDialog(w), config(_config)
{
    setupUi(this);

    connect(complMarkRB, SIGNAL(toggled(bool)), SLOT(sl_complMarkToggle(bool)));
    connect(startRB, SIGNAL(toggled(bool)), SLOT(sl_startToggle(bool)));

    switch(config.role) {
        case ColumnRole_Ignore: 
            ignoreRB->setChecked(true); 
            break;
        case ColumnRole_Name: 
            nameRB->setChecked(true);
            break;
        case ColumnRole_Qualifier: 
            qualifierRB->setChecked(true);
            qualifierNameEdit->setText(config.qualifierName);
            break;
        case ColumnRole_StartPos:
            startRB->setChecked(true);
            startOffsetCheck->setChecked(config.startPositionOffset!=0);
            startOffsetValue->setValue(config.startPositionOffset);
            break;
        case ColumnRole_EndPos:
            endRB->setChecked(true);
            endInclusiveCheck->setChecked(config.endPositionIsInclusive);
            break;
        case ColumnRole_Length:
            lengthRB->setChecked(true);
            break;
        case ColumnRole_ComplMark:
            complMarkRB->setChecked(true);
            complValueEdit->setText(config.complementMark);
            complValueCheck->setChecked(!config.complementMark.isEmpty());
            break;
        default:
            assert(0);
    }
}


void CSVColumnConfigurationDialog::accept() {
    config.reset();
    if (startRB->isChecked()) {
        config.role = ColumnRole_StartPos;
        config.startPositionOffset = startOffsetCheck->isChecked() ? startOffsetValue->value() : 0;
    } else if (endRB->isChecked()) {
        config.role = ColumnRole_EndPos;
        config.endPositionIsInclusive = endInclusiveCheck->isChecked();
    } else if (lengthRB->isChecked()) {
        config.role = ColumnRole_Length;
    } else if (qualifierRB->isChecked()) {
        config.role = ColumnRole_Qualifier;
        config.qualifierName = qualifierNameEdit->text();
        if (!Annotation::isValidQualifierName(config.qualifierName)) {
            QMessageBox::critical(this, L10N::errorTitle(), tr("Invalid qualifier name!"));
            qualifierNameEdit->setFocus();
            return;
        }
    } else if (nameRB->isChecked()) {
        config.role = ColumnRole_Name;
    } else if (complMarkRB->isChecked()) {
        config.role = ColumnRole_ComplMark;
        config.complementMark = complValueCheck->isChecked() ? complValueEdit->text() : QString();
    } else {
        assert(ignoreRB->isChecked());
    }
    QDialog::accept();
}


void CSVColumnConfigurationDialog::sl_complMarkToggle(bool checked) {
    complValueCheck->setEnabled(checked);
    complValueEdit->setEnabled(checked && complValueCheck->isChecked());
}

void CSVColumnConfigurationDialog::sl_startToggle(bool checked) {
    startOffsetCheck->setEnabled(checked);
    startOffsetValue->setEnabled(checked && startOffsetCheck->isChecked());
}


} //namespace
