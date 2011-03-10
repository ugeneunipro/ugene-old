#include "BuildSArraySettingsWidget.h"
#include "GenomeAlignerTask.h"


namespace U2 {

BuildSArraySettingsWidget::BuildSArraySettingsWidget(QWidget* parent) : DnaAssemblyAlgorithmBuildIndexWidget(parent) {
	setupUi(this);
	layout()->setContentsMargins(0,0,0,0);
}

QMap<QString,QVariant> BuildSArraySettingsWidget::getBuildIndexCustomSettings() {
	QMap<QString,QVariant> settings;

	if (groupBox_mismatches->isChecked()) {
		settings.insert(GenomeAlignerTask::OPTION_MISMATCHES, mismatchesAllowedSpinBox->value());
		settings.insert(GenomeAlignerTask::OPTION_IF_ABS_MISMATCHES, absRadioButton->isChecked());
		settings.insert(GenomeAlignerTask::OPTION_PERCENTAGE_MISMATCHES, percentMismatchesAllowedSpinBox->value());
	} else {
		settings.insert(GenomeAlignerTask::OPTION_MISMATCHES, 0);
		settings.insert(GenomeAlignerTask::OPTION_IF_ABS_MISMATCHES, true);
		settings.insert(GenomeAlignerTask::OPTION_PERCENTAGE_MISMATCHES, 0);
	}

	return settings;
}

QString BuildSArraySettingsWidget::getIndexFileExtension() {
	return "";
}

} //namespace
