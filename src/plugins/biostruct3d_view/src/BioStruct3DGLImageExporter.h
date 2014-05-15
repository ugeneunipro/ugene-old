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

#ifndef _U2_BIO_STRUCT_3DGL_IMAGE_EXPORTER_H_
#define _U2_BIO_STRUCT_3DGL_IMAGE_EXPORTER_H_

#include <U2Gui/ImageExporter.h>

namespace U2 {

class BioStruct3DGLWidget;

class BioStruct3DGLImageExporter : public ImageExporter {
    Q_OBJECT
public:
    BioStruct3DGLImageExporter(BioStruct3DGLWidget* widget);

    bool exportToSVG(const QString &filename) const;
    bool exportToPDF(const QString &filename, const QString &format) const;
    bool exportToBitmap(const QString &filename, const QString &format, const QSize &size, int quality) const;

    int getImageWidth() const;
    int getImageHeight() const;

protected:
    virtual void initSettingsWidget() {}
private:
    BioStruct3DGLWidget* glWidget;
}; // class BioStruct3DGLImageExporter

} // namespace

#endif // _U2_BIO_STRUCT_3DGL_IMAGE_EXPORTER_H_
