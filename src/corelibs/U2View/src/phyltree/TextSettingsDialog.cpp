#include "TextSettingsDialog.h"
#include "U2View/TreeViewerUtils.h"
#include <QtGui/QColorDialog>

namespace U2 {

QColor TextSettings::defaultColor = QColor(Qt::gray);
QFont TextSettings::defaultFont = TreeViewerUtils::getFont();

TextSettings::TextSettings() {

    textColor = defaultColor;
    textFont = defaultFont;
}

TextSettingsDialog::TextSettingsDialog(QWidget *parent, const TextSettings &textSettings)
: QDialog(parent), settings(textSettings), changedSettings(textSettings) {

    setupUi(this);

    updateColorButton();
    fontComboBox->setCurrentFont(settings.textFont);
    sizeSpinBox->setValue(settings.textFont.pointSize());

    boldToolButton->setChecked(settings.textFont.bold());
    italicToolButton->setChecked(settings.textFont.italic());
    underlineToolButton->setChecked(settings.textFont.underline());
    overlineToolButton->setChecked(settings.textFont.overline());

    connect(colorButton, SIGNAL(clicked()), SLOT(sl_colorButton()));
}

void TextSettingsDialog::updateColorButton() {

    static const QString COLOR_STYLE("QPushButton { background-color : %1;}");
    colorButton->setStyleSheet(COLOR_STYLE.arg(changedSettings.textColor.name()));
}

void TextSettingsDialog::sl_colorButton() {

    QColor newColor = QColorDialog::getColor(changedSettings.textColor, this);
    if (newColor.isValid()) {
        changedSettings.textColor = newColor;
        updateColorButton();
    }
}

void TextSettingsDialog::accept() {

    changedSettings.textFont = fontComboBox->currentFont();
    changedSettings.textFont.setPointSize(sizeSpinBox->value());

    changedSettings.textFont.setBold(boldToolButton->isChecked());
    changedSettings.textFont.setItalic(italicToolButton->isChecked());
    changedSettings.textFont.setUnderline(underlineToolButton->isChecked());
    changedSettings.textFont.setOverline(overlineToolButton->isChecked());

    settings = changedSettings;
    QDialog::accept();
}

TextSettings TextSettingsDialog::getSettings() const {

    return settings;
}

} //namespace
