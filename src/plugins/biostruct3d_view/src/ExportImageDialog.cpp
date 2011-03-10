#include "ExportImageDialog.h"
#include "BioStruct3DGLWidget.h"
#include "gl2ps/gl2ps.h"

#include <U2Core/Log.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/TextUtils.h>
#include <U2Misc/DialogUtils.h>

#include <QtGui/QImageWriter>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2 {

static const QString IMAGE_DIR = "image";

static const QString SVG_FORMAT = "svg";
static const QString PS_FORMAT = "ps";

ExportImageDialog::ExportImageDialog( BioStruct3DGLWidget* widget ) : QDialog(widget), glWidget(widget)
{
    setupUi(this);

    supportedFormats.append("svg");
    supportedFormats.append("png");
    supportedFormats.append("ps");
    supportedFormats.append("jpg");
    supportedFormats.append("tiff");

    setupComponents();
}

void ExportImageDialog::setupComponents()
{
    widthSpinBox->setValue(glWidget->width());
    heightSpinBox->setValue(glWidget->height());
    
    foreach (const QString &format, supportedFormats) {
        formatsBox->addItem(format);
    }

    LastOpenDirHelper lod(IMAGE_DIR, QDir::homePath());

    QString fileName = lod.dir + "/" + glWidget->getPDBId() + "."  + formatsBox->currentText();
    fileName = GUrlUtils::rollFileName(fileName, "_copy", QSet<QString>());
    fileNameEdit->setText(QDir::toNativeSeparators(fileName));

    setSizeControlsEnabled(!isVectorGraphicFormat(formatsBox->currentText()));

    connect(browseFileButton, SIGNAL(clicked()), this, SLOT(sl_onBrowseButtonClick()));
    connect(formatsBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(sl_onFormatsBoxItemChanged(const QString&)));
}

void ExportImageDialog::accept() 
{
    QString fileName = fileNameEdit->text();
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("The filename is empty!"));
        return;
    }

    QString format = formatsBox->currentText();

    if (QFile::exists(fileName)) {
        int res = QMessageBox::warning(this,
                                       tr("Overwrite file?"),
                                       tr("The file \"%1\" already exists. Do you wish to overwrite it?").arg(fileName),
                                       QMessageBox::Yes, QMessageBox::No);
        if (res == QMessageBox::No) {
            return;
        }
    }

    ioLog.info(tr("Saving image to '%1'...").arg(fileName));

    QCursor cursor = this->cursor();
    setCursor(Qt::WaitCursor);

    if (isVectorGraphicFormat(format)) {
        int opt = GL2PS_NONE;
        int formatId = getVectorFormatIdByName(format);
        glWidget->writeImage2DToFile(formatId, opt, 2, qPrintable(fileName));
    }
    else {
        QPixmap image = glWidget->renderPixmap(widthSpinBox->value(), heightSpinBox->value());
        bool result = image.save(fileName, qPrintable(format));
        if (!result) {
            QMessageBox::critical(this, L10N::errorTitle(), L10N::errorImageSave(fileName, format));
            return;
        }
    }
    setCursor(cursor);

    ioLog.info("Done!");
    QDialog::accept();
}

void ExportImageDialog::sl_onBrowseButtonClick() {
    QString fileFormats;
    foreach (const QString &formatName, supportedFormats) {
        fileFormats += formatName.toUpper() + " format (*." + formatName + ");;";
    }

    QString fileName = fileNameEdit->text();
    LastOpenDirHelper lod(IMAGE_DIR);
    lod.url = QFileDialog::getSaveFileName(this, tr("Save image to..."), fileName, fileFormats, 0, QFileDialog::DontConfirmOverwrite);
    if (lod.url.isEmpty()) {
        return;
    }

    QString ext = QFileInfo(lod.url).suffix().toLower();
    if (ext.isEmpty() || !supportedFormats.contains(ext)) {
        ext = formatsBox->currentText();
        lod.url += "." + ext;
    }

    fileNameEdit->setText(QDir::toNativeSeparators(lod.url));

    int index = formatsBox->findText(ext);
    formatsBox->setCurrentIndex(index);

    setSizeControlsEnabled(!isVectorGraphicFormat(formatsBox->currentText()));
}

void ExportImageDialog::sl_onFormatsBoxItemChanged(const QString &text)
{
    QString format = text;
    assert(supportedFormats.contains(text));

    QString fileName = fileNameEdit->text();

    // Remove old suffix if present
    QString ext = QFileInfo(fileName).suffix().toLower();
    if (supportedFormats.contains(ext)) {
        fileName.chop(ext.size() + 1);
    }

    fileName += "." + format;
    fileNameEdit->setText(QDir::toNativeSeparators(fileName));

    setSizeControlsEnabled(!isVectorGraphicFormat(format));
}

bool ExportImageDialog::isVectorGraphicFormat( const QString& formatName )
{
    if ((formatName == SVG_FORMAT) || (formatName == PS_FORMAT)) {
        return true;
    }
    return false;

}

int ExportImageDialog::getVectorFormatIdByName( const QString& formatName )
{
    if (formatName == SVG_FORMAT) {
        return GL2PS_SVG;
    } else if (formatName == PS_FORMAT) {
        return GL2PS_EPS;
    } else 
        return -1;
    
}

void ExportImageDialog::setSizeControlsEnabled(bool enabled)
{
    widthLabel->setEnabled(enabled); heightLabel->setEnabled(enabled);
    widthSpinBox->setEnabled(enabled); heightSpinBox->setEnabled(enabled);
}

} // namespace

