#include "ButtonSettingsDialog.h"
#include <QtGui/QColorDialog>

namespace U2 {

QColor ButtonSettings::defaultColor = QColor(0, 0, 0);
int ButtonSettings::defaultRadius = 2;

ButtonSettings::ButtonSettings() {

    col = defaultColor;
    radius = defaultRadius;
}

ButtonSettingsDialog::ButtonSettingsDialog(QWidget *parent, const ButtonSettings &buttonSettings)
: QDialog(parent), settings(buttonSettings), changedSettings(buttonSettings) {

    setupUi(this);

    radiusSpinBox->setValue(settings.radius);
    updateColorButton();

    connect(colorButton, SIGNAL(clicked()), SLOT(sl_colorButton()));
}

void ButtonSettingsDialog::updateColorButton() {

    static const QString COLOR_STYLE("QPushButton { background-color : %1;}");
    colorButton->setStyleSheet(COLOR_STYLE.arg(changedSettings.col.name()));
}

void ButtonSettingsDialog::sl_colorButton() {

    QColor newColor = QColorDialog::getColor(changedSettings.col, this);
    if (newColor.isValid()) {
        changedSettings.col = newColor;
        updateColorButton();
    }
}

void ButtonSettingsDialog::accept() {

    changedSettings.radius = radiusSpinBox->value();

    settings = changedSettings;
    QDialog::accept();
}

ButtonSettings ButtonSettingsDialog::getSettings() const {

    return settings;
}

} //namespace
