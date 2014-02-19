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

#include <ui/ui_ExportImageDialog.h>
#include <U2Core/Log.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/TextUtils.h>
#include <U2Gui/DialogUtils.h>

#include <QtGui/QImageWriter>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>
#include <U2Gui/HelpButton.h>

#include <QtSvg/QSvgGenerator>
#include <QDomDocument>
#include <QtGui/QImageWriter>

namespace U2 {

static const QString IMAGE_DIR = "image";

static const QString SVG_FORMAT = "svg";
static const int SVG_FORMAT_ID = 1;
static const QString PS_FORMAT = "ps";
static const int PS_FORMAT_ID = 2;
static const QString PDF_FORMAT = "pdf";

ExportImageDialog::ExportImageDialog( QWidget* widget, bool _showSizeRuler, bool _useVectorFormats, const QString& _filename )
: QDialog(widget), widget(widget), filename(_filename), origFilename(_filename), lod(IMAGE_DIR, QDir::homePath()), showSizeRuler(_showSizeRuler), useVectorFormats(_useVectorFormats) {
    setupComponents();
}

ExportImageDialog::ExportImageDialog( QWidget* widget, QRect _rect, bool _showSizeRuler, bool _useVectorFormats, const QString& _filename )
: QDialog(widget), widget(widget), filename(_filename), origFilename(_filename), lod(IMAGE_DIR, QDir::homePath()), rect(_rect), showSizeRuler(_showSizeRuler), useVectorFormats(_useVectorFormats) {
    setupComponents();
}

int ExportImageDialog::getWidth() {
    return ui->widthSpinBox->value();
}

int ExportImageDialog::getHeight() {
    return ui->heightSpinBox->value();
}

void ExportImageDialog::setupComponents() {
    ui = new Ui_ImageExportForm;
    ui->setupUi(this);
    new HelpButton(this, ui->buttonBox, "4227400");

    if(useVectorFormats){
        supportedFormats.append("svg");
        supportedFormats.append("ps");
        supportedFormats.append("pdf");
    }
    QList<QByteArray> list=QImageWriter::supportedImageFormats();
    foreach(QByteArray format,list){
        if(format != "ico"){
            supportedFormats.append(QString(format));
        }
    }

    if(!showSizeRuler){
        ui->widthLabel->hide();
        ui->widthSpinBox->hide();
        ui->heightLabel->hide();
        ui->heightSpinBox->hide();
    }
    ui->widthSpinBox->setValue(widget->width());
    ui->heightSpinBox->setValue(widget->height());
    
    foreach (const QString &format, supportedFormats) {
        ui->formatsBox->addItem(format);
    }

    filename = lod.dir + "/" + origFilename + "."  + ui->formatsBox->currentText();
    filename = GUrlUtils::rollFileName(filename, "_copy", QSet<QString>());
    ui->fileNameEdit->setText(QDir::cleanPath(QDir::toNativeSeparators(filename)));

    setSizeControlsEnabled(!isVectorGraphicFormat(ui->formatsBox->currentText()));

    connect(ui->browseFileButton, SIGNAL(clicked()), this, SLOT(sl_onBrowseButtonClick()));
    connect(ui->formatsBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(sl_onFormatsBoxItemChanged(const QString&)));

    sl_onFormatsBoxItemChanged(ui->formatsBox->currentText());
    setMaximumHeight(layout()->minimumSize().height());
}

void ExportImageDialog::accept() 
{
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

    if(rect.isEmpty()){
        rect = widget->rect();
    }
    bool result=false;
    if (isVectorGraphicFormat(format)) {
        int formatId = getVectorFormatIdByName(format);

        if(formatId == SVG_FORMAT_ID){
            result=exportToSVG();
        }else if(formatId == PS_FORMAT_ID){
            result=exportToPDF();
        }
    }
    else {
        result=exportToBitmap();
    }
    if (!result) {
        QMessageBox::critical(this, L10N::errorTitle(), L10N::errorImageSave(filename, format));
        return;
    }
    setCursor(cursor);

    ioLog.info("Done!");
    QDialog::accept();
}
bool ExportImageDialog::exportToSVG(){
    bool result=false;
    QPainter painter;
    QSvgGenerator generator;
    generator.setFileName(filename);
    generator.setSize(rect.size());
    generator.setViewBox(rect);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(&generator);
    widget->render(&painter);
    result = painter.end();
    //fix for UGENE-76
    QDomDocument doc("svg");
    QFile file(lod.url);
    bool ok=file.open(QIODevice::ReadOnly);
    if (!ok && !result){
       result=false;
    }
    ok=doc.setContent(&file);
    if (!ok && !result) {
        file.close();
        result=false;
    }
    if(result){
        file.close();
        QDomNodeList radialGradients=doc.elementsByTagName("radialGradient");
        for(uint i=0;i<radialGradients.length();i++){
            if(radialGradients.at(i).isElement()){
                QDomElement tag=radialGradients.at(i).toElement();
                if(tag.hasAttribute("xml:id")){
                    QString id=tag.attribute("xml:id");
                    tag.removeAttribute("xml:id");
                    tag.setAttribute("id",id);
                }
            }
        }
        file.open(QIODevice::WriteOnly);
        file.write(doc.toByteArray());
        file.close();
    }
    //end of fix UGENE-76
    return result;
}
bool ExportImageDialog::exportToPDF(){
    QPainter painter;
    QPrinter printer;
    printer.setOutputFileName(filename);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(&printer);
    widget->render(&painter);
    return painter.end();
}
bool ExportImageDialog::exportToBitmap(){
    rect.setRight( this->getWidth() );
    rect.setBottom( this->getHeight() );
    QPixmap image = QPixmap::grabWidget(widget, widget->rect());
    image = image.scaled(rect.size(), Qt::KeepAspectRatio);
    if(hasQuality()){
        return image.save(filename, qPrintable(format),getQuality());
    }else{
        return image.save(filename, qPrintable(format));
    }
}
bool ExportImageDialog::hasQuality(){
    return ui->qualitySpinBox->isEnabled();
}
int ExportImageDialog::getQuality(){
    return ui->qualitySpinBox->value();
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
    QString format = text;
    assert(supportedFormats.contains(text));

    QString fileName = ui->fileNameEdit->text();

    // Remove old suffix if present
    QString ext = QFileInfo(fileName).suffix().toLower();
    if (supportedFormats.contains(ext)) {
        fileName.chop(ext.size() + 1);
    }

    fileName += "." + format;
    ui->fileNameEdit->setText(QDir::toNativeSeparators(fileName));

    setSizeControlsEnabled(!isVectorGraphicFormat(format));

    const bool areQualityWidgetsVisible = isLossyFormat( format );
    ui->qualityLabel->setVisible( areQualityWidgetsVisible );
    ui->qualityHorizontalSlider->setVisible( areQualityWidgetsVisible );
    ui->qualitySpinBox->setVisible( areQualityWidgetsVisible );
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

void ExportImageDialog::setSizeControlsEnabled(bool enabled)
{
    ui->widthLabel->setEnabled(enabled); ui->heightLabel->setEnabled(enabled);
    ui->widthSpinBox->setEnabled(enabled); ui->heightSpinBox->setEnabled(enabled);
}

} // namespace

