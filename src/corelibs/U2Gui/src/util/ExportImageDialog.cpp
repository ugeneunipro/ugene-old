/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QImageWriter>
#include <QMessageBox>
#include <QPushButton>

#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/SaveDocumentController.h>

#include "ExportImageDialog.h"
#include "imageExport/WidgetScreenshotExportTask.h"
#include "ui_ExportImageDialog.h"

static const QString IMAGE_DIR = "image";
static const char *DIALOG_ACCEPT_ERROR_TITLE = "Unable to save file";

namespace U2 {

ExportImageDialog::ExportImageDialog(QWidget *screenShotWidget,
                                     InvokedFrom invoSource,
                                     ImageScalingPolicy scalingPolicy,
                                     QWidget *parent, const QString &file)
    : QDialog(parent),
      scalingPolicy(scalingPolicy),
      filename(file),
      origFilename(file),
      source(invoSource)
{
    exportController = new WidgetScreenshotImageExportController(screenShotWidget);
    init();
}

ExportImageDialog::ExportImageDialog(ImageExportController *factory,
                                     InvokedFrom invoSource,
                                     ImageScalingPolicy scalingPolicy,
                                     QWidget *parent, const QString &file)
    : QDialog(parent),
      exportController(factory),
      scalingPolicy(scalingPolicy),
      filename(file),
      origFilename(file),
      source(invoSource)
{
    SAFE_POINT( exportController != NULL, tr("Image export task factory is NULL"), );
    init();
}

ExportImageDialog::~ExportImageDialog() {
    delete ui;
}

int ExportImageDialog::getWidth() const {
    return ui->widthSpinBox->value();
}

int ExportImageDialog::getHeight() const {
    return ui->heightSpinBox->value();
}

bool ExportImageDialog::hasQuality() const {
    return ui->qualitySpinBox->isEnabled();
}

int ExportImageDialog::getQuality() const {
    return ui->qualitySpinBox->value();
}

void ExportImageDialog::accept() {
    filename = saveController->getSaveFileName();
    if (filename.isEmpty()) {
        QMessageBox::warning(this, tr(DIALOG_ACCEPT_ERROR_TITLE), tr("The image file path is empty."));
        return;
    }

    if (!GUrlUtils::canWriteFile(filename)) {
        QMessageBox::warning(this, tr(DIALOG_ACCEPT_ERROR_TITLE), tr("The image file cannot be created. No write permissions."));
        return;
    }

    format = saveController->getFormatIdToSave();

    LastUsedDirHelper lod(IMAGE_DIR);
    lod.url = filename;
    ioLog.info(tr("Saving image to '%1'...").arg(filename));

    ImageExportTaskSettings settings(filename, format,
                                     QSize(getWidth(), getHeight()),
                                     (hasQuality() ? getQuality() : -1),
                                     ui->dpiSpinBox->value());
    Task* task = exportController->getTaskInstance(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);

    QDialog::accept();
}

void ExportImageDialog::sl_onFormatsBoxItemChanged(const QString &format) {
    setSizeControlsEnabled(!isVectorGraphicFormat(format));

    const bool areQualityWidgetsVisible = isLossyFormat( format );
    ui->qualityLabel->setVisible( areQualityWidgetsVisible );
    ui->qualityHorizontalSlider->setVisible( areQualityWidgetsVisible );
    ui->qualitySpinBox->setVisible( areQualityWidgetsVisible );
}

void ExportImageDialog::sl_showMessage(const QString &message) {
    ui->hintLabel->setText(message);
    if (!message.isEmpty()) {
        ui->hintLabel->show();
    } else {
        ui->hintLabel->hide();
    }
}

void ExportImageDialog::sl_disableExport(bool disable) {
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(disable);
}

void ExportImageDialog::init() {
    ui = new Ui_ImageExportForm;
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Export"));

    switch (source) {
    case WD:
        new HelpButton(this, ui->buttonBox, "17467907");
        break;
    case CircularView:
        new HelpButton(this, ui->buttonBox, "17467582");
        break;
    case MSA:
        new HelpButton(this, ui->buttonBox, "17467652");
        break;
    case SequenceView:
        new HelpButton(this, ui->buttonBox, "17468747");
        break;
    case AssemblyView:
        new HelpButton(this, ui->buttonBox, "17467688");
        break;
    case PHYTreeView:
        new HelpButton(this, ui->buttonBox, "17467716");
        break;
    case DotPlot:
        new HelpButton(this, ui->buttonBox, "17467613");
        break;
    case MolView:
        new HelpButton(this, ui->buttonBox, "17467596");
        break;
    default:
        FAIL("Can't find help Id",);
        break;
    }

    ui->dpiWidget->setVisible(source == DotPlot);

    // set tip color
    QString style = "QLabel { color: " + L10N::errorColorLabelStr() + "; font: bold;}";
    ui->hintLabel->setStyleSheet(style);
    ui->hintLabel->hide();

    initSaveController();

    if (scalingPolicy == NoScaling) {
        ui->imageSizeSettingsContainer->hide();
    }

    ui->widthSpinBox->setValue(exportController->getImageWidth());
    ui->heightSpinBox->setValue(exportController->getImageHeight());

    setSizeControlsEnabled(!isVectorGraphicFormat(saveController->getFormatIdToSave()));

    connect(ui->formatsBox, SIGNAL(currentIndexChanged(const QString&)), exportController, SLOT(sl_onFormatChanged(const QString&)));
    connect(ui->formatsBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(sl_onFormatsBoxItemChanged(const QString&)));

    connect(exportController, SIGNAL(si_disableExport(bool)), SLOT(sl_disableExport(bool)));
    connect(exportController, SIGNAL(si_showMessage(QString)), SLOT(sl_showMessage(QString)));

    if (exportController->isExportDisabled()) {
        sl_disableExport(true);
        sl_showMessage(exportController->getDisableMessage());
    }

    QWidget* settingsWidget = exportController->getSettingsWidget();
    if (settingsWidget == NULL) {
        ui->settingsGroupBox->hide();
    } else {
        ui->settingsLayout->addWidget(settingsWidget);
    }
}

void ExportImageDialog::initSaveController() {
    LastUsedDirHelper dirHelper(IMAGE_DIR, GUrlUtils::getDefaultDataPath());

    SaveDocumentControllerConfig config;
    config.defaultDomain = IMAGE_DIR;
    config.defaultFileName = dirHelper.dir + "/" + GUrlUtils::fixFileName(origFilename);
    config.defaultFormatId = "png";
    config.fileDialogButton = ui->browseFileButton;
    config.fileNameEdit = ui->fileNameEdit;
    config.formatCombo = ui->formatsBox;
    config.parentWidget = this;
    config.saveTitle = tr("Save Image As");
    config.rollSuffix = "_copy";

    SaveDocumentController::SimpleFormatsInfo formatsInfo;
    QStringList formats = getFormats();
    foreach (const QString &format, formats) {
        formatsInfo.addFormat(format, format, QStringList() << format);
    }

    saveController = new SaveDocumentController(config, formatsInfo, this);
    saveController->setFormat(saveController->getFormatIdToSave());
}

void ExportImageDialog::setSizeControlsEnabled(bool enabled) {
    ui->widthLabel->setEnabled(enabled);
    ui->heightLabel->setEnabled(enabled);
    ui->widthSpinBox->setEnabled(enabled);
    ui->heightSpinBox->setEnabled(enabled);
}

QStringList ExportImageDialog::getFormats() {
    return getRasterFormats() + getSvgAndPdfFormats();
}

QStringList ExportImageDialog::getRasterFormats() {
    QStringList result;
    CHECK(exportController->isRasterFormatsEnabled(), result);
    QList<QByteArray> list = QImageWriter::supportedImageFormats();
    list.removeAll("ico");
    foreach (const QByteArray &format, list) {
        result << format;
    }
    return result;
}

QStringList ExportImageDialog::getSvgAndPdfFormats() {
    QStringList result;
    if (exportController->isSvgSupported()) {
        result << ImageExportTaskSettings::SVG_FORMAT;
    }

    if (exportController->isPdfSupported()) {
        result << ImageExportTaskSettings::PS_FORMAT;
        result << ImageExportTaskSettings::PDF_FORMAT;
    }

    return result;
}

bool ExportImageDialog::isVectorGraphicFormat( const QString &formatName ) {
    return ( ImageExportTaskSettings::SVG_FORMAT == formatName ) || ( ImageExportTaskSettings::PS_FORMAT == formatName )
        || ( ImageExportTaskSettings::PDF_FORMAT == formatName );
}

bool ExportImageDialog::isLossyFormat(const QString &formatName) {
    return ( "jpeg" == formatName ) || ( "jpg" == formatName );
}

} // namespace

