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

#include "ExportImageCircularViewDialog.h"
#include "CircularView.h"

#include <U2Core/Log.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/TextUtils.h>
#include <U2Gui/DialogUtils.h>

#include <QtGui/QImageWriter>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPrinter>
#else
#include <QtPrintSupport/QPrinter>
#endif
#include <QDomDocument>
#include <QtSvg/QSvgGenerator>

namespace U2 {

ExportImageCVDialog::ExportImageCVDialog(CircularView * widget ) : ExportImageDialog(widget,true,true), cvWidget(widget)
{
}

bool ExportImageCVDialog::exportToSVG(){
    QPainter painter;
    QSvgGenerator generator;
    generator.setFileName(getFilename());
    generator.setSize(cvWidget->size());
    generator.setViewBox(cvWidget->rect());

    painter.begin(&generator);
    cvWidget->paint(painter);
    bool result = painter.end();
    //fix for UGENE-76
    QDomDocument doc("svg");
    QFile file(getFilename());
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

    return result;
}
bool ExportImageCVDialog::exportToPDF(){
    QPainter painter;
    QPrinter printer;
    printer.setOutputFileName(getFilename());

    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(&printer);
    cvWidget->paint(painter);
    return painter.end();
}

} // namespace

