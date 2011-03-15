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
