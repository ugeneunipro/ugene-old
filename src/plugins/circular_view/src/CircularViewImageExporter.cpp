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

#include "CircularViewImageExporter.h"
#include "CircularView.h"

#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPrinter>
#else
#include <QtPrintSupport/QPrinter>
#endif
#include <QtXml/QDomDocument>
#include <QtSvg/QSvgGenerator>
#include <iostream>

namespace U2 {

CircularViewImageExporter::CircularViewImageExporter(CircularView *cv)
    : ImageExporter(ImageExporter::Resizable, ImageExporter::SupportVectorFormats),
      cvWidget(cv)
{
    initSettingsWidget();
}

bool CircularViewImageExporter::exportToBitmap(const QString &filename, const QString &format, const QSize &size, int quality) const {
    QPixmap *im = new QPixmap(size);
    im->fill(Qt::white);
    QPainter *painter = new QPainter(im);
    cvWidget->paint(*painter, size.width(), size.height(),
                    includeSelectionCheckbox->isChecked(),
                    includeMarkerCheckbox->isChecked());

    return im->save(filename, qPrintable(format), quality);
}

bool CircularViewImageExporter::exportToPDF(const QString &filename, const QString &) const {
    QPainter painter;
    QPrinter printer;
    printer.setOutputFileName(filename);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(&printer);
    cvWidget->paint(painter, cvWidget->width(), cvWidget->height(),
                    includeSelectionCheckbox->isChecked(),
                    includeMarkerCheckbox->isChecked());

    return painter.end();
}

bool CircularViewImageExporter::exportToSVG(const QString &filename) const {
    QPainter painter;
    QSvgGenerator generator;
    generator.setFileName(filename);
    generator.setSize(cvWidget->size());
    generator.setViewBox(cvWidget->rect());

    painter.begin(&generator);
    cvWidget->paint(painter, cvWidget->width(), cvWidget->height(),
                    includeSelectionCheckbox->isChecked(),
                    includeMarkerCheckbox->isChecked());
    bool result = painter.end();
    //fix for UGENE-76
    QDomDocument doc("svg");
    QFile file(filename);
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

int CircularViewImageExporter::getImageWidth() const {
    return cvWidget->width();
}

int CircularViewImageExporter::getImageHeight() const {
    return cvWidget->height();
}

void CircularViewImageExporter::initSettingsWidget() {
    QVBoxLayout* layout = new QVBoxLayout();

    includeMarkerCheckbox = new QCheckBox(tr("Include position marker"));
    includeSelectionCheckbox = new QCheckBox(tr("Include selection marker"));

    includeMarkerCheckbox->setChecked(true);
    includeSelectionCheckbox->setChecked(true);

    layout->addWidget(includeMarkerCheckbox);
    layout->addWidget(includeSelectionCheckbox);

    settingsWidget = new QWidget();
    settingsWidget->setLayout(layout);
}

} // namespace

