/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "ExportImage3DGLDialog.h"
#include "BioStruct3DGLWidget.h"
#include "gl2ps/gl2ps.h"

#include <U2Core/Log.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/TextUtils.h>
#include <U2Gui/DialogUtils.h>

#include <QtGui/QImageWriter>

namespace U2 {

ExportImage3DGLDialog::ExportImage3DGLDialog( BioStruct3DGLWidget* widget ) : ExportImageDialog(widget, true, true), glWidget(widget)
{
}

bool ExportImage3DGLDialog::exportToSVG(){
    int opt = GL2PS_NONE;
    glWidget->writeImage2DToFile(GL2PS_SVG, opt, 2, qPrintable(getFilename()));
    return true; //TODO: need check on error
}
bool ExportImage3DGLDialog::exportToPDF(){
    int opt = GL2PS_NONE;
    if(getFormat() == "ps"){
        glWidget->writeImage2DToFile(GL2PS_PS, opt, 2, qPrintable(getFilename()));
        return true; //TODO: need check on error
    }else if(getFormat() == "pdf"){
        glWidget->writeImage2DToFile(GL2PS_PDF, opt, 2, qPrintable(getFilename()));
        return true; //TODO: need check on error
    }
    return false;
}
bool ExportImage3DGLDialog::exportToBitmap(){
    QPixmap image = glWidget->renderPixmap(getWidth(), getHeight());
    if(hasQuality()){
        return image.save(getFilename(), qPrintable(getFormat()), getQuality());
    }else{
        return image.save(getFilename(), qPrintable(getFormat()));
    }

}

} // namespace

