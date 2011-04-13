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

#include "GenomeAlignerTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/GUrl.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Algorithm/OpenCLGpuRegistry.h>
#include <QtGui/QFileDialog>
#include <U2Misc/DialogUtils.h>

#include "GenomeAlignerSettingsWidget.h"


namespace U2 {

static const int MIN_READ_SIZE = 10;
static const int MIN_PART_SIZE = 1;
static const int DEFAULT_PART_SIZE = 10;

GenomeAlignerSettingsWidget::GenomeAlignerSettingsWidget(QWidget* parent) : DnaAssemblyAlgorithmMainWidget(parent) {
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);

    connect(buildIndexFileButton, SIGNAL(clicked()), SLOT(sl_onSetIndexDirButtonClicked()));
    connect(partSlider, SIGNAL(valueChanged(int)), SLOT(sl_onPartSliderChanged(int)));
    connect(readSlider, SIGNAL(valueChanged(int)), SLOT(sl_onReadSliderChanged(int)));

    buildIndexFileButton->toggle();
    if (AppContext::getOpenCLGpuRegistry()->empty()) {
        gpuBox->setEnabled(false);
        gpuBox->setChecked(false);
    } else {
        gpuBox->setChecked(true);
    }

    systemSize = AppContext::getAppSettings()->getAppResourcePool()->getMaxMemorySizeInMB();
    partSlider->setEnabled(false);
    readSlider->setMinimum(MIN_READ_SIZE);
    readSlider->setMaximum(systemSize);
    readSlider->setValue(systemSize*2/3);

    indexDirEdit->setText(AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath());

    partSizeLabel->setText(QByteArray::number(partSlider->value()) + " Mb");
    indexSizeLabel->setText(QByteArray::number(partSlider->value()*13) + " Mb");
    totalSizeLabel->setText(QByteArray::number(partSlider->value()*13 + readSlider->value()) + " Mb");
    systemSizeLabel->setText(QByteArray::number(systemSize) + " Mb");
}

QMap<QString,QVariant> GenomeAlignerSettingsWidget::getDnaAssemblyCustomSettings() {
    QMap<QString,QVariant> settings;
    
    settings.insert(GenomeAlignerTask::OPTION_ALIGN_REVERSED, reverseBox->isChecked());
    settings.insert(GenomeAlignerTask::OPTION_OPENCL, gpuBox->isChecked());
    settings.insert(GenomeAlignerTask::OPTION_BEST, firstMatchBox->isChecked());
    settings.insert(GenomeAlignerTask::OPTION_READS_MEMORY_SIZE, readSlider->value());
    settings.insert(GenomeAlignerTask::OPTION_SEQ_PART_SIZE, partSlider->value());
    settings.insert(GenomeAlignerTask::OPTION_INDEX_DIR, indexDirEdit->text());
    if (omitQualitiesBox->isChecked()) {
        settings.insert(GenomeAlignerTask::OPTION_QUAL_THRESHOLD, qualThresholdBox->value() );
    }
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

void GenomeAlignerSettingsWidget::buildIndexUrl(const GUrl& url) {
    QString refUrl = url.getURLString();
    QFile file(refUrl);
    if (file.exists()) {
        int fileSize = 1 + (int)(file.size()/(1024*1024));
        int maxPartSize = qMin(fileSize, systemSize - MIN_READ_SIZE);
        partSlider->setMinimum(MIN_PART_SIZE);
        partSlider->setMaximum(maxPartSize);
        partSlider->setEnabled(true);
        partSlider->setValue(qMin(maxPartSize, DEFAULT_PART_SIZE));
    }
}

bool GenomeAlignerSettingsWidget::isParametersOk(QString &error) {
    if (systemSize < readSlider->value() + 13*partSlider->value()) {
        error = "There is no enough memory for the aligning on your computer. Try to reduce a memory size for short reads or for the reference fragment.";
        return false;
    }

    return true;
}

void GenomeAlignerSettingsWidget::sl_onSetIndexDirButtonClicked() {
    LastOpenDirHelper lod;

    lod.url = QFileDialog::getExistingDirectory(this, tr("Set index files directory"), indexDirEdit->text());
    if (!lod.url.isEmpty()) {
        GUrl result = lod.url;
        indexDirEdit->setText(result.getURLString());
    }
}

void GenomeAlignerSettingsWidget::sl_onPartSliderChanged(int value) {
    partSizeLabel->setText(QByteArray::number(value) + " Mb");
    indexSizeLabel->setText(QByteArray::number(value*13) + " Mb");

    readSlider->setMaximum(systemSize - 13*value);

    totalSizeLabel->setText(QByteArray::number(value*13 + readSlider->value()) + " Mb");
}

void GenomeAlignerSettingsWidget::sl_onReadSliderChanged(int value) {
    readSizeLabel->setText(QByteArray::number(value) + " Mb");
    totalSizeLabel->setText(QByteArray::number(partSlider->value()*13 + value) + " Mb");
}

} //namespace
