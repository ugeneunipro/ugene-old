#include "TreeSettingsDialog.h"

namespace U2 {

const QString TreeSettings::CLADO_TYPE = QString("Cladogram");
const QString TreeSettings::PHYLO_TYPE = QString("Phylogram");

QString TreeSettings::default_type = TreeSettings::PHYLO_TYPE;
int TreeSettings::default_width_coef = 1;
int TreeSettings::default_height_coef = 1;

TreeSettings::TreeSettings() {

    type = default_type;
    width_coef = default_width_coef;
    height_coef = default_height_coef;
   
}

TreeSettingsDialog::TreeSettingsDialog(QWidget *parent, const TreeSettings &treeSettings, bool isRectLayout)
: QDialog(parent), settings(treeSettings), changedSettings(treeSettings) {

    setupUi(this);

    heightSlider->setValue(settings.height_coef);
    widthlSlider->setValue(settings.width_coef);

    heightSlider->setEnabled(isRectLayout);

    treeViewCombo->addItem(TreeSettings::CLADO_TYPE);
    treeViewCombo->addItem(TreeSettings::PHYLO_TYPE);

    treeViewCombo->setCurrentIndex(treeViewCombo->findText(settings.type));
}

void TreeSettingsDialog::accept() {
    changedSettings.height_coef = heightSlider->value();
    changedSettings.width_coef = widthlSlider->value();

    changedSettings.type = treeViewCombo->currentText();
        
    settings = changedSettings;
    QDialog::accept();
}

TreeSettings TreeSettingsDialog::getSettings() const {

    return settings;
}

} //namespace
