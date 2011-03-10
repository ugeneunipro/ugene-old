#include "GenomeAlignerSettingsWidget.h"
#include "GenomeAlignerTask.h"

#include <U2Misc/DialogUtils.h>
#include <U2Core/GUrl.h>
#include <QtGui/QFileDialog>
#include <U2Core/AppContext.h>
#include <U2Algorithm/OpenCLGpuRegistry.h>


namespace U2 {

GenomeAlignerSettingsWidget::GenomeAlignerSettingsWidget(QWidget* parent) : DnaAssemblyAlgorithmMainWidget(parent), refUrl("") {
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);
    prebuiltIndexFileButton->setEnabled(false);
    prebuiltIndexFileEdit->setEnabled(false);

    connect(buildIndexFileButton, SIGNAL(clicked()), SLOT(sl_onSetIndexFileNameButtonClicked()));
    connect(prebuiltIndexFileButton, SIGNAL(clicked()), SLOT(sl_onOpenPrebuildIndexFileNameButtonClicked()));

    buildIndexFileButton->toggle();
    if (AppContext::getOpenCLGpuRegistry()->empty()) {
        gpuBox->setEnabled(false);
        gpuBox->setChecked(false);
    } else {
        gpuBox->setChecked(true);
    }
}

QMap<QString,QVariant> GenomeAlignerSettingsWidget::getDnaAssemblyCustomSettings() {
    QMap<QString,QVariant> settings;
    
    settings.insert(GenomeAlignerTask::OPTION_ALIGN_REVERSED, reverseBox->isChecked());
    settings.insert(GenomeAlignerTask::OPTION_OPENCL, gpuBox->isChecked());
    settings.insert(GenomeAlignerTask::OPTION_PREBUILT_INDEX, prebuiltIndexRadioButton->isChecked());
    settings.insert(GenomeAlignerTask::OPTION_BEST, firstMatchBox->isChecked());
    if (omitQualitiesBox->isChecked()) {
        settings.insert(GenomeAlignerTask::OPTION_QUAL_THRESHOLD, qualThresholdBox->value() );
    }
    if (buildIndexRadioButton->isChecked()) {
        settings.insert(GenomeAlignerTask::OPTION_INDEX_URL, buildIndexFileEdit->text());
    } else {
        settings.insert(GenomeAlignerTask::OPTION_INDEX_URL, prebuiltIndexFileEdit->text());
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
    refUrl = url.getURLString();
    buildIndexFileEdit->setText(url.dirPath() + "/" + url.baseFileName());// + "." + GenomeAlignerTask::INDEX_EXTENSION);
    prebuiltIndexFileEdit->setText(url.dirPath() + "/" + url.baseFileName());
}

bool GenomeAlignerSettingsWidget::isIndexOk() {
    if (buildIndexRadioButton->isChecked()) {
        return !buildIndexFileEdit->text().isEmpty();
    } else {
        return !prebuiltIndexFileEdit->text().isEmpty();
    }
}

void GenomeAlignerSettingsWidget::sl_onSetIndexFileNameButtonClicked() {
    LastOpenDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Set index file name"), lod.dir);
    if (!lod.url.isEmpty()) {
        GUrl result = lod.url;
        /*if (result.lastFileSuffix().isEmpty()) {
            result = QString( "%1.%2" ).arg( result.getURLString() ).arg(GenomeAlignerTask::INDEX_EXTENSION);
        }*/
        buildIndexFileEdit->setText(result.getURLString());
    }
}

void GenomeAlignerSettingsWidget::sl_onOpenPrebuildIndexFileNameButtonClicked() {
    LastOpenDirHelper lod;
    QString filter;

    lod.url = QFileDialog::getOpenFileName(this, tr("Open index file"), lod.dir, filter);
    if (lod.url.isEmpty()) {
        return;
    }

    prebuiltIndexFileEdit->setText(lod.url);
}

} //namespace
