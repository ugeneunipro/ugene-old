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

#include "ImageExporter.h"
#include <U2Core/U2SafePoints.h>

#include <QtCore/QFile>
#include <QtGui/QPainter>
#include <QtSvg/QSvgGenerator>
#include <QtXml/QDomDocument>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPrinter>
#else
#include <QtPrintSupport/QPrinter>
#endif


namespace U2 {

ImageExporter::ImageExporter(ImageScalingPolicy scalingPolicy, FormatPolicy formatPolicy)
    : settingsWidget(NULL),
      scalingPolicy(scalingPolicy),
      formatPolicy(formatPolicy)
{}

bool ImageExporter::exportToSVG(const QString & /*filename*/) const {
    FAIL(tr("Not implemented"), false);
}

bool ImageExporter::exportToPDF(const QString & /*filename*/, const QString &/*format*/) const {
    FAIL(tr("Not implemented"), false);
}

bool ImageExporter::exportToBitmap(const QString &/*filename*/, const QString &/*format*/,
                                   const QSize &/*size*/, int /*quality*/) const {
    FAIL(tr("Not implemented"), false);
}

void ImageExporter::sl_setSettingsWidgetVisible(bool visible) {
    if (settingsWidget != NULL) {
        settingsWidget->setVisible(visible);
    }
}

//----------------------------------------------------------------------------------------------------------------

WidgetScreenshotImageExporter::WidgetScreenshotImageExporter(QWidget *widget,
                                                             ImageScalingPolicy scalingPolicy, FormatPolicy formatPolicy)
    : ImageExporter(scalingPolicy, formatPolicy),
      widget(widget)
{
    shortDescription = tr("Screenshot");
}

bool WidgetScreenshotImageExporter::exportToSVG(const QString &filename) const {
    bool result=false;
    QPainter painter;
    QSvgGenerator generator;
    generator.setFileName(filename);

    generator.setSize(widget->rect().size());

    generator.setViewBox(widget->rect());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(&generator);
    widget->render(&painter);
    result = painter.end();
    QDomDocument doc("svg");

    QFile file(filename);

    bool ok=file.open(QIODevice::ReadOnly);
    CHECK(ok, false);

    ok = doc.setContent(&file);
    CHECK(ok, false);

    if (result) {
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
    }
    return result;
}

bool WidgetScreenshotImageExporter::exportToPDF(const QString &filename, const QString &) const {
    QPainter painter;
    QPrinter printer;
    printer.setOutputFileName(filename);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(&printer);
    widget->render(&painter);
    return painter.end();
}

bool WidgetScreenshotImageExporter::exportToBitmap(const QString &filename, const QString &format, const QSize &size, int quality) const {
    QPixmap image = QPixmap::grabWidget(widget, widget->rect());
    image = image.scaled(size, Qt::KeepAspectRatio);
    return image.save(filename, qPrintable(format), quality);
}

} // namespace
