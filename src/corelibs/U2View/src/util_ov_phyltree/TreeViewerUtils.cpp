#include "TreeViewerUtils.h"
#include <U2Misc/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#define IMAGE_DIR "image"

namespace U2 {

QFont* TreeViewerUtils::font = NULL;
const char* TreeViewerUtils::IMAGE_FILTERS =
    "BMP - Windows Bitmap (*.bmp);;"
    "GIF - Graphic Interchange Format (*.gif);;"
    "JPG/JPEG format (*.jpg);;"
    "PBM - Portable Bitmap (*.pbm);;"
    "PNG - Portable Network Graphics (*.png);;"
    "PPM - Portable Pixmap (*.ppm);;"
    "TIFF - Tagged Image File format (*.tif);;"
    "XBM - X11 Bitmap (*.xbm);;"
    "XPM - X11 Pixmap (*.xpm)";

void TreeViewerUtils::saveImageDialog(const QString& filters, QString &fileName, QString &format) {
    LastOpenDirHelper lod(IMAGE_DIR);
    int i = fileName.lastIndexOf('.');
    if (i != -1) {
        fileName = fileName.left(i);
    }
    QString initialPath = lod.dir + "/" + fileName;
    fileName = QFileDialog::getSaveFileName(NULL, QObject::tr("Save As"), initialPath, filters, &format);
    lod.url = fileName;
    if (fileName.isEmpty())
        return;

    format = format.left(3).toLower();
    if (!fileName.endsWith("." + format)) {
        fileName.append("." + format);
        if (QFile::exists(fileName)) {
            QMessageBox::StandardButtons b = QMessageBox::warning(0, QObject::tr("Replace file"),
                QObject::tr("%1 already exists.\nDo you want to replace it?").arg(fileName),
                QMessageBox::Yes | QMessageBox::No);
            if (QMessageBox::Yes != b) {
                return;
            }
        }
    }
}

const QFont& TreeViewerUtils::getFont() {
    if (font == NULL) {
        font = new QFont();
    }
    return *font;
}

}
