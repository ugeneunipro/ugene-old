/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "ExportImageDialog.h"
#include "imageExport/WidgetScreenshotExportTask.h"

#include <ui/ui_ExportImageDialog.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/AppContext.h>
#include <U2Gui/HelpButton.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#endif
#include <QtGui/QImageWriter>
#include <QtGui/QRadioButton>
#include <QtGui/QButtonGroup>
#include <QtGui/QPushButton>


namespace U2 {

static const QString IMAGE_DIR = "image";

ExportImageDialog::ExportImageDialog(QWidget *screenShotWidget,
                                     InvokedFrom invoSource,
                                     ImageScalingPolicy scalingPolicy,
                                     FormatPolicy formatPolicy,
                                     QWidget *parent, const QString &file)
    : QDialog(parent),
      scalingPolicy(scalingPolicy),
      formatPolicy(formatPolicy),
      filename(file),
      origFilename(file),
      lod(IMAGE_DIR, QDir::homePath()),
      source(invoSource)
{
    exportTaskFactory = new WidgetScreenshotImageExportTaskFactory(screenShotWidget);
    init();
}

ExportImageDialog::ExportImageDialog(ImageExportTaskFactory *factory,
                                     InvokedFrom invoSource,
                                     ImageScalingPolicy scalingPolicy,
                                     FormatPolicy formatPolicy,
                                     QWidget *parent, const QString &file)
    : QDialog(parent),
      exportTaskFactory(factory),
      scalingPolicy(scalingPolicy),
      formatPolicy(formatPolicy),

      filename(file),
      origFilename(file),
      lod(IMAGE_DIR, QDir::homePath()),
      source(invoSource)
{
    SAFE_POINT( exportTaskFactory != NULL, tr("Image export task factory is NULL"), );
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
        QMessageBox::warning(this, tr("Error"), tr("The filename is empty!"));
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
                                     (hasQuality() ? getQuality() : -1));
    Task* task = exportTaskFactory->getTaskInstance(settings);
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
    lod.url = QFileDialog::getSaveFileName(this, tr("Save Image As"), fileName, fileFormats, 0, QFileDialog::DontConfirmOverwrite);
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

void ExportImageDialog::sl_hintMessageChanged(const QString &message) {
    ui->hintLabel->setText(message);
}

void ExportImageDialog::init() {
    ui = new Ui_ImageExportForm;
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Export"));

    switch (source) {
    case WD:
        new HelpButton(this, ui->buttonBox, "4227400");
        break;
    case CircularView:
        new HelpButton(this, ui->buttonBox, "4227400");
        break;
    case MSA:
        new HelpButton(this, ui->buttonBox, "4227400");
        break;
    case SequenceView:
        new HelpButton(this, ui->buttonBox, "4227400");
        break;
    case AssemblyView:
        new HelpButton(this, ui->buttonBox, "4227400");
        break;
    case PHYTreeView:
        new HelpButton(this, ui->buttonBox, "4227400");
        break;
    case DotPlot:
        new HelpButton(this, ui->buttonBox, "4227400");
        break;
    case MolView:
        new HelpButton(this, ui->buttonBox, "4227400");
        break;
    default:
        FAIL("Can't find help Id",);
        break;
    }

    QList<QByteArray> list = QImageWriter::supportedImageFormats();
    foreach (QByteArray format,list){
        if (format != "ico"){
            supportedFormats.append(QString(format));
        }
    }
    foreach (const QString &format, supportedFormats) {
        ui->formatsBox->addItem(format);
    }
    setVectorFormats();
    if (ui->formatsBox->findText("png") != -1) {
        ui->formatsBox->setCurrentIndex( ui->formatsBox->findText("png") );
    }

    if (scalingPolicy == Constant){
        ui->widthLabel->hide();
        ui->widthSpinBox->hide();
        ui->heightLabel->hide();
        ui->heightSpinBox->hide();
    }
    ui->hintLabel->hide();

    ui->widthSpinBox->setValue(exportTaskFactory->getImageWidth());
    ui->heightSpinBox->setValue(exportTaskFactory->getImageHeight());

    filename = lod.dir + "/" + origFilename + "."  + ui->formatsBox->currentText();
    filename = GUrlUtils::rollFileName(filename, "_copy", QSet<QString>());
    ui->fileNameEdit->setText(QDir::cleanPath(QDir::toNativeSeparators(filename)));

    setSizeControlsEnabled(!isVectorGraphicFormat(ui->formatsBox->currentText()));

    connect(ui->browseFileButton, SIGNAL(clicked()), this, SLOT(sl_onBrowseButtonClick()));
    connect(ui->formatsBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(sl_onFormatsBoxItemChanged(const QString&)));

    sl_onFormatsBoxItemChanged(ui->formatsBox->currentText());

    QWidget* settingsWidget = exportTaskFactory->getSettingsWidget();
    if (settingsWidget == NULL) {
        ui->settingsGroupBox->hide();
    } else {
        ui->settingsGroupBox->setTitle( ui->settingsGroupBox->title().prepend(exportTaskFactory->getExportDescription()) );
        ui->settingsLayout->addWidget(settingsWidget);
    }
    resize(width(), ui->dialogLayout->minimumSize().height());
}

void ExportImageDialog::setSizeControlsEnabled(bool enabled)
{
    ui->widthLabel->setEnabled(enabled);
    ui->heightLabel->setEnabled(enabled);
    ui->widthSpinBox->setEnabled(enabled);
    ui->heightSpinBox->setEnabled(enabled);
}

void ExportImageDialog::setVectorFormats()
{
    if (formatPolicy == SupportVectorFormats && !supportedFormats.contains(ImageExportTaskSettings::SVG_FORMAT)) {
        supportedFormats.append(ImageExportTaskSettings::SVG_FORMAT);
        supportedFormats.append(ImageExportTaskSettings::PS_FORMAT);
        supportedFormats.append(ImageExportTaskSettings::PDF_FORMAT);
    } else if (formatPolicy == IgnoreVectorFormats && supportedFormats.contains(ImageExportTaskSettings::SVG_FORMAT)) {
        supportedFormats.removeOne(ImageExportTaskSettings::SVG_FORMAT);
        supportedFormats.removeOne(ImageExportTaskSettings::PS_FORMAT);
        supportedFormats.removeOne(ImageExportTaskSettings::PDF_FORMAT);

        QString fileName = ui->fileNameEdit->text();
        QString ext = QFileInfo(fileName).suffix().toLower();
        if (ext == ImageExportTaskSettings::SVG_FORMAT || ext == ImageExportTaskSettings::PS_FORMAT || ext == ImageExportTaskSettings::PDF_FORMAT) {
            fileName.chop(ext.size() + 1);
        }
        ui->fileNameEdit->setText(QDir::toNativeSeparators(fileName));

    } else {
        return;
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

bool ExportImageDialog::isVectorGraphicFormat( const QString &formatName ) {
    return ( ImageExportTaskSettings::SVG_FORMAT == formatName ) || ( ImageExportTaskSettings::PS_FORMAT == formatName )
        || ( ImageExportTaskSettings::PDF_FORMAT == formatName );
}

bool ExportImageDialog::isLossyFormat(const QString &formatName) {
    return ( "jpeg" == formatName ) || ( "jpg" == formatName );
}

} // namespace

