/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>

#include "BwaSettingsWidget.h"
#include "BwaTask.h"

namespace U2 {

// BwaSettingsWidget

BwaSettingsWidget::BwaSettingsWidget(QWidget *parent):
    DnaAssemblyAlgorithmMainWidget(parent)
{
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);

    threadsSpinBox->setMaximum(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
    threadsSpinBox->setValue(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
}

QMap<QString,QVariant> BwaSettingsWidget::getDnaAssemblyCustomSettings() {
    QMap<QString, QVariant> settings;

    {
        QString algorithm;
        switch(indexAlgorithmComboBox->currentIndex()) {
        case 0:
            algorithm = "bwtsw";
            break;
        case 1:
            algorithm = "div";
            break;
        case 2:
            algorithm = "is";
            break;
        default:
            assert(false);
            break;
        }
        settings.insert(BwaTask::OPTION_INDEX_ALGORITHM, algorithm);
    }
    if(maxDiffRadioButton->isChecked()) {
        settings.insert(BwaTask::OPTION_N, maxDiffSpinBox->value());
    } else {
        settings.insert(BwaTask::OPTION_N, missingProbSpinBox->value());
    }
    settings.insert(BwaTask::OPTION_MAX_GAP_OPENS, maxGapOpensSpinBox->value());
    if(enableLongGapsCheckBox) {
        settings.insert(BwaTask::OPTION_MAX_GAP_EXTENSIONS, maxGapExtensionsSpinBox->value());
    }
    settings.insert(BwaTask::OPTION_INDEL_OFFSET, indelOffsetSpinBox->value());
    settings.insert(BwaTask::OPTION_MAX_LONG_DELETION_EXTENSIONS, maxLongDeletionExtensionsSpinBox->value());
    settings.insert(BwaTask::OPTION_SEED_LENGTH, seedLengthSpinBox->value());
    settings.insert(BwaTask::OPTION_MAX_SEED_DIFFERENCES, maxSeedDifferencesSpinBox->value());
    settings.insert(BwaTask::OPTION_MAX_QUEUE_ENTRIES, maxQueueEntriesSpinBox->value());
    settings.insert(BwaTask::OPTION_THREADS, threadsSpinBox->value());
    settings.insert(BwaTask::OPTION_MISMATCH_PENALTY, mismatchPenaltySpinBox->value());
    settings.insert(BwaTask::OPTION_GAP_OPEN_PENALTY, gapOpenPenaltySpinBox->value());
    settings.insert(BwaTask::OPTION_GAP_EXTENSION_PENALTY, gapExtensionPenaltySpinBox->value());
    settings.insert(BwaTask::OPTION_BEST_HITS, bestHitsSpinBox->value());
    settings.insert(BwaTask::OPTION_QUALITY_THRESHOLD, qualityThresholdSpinBox->value());
    settings.insert(BwaTask::OPTION_BARCODE_LENGTH, barcodeLengthSpinBox->value());
    settings.insert(BwaTask::OPTION_COLORSPACE, colorspaceCheckBox->isChecked());
    settings.insert(BwaTask::OPTION_LONG_SCALED_GAP_PENALTY_FOR_LONG_DELETIONS, longScaledGapPenaltyForLongDeletionsCheckBox->isChecked());
    settings.insert(BwaTask::OPTION_NON_ITERATIVE_MODE, nonIterativeModeCheckBox->isChecked());

    return settings;
}

void BwaSettingsWidget::buildIndexUrl(const GUrl &) {
    // do nothing
}

bool BwaSettingsWidget::isParametersOk(QString &) {
    return true;
}

// BwaBuildSettingsWidget

BwaBuildSettingsWidget::BwaBuildSettingsWidget(QWidget *parent):
    DnaAssemblyAlgorithmBuildIndexWidget(parent)
{
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);
}

QMap<QString,QVariant> BwaBuildSettingsWidget::getBuildIndexCustomSettings() {
    QMap<QString, QVariant> settings;
    settings.insert(BwaTask::OPTION_COLORSPACE, colorspaceCheckBox->isChecked());
    {
        QString algorithm;
        switch(indexAlgorithmComboBox->currentIndex()) {
        case 0:
            algorithm = "bwtsw";
            break;
        case 1:
            algorithm = "div";
            break;
        case 2:
            algorithm = "is";
            break;
        default:
            assert(false);
            break;
        }
        settings.insert(BwaTask::OPTION_INDEX_ALGORITHM, algorithm);
    }
    return settings;
}

QString BwaBuildSettingsWidget::getIndexFileExtension() {
    return QString();
}

void BwaBuildSettingsWidget::buildIndexUrl(const GUrl& ) {
    // do nothing
}

// BwaGUIExtensionsFactory

DnaAssemblyAlgorithmMainWidget *BwaGUIExtensionsFactory::createMainWidget(QWidget *parent) {
    return new BwaSettingsWidget(parent);
}

DnaAssemblyAlgorithmBuildIndexWidget *BwaGUIExtensionsFactory::createBuildIndexWidget(QWidget *parent) {
    return new BwaBuildSettingsWidget(parent);
}

bool BwaGUIExtensionsFactory::hasMainWidget() {
    return true;
}

bool BwaGUIExtensionsFactory::hasBuildIndexWidget() {
    return true;
}

} //namespace
