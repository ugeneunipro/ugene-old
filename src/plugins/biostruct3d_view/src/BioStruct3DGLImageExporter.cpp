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

#include "BioStruct3DGLImageExporter.h"
#include "BioStruct3DGLWidget.h"
#include "gl2ps/gl2ps.h"


namespace U2 {

BioStruct3DGLImageExporter::BioStruct3DGLImageExporter(BioStruct3DGLWidget *widget)
    : ImageExporter(ImageExporter::Resizable, ImageExporter::SupportVectorFormats),
      glWidget(widget)
{}

bool BioStruct3DGLImageExporter::exportToSVG(const QString &filename) const {
    int opt = GL2PS_NONE;
    glWidget->writeImage2DToFile(GL2PS_SVG, opt, 2, qPrintable(filename));
    return true; //TODO: need check on error
}

bool BioStruct3DGLImageExporter::exportToPDF(const QString &filename, const QString &format) const {
    int opt = GL2PS_NONE;

    if (format == "ps"){
        glWidget->writeImage2DToFile(GL2PS_PS, opt, 2, qPrintable(filename));
        return true; //TODO: need check on error
    } else if (format == "pdf"){
        glWidget->writeImage2DToFile(GL2PS_PDF, opt, 2, qPrintable(filename));
        return true; //TODO: need check on error
    }
    return false;

}

bool BioStruct3DGLImageExporter::exportToBitmap(const QString &filename, const QString &format, const QSize &size, int quality) const {
    glWidget->setImageRenderingMode(true);
    QPixmap image = glWidget->renderPixmap().scaled( size, Qt::KeepAspectRatio);
    glWidget->setImageRenderingMode(false);

    return image.save(filename, qPrintable(format), quality);
}

int BioStruct3DGLImageExporter::getImageWidth() const {
    return glWidget->width();
}

int BioStruct3DGLImageExporter::getImageHeight() const {
    return glWidget->height();
}


} // namespace

