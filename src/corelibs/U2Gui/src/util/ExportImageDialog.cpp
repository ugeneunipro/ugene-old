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
#include "ImageExporter.h"

#include <ui/ui_ExportImageDialog.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
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


namespace U2 {

static const QString IMAGE_DIR = "image";

static const QString SVG_FORMAT = "svg";
static const int SVG_FORMAT_ID = 1;
static const QString PS_FORMAT = "ps";
static const int PS_FORMAT_ID = 2;
static const QString PDF_FORMAT = "pdf";


ExportImageDialog::ExportImageDialog(ImageExporter *exporter,
                                     QWidget *parent,
                                     const QString &filename)
    : QDialog(parent),
      currentExporter(exporter),
      filename(filename),
      origFilename(filename),
      lod(IMAGE_DIR, QDir::homePath())
{
    SAFE_POINT(exporter != NULL, tr("Default image exporter is NULL"), );
    exporters << exporter;
    setupComponents();
    setupExporters();
}

ExportImageDialog::ExportImageDialog(const QList<ImageExporter*> &exporters,
                                     ImageExporter* defaultExporter,
                                     QWidget *parent, const QString &filename)
    : QDialog(parent),
      exporters(exporters),
      currentExporter(defaultExporter),
      filename(filename),
      origFilename(filename),
      lod(IMAGE_DIR, QDir::homePath())
{
    SAFE_POINT( !exporters.isEmpty(), tr("Image exporters list is emptry"), );
    if (defaultExporter == NULL) {
        currentExporter = exporters.first();
    }
    SAFE_POINT( currentExporter != NULL, tr("Default image exporter is NULL"), );
    setupComponents();
    setupExporters();
}

ExportImageDialog::ExportImageDialog(QWidget *screenShotWidget,
                                     ImageExporter::ImageScalingPolicy scalingPolicy,
                                     ImageExporter::FormatPolicy formatPolicy,
                                     QWidget *parent, const QString &file)
    : QDialog(parent),
      filename(file),
      origFilename(file),
      lod(IMAGE_DIR, QDir::homePath())
{
    WidgetScreenshotImageExporter* exporter = new WidgetScreenshotImageExporter(screenShotWidget, scalingPolicy, formatPolicy);
    exporter->setParent(this);
    exporters << exporter;
    currentExporter = exporter;
    setupComponents();
    setupExporters();
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

    QCursor cursor = this->cursor();
    setCursor(Qt::WaitCursor);

    bool result=false;
    if (isVectorGraphicFormat(format)) {
        int formatId = getVectorFormatIdByName(format);
        if (formatId == SVG_FORMAT_ID){
            result = currentExporter->exportToSVG(filename);
        }else if (formatId == PS_FORMAT_ID){
            result = currentExporter->exportToPDF(filename, format);
        }
    }
    else {
        result = currentExporter->exportToBitmap(filename, format,
                                                 QSize(getWidth(), getHeight()),
                                                 (hasQuality() ? getQuality() : -1));
    }
    if (!result) {
        QMessageBox::critical(this, L10N::errorTitle(), L10N::errorImageSave(filename, format));
        return;
    }
    setCursor(cursor);

    ioLog.info(tr("Done!"));
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
    lod.url = QFileDialog::getSaveFileName(this, tr("Save image to..."), fileName, fileFormats, 0, QFileDialog::DontConfirmOverwrite);
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

void ExportImageDialog::sl_onExporterSelected(QAbstractButton *b) {
    QRadioButton *button = qobject_cast<QRadioButton*>(b);

    CHECK(button != NULL, );
    CHECK(exportersButtons.contains(button), );

    currentExporter = exportersButtons.value(button);
    setVectorFormats();
}

void ExportImageDialog::setupComponents() {
    ui = new Ui_ImageExportForm;
    ui->setupUi(this);
    new HelpButton(this, ui->buttonBox, "4227400");

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

    if (currentExporter->getScalingPolicy() == ImageExporter::Constant){
        ui->widthLabel->hide();
        ui->widthSpinBox->hide();
        ui->heightLabel->hide();
        ui->heightSpinBox->hide();
    }
    ui->widthSpinBox->setValue(currentExporter->getImageWidth());
    ui->heightSpinBox->setValue(currentExporter->getImageHeight());

    filename = lod.dir + "/" + origFilename + "."  + ui->formatsBox->currentText();
    filename = GUrlUtils::rollFileName(filename, "_copy", QSet<QString>());
    ui->fileNameEdit->setText(QDir::cleanPath(QDir::toNativeSeparators(filename)));

    setSizeControlsEnabled(!isVectorGraphicFormat(ui->formatsBox->currentText()));

    connect(ui->browseFileButton, SIGNAL(clicked()), this, SLOT(sl_onBrowseButtonClick()));
    connect(ui->formatsBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(sl_onFormatsBoxItemChanged(const QString&)));

    sl_onFormatsBoxItemChanged(ui->formatsBox->currentText());
}

void ExportImageDialog::setupExporters() {

    if (exporters.size() == 1) {
        ui->exportObjectGroupBox->hide();
        if (exporters.first()->getSettingsWidget() == NULL) {
            ui->settingsGroupBox->hide();
            resize(width(), ui->gridLayout->minimumSize().height());
            return;
        }
    }

    exportersGroup = new QButtonGroup(this);
    foreach (ImageExporter* exp, exporters) {
        QRadioButton *button = new QRadioButton(exp->getExportDescription(), this);
        button->setDisabled( !exp->isAvailable() );
        connect(button, SIGNAL(toggled(bool)), exp, SLOT(sl_setSettingsWidgetVisible(bool)));

        exportersGroup->addButton(button);
        exportersButtons.insert(button, exp);
        ui->exportersDescriptorsLayout->addWidget(button);

        QWidget *settingsWidget = exp->getSettingsWidget();
        if (settingsWidget == NULL) {
            settingsWidget = new QLabel(tr("<i>No additional settings<br>for current export object.</i>"), this);
            ((QLabel*)settingsWidget)->setMargin(9);
            connect(button, SIGNAL(toggled(bool)), settingsWidget, SLOT(setVisible(bool)));
        }
        settingsWidget->setVisible(false);
        ui->exporterSettingsLayout->addWidget(settingsWidget);

        if (exp == currentExporter) {
            settingsWidget->setVisible(true);
            button->setChecked(true);
        }

    }
    connect(exportersGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            SLOT(sl_onExporterSelected(QAbstractButton*)));

    ui->exportersDescriptorsLayout->addSpacerItem(new QSpacerItem(1, 1,
                                                                  QSizePolicy::MinimumExpanding,
                                                                  QSizePolicy::Expanding));
    ui->exporterSettingsLayout->addSpacerItem(new QSpacerItem(1, 1,
                                                                  QSizePolicy::MinimumExpanding,
                                                                  QSizePolicy::Expanding));

    ui->horizontalLayout->setAlignment(ui->exporterSettingsLayout, Qt::AlignRight);
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
    SAFE_POINT(currentExporter != NULL, tr("Current exporter is NULL"), );
    if (currentExporter->getFormatPolicy() == ImageExporter::SupportVectorFormats && !supportedFormats.contains(SVG_FORMAT)) {
        supportedFormats.append(SVG_FORMAT);
        supportedFormats.append(PS_FORMAT);
        supportedFormats.append(PDF_FORMAT);
    } else if (currentExporter->getFormatPolicy() == ImageExporter::IgnoreVectorFormats && supportedFormats.contains(SVG_FORMAT)) {
        supportedFormats.removeOne(SVG_FORMAT);
        supportedFormats.removeOne(PS_FORMAT);
        supportedFormats.removeOne(PDF_FORMAT);

        QString fileName = ui->fileNameEdit->text();
        QString ext = QFileInfo(fileName).suffix().toLower();
        if (ext == SVG_FORMAT || ext == PS_FORMAT || ext == PDF_FORMAT) {
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
    return ( SVG_FORMAT == formatName ) || ( PS_FORMAT == formatName )
        || ( PDF_FORMAT == formatName );
}

bool ExportImageDialog::isLossyFormat(const QString &formatName) {
    return ( "jpeg" == formatName ) || ( "jpg" == formatName ) || ( "png" == formatName );
}

int ExportImageDialog::getVectorFormatIdByName( const QString& formatName )
{
    if (formatName == SVG_FORMAT) {
        return SVG_FORMAT_ID;
    } else if ((formatName == PS_FORMAT) || (formatName == PDF_FORMAT)) {
        return PS_FORMAT_ID;
    } else
        return -1;

    return -1;
}

} // namespace

