/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "BowtieSettingsWidget.h"
#include "BowtieTask.h"


namespace U2 {

BowtieSettingsWidget::BowtieSettingsWidget(QWidget* parent) : DnaAssemblyAlgorithmMainWidget(parent) {
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);

}

QMap<QString,QVariant> BowtieSettingsWidget::getDnaAssemblyCustomSettings() {
    QMap<QString,QVariant> settings;
	
    switch(mismatchesComboBox->currentIndex()) {
	    case 0: settings.insert(BowtieTask::OPTION_N_MISMATCHES, mismatchesSpinBox->value()); break;
		case 1: settings.insert(BowtieTask::OPTION_V_MISMATCHES, mismatchesSpinBox->value()); break;
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
