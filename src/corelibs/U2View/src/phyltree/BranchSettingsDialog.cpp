#include "BranchSettingsDialog.h"
#include <QtGui/QColorDialog>

namespace U2 {

BranchSettingsDialog::BranchSettingsDialog(QWidget *parent, const BranchSettings &branchSettings)
: QDialog(parent), settings(branchSettings), changedSettings(branchSettings) {

    setupUi(this);

    thicknessSpinBox->setValue(settings.branchThickness);
    updateColorButton();

    connect(colorButton, SIGNAL(clicked()), SLOT(sl_colorButton()));
}

void BranchSettingsDialog::updateColorButton() {

    static const QString COLOR_STYLE("QPushButton { background-color : %1;}");
    colorButton->setStyleSheet(COLOR_STYLE.arg(changedSettings.branchColor.name()));
}

void BranchSettingsDialog::sl_colorButton() {

    QColor newColor = QColorDialog::getColor(changedSettings.branchColor, this);
    if (newColor.isValid()) {
        changedSettings.branchColor = newColor;
        updateColorButton();
    }
}

void BranchSettingsDialog::accept() {

    changedSettings.branchThickness = thicknessSpinBox->value();

    settings = changedSettings;
    QDialog::accept();
}

BranchSettings BranchSettingsDialog::getSettings() const {

    return settings;
}

} //namespace
