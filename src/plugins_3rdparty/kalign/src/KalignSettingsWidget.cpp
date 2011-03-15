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

#include "KalignSettingsWidget.h"
#include "KalignTask.h"


namespace U2 {

KalignSettingsWidget::KalignSettingsWidget(QWidget* parent) : MSAAlignAlgorithmMainWidget(parent) {
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);
}

QMap<QString,QVariant> KalignSettingsWidget::getMSAAlignCustomSettings() {
    QMap<QString,QVariant> settings;

    if (gapOpenCheckBox->isChecked()) {
        settings.insert(KalignMainTask::OPTION_GAP_OPEN_PENALTY, gapOpenSpinBox->value());
    }

    if (gapExtensionPenaltyCheckBox->isChecked()) {
        settings.insert(KalignMainTask::OPTION_GAP_EXTENSION_PENALTY, gapExtensionPenaltySpinBox->value());
    }

    if (terminalGapCheckBox->isChecked()) {
        settings.insert(KalignMainTask::OPTION_TERMINAL_GAP_PENALTY, terminalGapSpinBox->value());
    }

    if (bonusScoreCheckBox->isChecked()) {
        settings.insert(KalignMainTask::OPTION_BONUS_SCORE, bonusScoreSpinBox->value());
    }    
    return settings;
}
} //namespace