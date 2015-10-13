/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QtGui/QImageWriter>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/HelpButton.h>
#include <U2Gui/U2FileDialog.h>

#include "ExportImageDialog.h"
#include "imageExport/WidgetScreenshotExportTask.h"
#include "ui/ui_ExportImageDialog.h"

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
      lod(IMAGE_DIR, QDir::homePath()),
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
      lod(IMAGE_DIR, QDir::homePath()),
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
    filename = ui->fileNameEdit->text();
    if (filename.isEmpty()) {
        QMessageBox::warning(this, tr(DIALOG_ACCEPT_ERROR_TITLE), tr("The image file path is empty."));
        return;
    }

    if (!GUrlUtils::canWriteFile(filename)) {
        QMessageBox::warning(this, tr(DIALOG_ACCEPT_ERROR_TITLE), tr("The image file cannot be created. No write permissions."));
        return;
    }

    format = ui->formatsBox->currentText();
    if (QFile::exists(filename)) {
        int res = QMessageBox::warning(this,
                                       tr("Overwrite file?"),
                                       tr("The file \"%1\" already exists. Do you wish to overwrite it?").arg(filename),
                                       QMessageBox::Yes, QMessageBox::No);
        if (res == QMessageBox::No) {
            return;
        }
    }

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

void ExportImageDialog::sl_onBrowseButtonClick() {
    QString curFormat = ui->formatsBox->currentText();
    assert(supportedFormats.contains(curFormat));
    QList<QString> formats(supportedFormats);
    formats.removeAll(curFormat);
    formats.prepend(curFormat);
    QString fileFormats;
    for(int i = 0; i < formats.size(); ++i) {
        QString formatName = formats.at(i);
        fileFormats += formatName.toUpper() + " format (*." + formatName + ");;";
    }

    QString fileName = ui->fileNameEdit->text();
    LastUsedDirHelper lod(IMAGE_DIR);
    lod.url = U2FileDialog::getSaveFileName(this, tr("Save Image As"), fileName, fileFormats, 0, QFileDialog::DontConfirmOverwrite);
    if (lod.url.isEmpty()) {
        return;
    }

    QString ext = QFileInfo(lod.url).suffix().toLower();
    if (ext.isEmpty() || !supportedFormats.contains(ext)) {
        ext = ui->formatsBox->currentText();
        lod.url += "." + ext;
    }

    ui->fileNameEdit->setText(QDir::toNativeSeparators(lod.url));

    int index = ui->formatsBox->findText(ext);
    ui->formatsBox->setCurrentIndex(index);

    setSizeControlsEnabled(!isVectorGraphicFormat(ui->formatsBox->currentText()));
}

void ExportImageDialog::sl_onFormatsBoxItemChanged(const QString &text)
{
    format = text;
    CHECK(supportedFormats.contains(format), );

    QString fileName = ui->fileNameEdit->text();

    // Remove old suffix if present
    QString ext = QFileInfo(fileName).suffix().toLower();
    if (supportedFormats.contains(ext)) {
        fileName.chop(ext.size() + 1);
    }
    if (ext.isEmpty() && fileName.endsWith('.')) {
        fileName.chop(1);
    }

    fileName += "." + format;

    ui->fileNameEdit->setText(QDir::toNativeSeparators(fileName));

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
        new HelpButton(this, ui->buttonBox, "16122511");
        break;
    case CircularView:
        new HelpButton(this, ui->buttonBox, "16122186");
        break;
    case MSA:
        new HelpButton(this, ui->buttonBox, "16122255");
        break;
    case SequenceView:
        new HelpButton(this, ui->buttonBox, "16122148");
        break;
    case AssemblyView:
        new HelpButton(this, ui->buttonBox, "16122290");
        break;
    case PHYTreeView:
        new HelpButton(this, ui->buttonBox, "16122318");
        break;
    case DotPlot:
        new HelpButton(this, ui->buttonBox, "16122217");
        break;
    case MolView:
        new HelpButton(this, ui->buttonBox, "16122200");
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

    setFormats();

    if (scalingPolicy == NoScaling) {
        ui->imageSizeSettingsContainer->hide();
    }

    ui->widthSpinBox->setValue(exportController->getImageWidth());
    ui->heightSpinBox->setValue(exportController->getImageHeight());

    QString fixedName = GUrlUtils::fixFileName(origFilename);
    filename = lod.dir + "/" + fixedName + "."  + ui->formatsBox->currentText();
    filename = GUrlUtils::rollFileName(filename, "_copy", QSet<QString>());
    ui->fileNameEdit->setText(QDir::cleanPath(QDir::toNativeSeparators(filename)));

    setSizeControlsEnabled(!isVectorGraphicFormat(ui->formatsBox->currentText()));

    connect(ui->browseFileButton, SIGNAL(clicked()), this, SLOT(sl_onBrowseButtonClick()));
    connect(ui->formatsBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(sl_onFormatsBoxItemChanged(const QString&)));

    connect(exportController, SIGNAL(si_disableExport(bool)), SLOT(sl_disableExport(bool)));
    connect(exportController, SIGNAL(si_showMessage(QString)), SLOT(sl_showMessage(QString)));

    if (exportController->isExportDisabled()) {
        sl_disableExport(true);
        sl_showMessage(exportController->getDisableMessage());
    }

    sl_onFormatsBoxItemChanged(ui->formatsBox->currentText());

    QWidget* settingsWidget = exportController->getSettingsWidget();
    if (settingsWidget == NULL) {
        ui->settingsGroupBox->hide();
    } else {
        ui->settingsGroupBox->setTitle( tr( ui->settingsGroupBox->title().prepend(exportController->getExportDescription()).toLatin1().data() ) );
        ui->settingsLayout->addWidget(settingsWidget);
    }

    connect(ui->formatsBox, SIGNAL(currentIndexChanged(const QString&)), exportController, SLOT(sl_onFormatChanged(const QString&)));
}

void ExportImageDialog::setSizeControlsEnabled(bool enabled)
{
    ui->widthLabel->setEnabled(enabled);
    ui->heightLabel->setEnabled(enabled);
    ui->widthSpinBox->setEnabled(enabled);
    ui->heightSpinBox->setEnabled(enabled);
}

void ExportImageDialog::setFormats() {
    setRasterFormats();
    setSvgAndPdfFormats();

    // set default format
    if (ui->formatsBox->findText("png") != -1) {
        ui->formatsBox->setCurrentIndex( ui->formatsBox->findText("png") );
    }
}

void ExportImageDialog::setRasterFormats() {
    if (exportController->isRasterFormatsEnabled())
    {
        QList<QByteArray> list = QImageWriter::supportedImageFormats();
        list.removeAll("ico");
        foreach (QByteArray format,list){
            supportedFormats.append(QString(format));
            ui->formatsBox->addItem(format);
        }
    }
}

void ExportImageDialog::setSvgAndPdfFormats()
{
    if (exportController->isSvgSupported()) {
        if (!supportedFormats.contains(ImageExportTaskSettings::SVG_FORMAT)) {
            supportedFormats.append(ImageExportTaskSettings::SVG_FORMAT);
        } else {
            supportedFormats.removeOne(ImageExportTaskSettings::SVG_FORMAT);
            removeOutputFileNameExtention(ImageExportTaskSettings::SVG_FORMAT);
        }
    }

    if (exportController->isPdfSupported()) {
        if (!supportedFormats.contains(ImageExportTaskSettings::PS_FORMAT)) {
            supportedFormats.append(ImageExportTaskSettings::PS_FORMAT);
        }
        if (!supportedFormats.contains(ImageExportTaskSettings::PDF_FORMAT)) {
            supportedFormats.append(ImageExportTaskSettings::PDF_FORMAT);
        }
    } else {
        supportedFormats.removeOne(ImageExportTaskSettings::PS_FORMAT);
        supportedFormats.removeOne(ImageExportTaskSettings::PDF_FORMAT);
        removeOutputFileNameExtention(ImageExportTaskSettings::PS_FORMAT);
        removeOutputFileNameExtention(ImageExportTaskSettings::PDF_FORMAT);
    }

    // keep the same format selection during exporter change
    int idx = ui->formatsBox->currentIndex();
    bool selectFormat = supportedFormats.contains(ui->formatsBox->currentText());

    ui->formatsBox->clear();
    foreach (const QString &format, supportedFormats) {
        ui->formatsBox->addItem(format);
    }

    if (selectFormat) {
        ui->formatsBox->setCurrentIndex(idx);
    }
}

void ExportImageDialog::removeOutputFileNameExtention( const QString &ext) {
    QString fileName = ui->fileNameEdit->text();
    QString fileExt = QFileInfo(fileName).suffix().toLower();
    if (fileExt == ext) {
        fileName.chop(ext.size() + 1);
    }
    ui->fileNameEdit->setText(QDir::toNativeSeparators(fileName));
}

bool ExportImageDialog::isVectorGraphicFormat( const QString &formatName ) {
    return ( ImageExportTaskSettings::SVG_FORMAT == formatName ) || ( ImageExportTaskSettings::PS_FORMAT == formatName )
        || ( ImageExportTaskSettings::PDF_FORMAT == formatName );
}

bool ExportImageDialog::isLossyFormat(const QString &formatName) {
    return ( "jpeg" == formatName ) || ( "jpg" == formatName );
}

} // namespace

