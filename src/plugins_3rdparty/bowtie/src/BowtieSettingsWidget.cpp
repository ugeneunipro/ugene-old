#include "BowtieSettingsWidget.h"
#include "BowtieTask.h"


namespace U2 {

BowtieSettingsWidget::BowtieSettingsWidget(QWidget* parent) : DnaAssemblyAlgorithmMainWidget(parent) {
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);
}

QMap<QString,QVariant> BowtieSettingsWidget::getDnaAssemblyCustomSettings() {
    QMap<QString,QVariant> settings;
	if(mismatchesCheckBox->isChecked())	{
		switch(mismatchesComboBox->currentIndex()) {
			case 0: settings.insert(BowtieTask::OPTION_N_MISMATCHES, mismatchesSpinBox->value()); break;
			case 1: settings.insert(BowtieTask::OPTION_V_MISMATCHES, mismatchesSpinBox->value()); break;
		}
	}
    if(maqerrCheckBox->isChecked())	{
        settings.insert(BowtieTask::OPTION_MAQERR, maqerrSpinBox->value());
    }
    if(maxbtsCheckBox->isChecked())	{
        settings.insert(BowtieTask::OPTION_MAXBTS, maxbtsSpinBox->value());
    }
    if(seedlenCheckBox->isChecked()) {
        settings.insert(BowtieTask::OPTION_SEED_LEN, seedlenSpinBox->value());
    }
    if(chunckmbsCheckBox->isChecked()) {
        settings.insert(BowtieTask::OPTION_CHUNKMBS, chunkmbsSpinBox->value());
    }
    if(seedCheckBox->isChecked()) {
        settings.insert(BowtieTask::OPTION_SEED, seedlenSpinBox->value());
    }

	settings.insert(BowtieTask::OPTION_PREBUILT_INDEX, prebuiltIndexCheckBox->isChecked());
	settings.insert(BowtieTask::OPTION_NOFW, nofwCheckBox->isChecked());
	settings.insert(BowtieTask::OPTION_NORC, norcCheckBox->isChecked());
	settings.insert(BowtieTask::OPTION_TRYHARD, tryhardCheckBox->isChecked());
    settings.insert(BowtieTask::OPTION_BEST, bestCheckBox->isChecked());
    settings.insert(BowtieTask::OPTION_ALL, allCheckBox->isChecked());
	settings.insert(BowtieTask::OPTION_NOMAQROUND, nomaqroundCheckBox->isChecked());

    return settings;
}
} //namespace
