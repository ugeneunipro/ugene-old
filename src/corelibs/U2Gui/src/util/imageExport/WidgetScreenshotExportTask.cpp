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

#include "WidgetScreenshotExportTask.h"

#include <U2Core/U2SafePoints.h>

#include <QtCore/QFile>
#include <QtGui/QPainter>
#include <QtSvg/QSvgGenerator>
#include <QtXml/QDomDocument>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPrinter>
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#include <QtPrintSupport/QPrinter>
#endif


namespace U2 {


void WidgetScreenshotExportToSvgTask::run() {
    SAFE_POINT_EXT(settings.isSVGFormat(),
                   setError(WRONG_FORMAT_MESSAGE.arg(settings.format).arg("WidgetScreenshotExportToSVGTask")), );

    bool result=false;
    QPainter painter;
    QSvgGenerator generator;
    generator.setFileName(settings.fileName);

    generator.setSize(widget->rect().size());

    generator.setViewBox(widget->rect());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(&generator);
    widget->render(&painter);
    result = painter.end();
    CHECK_EXT( result, setError(tr("Painter is still active")), );

    QDomDocument doc("svg");
    QFile file(settings.fileName);

    bool ok = file.open(QIODevice::ReadOnly);
    CHECK_EXT( ok, setError(tr("Can not open the file: %1").arg(file.fileName())), );

    ok = doc.setContent(&file);
    CHECK_EXT( ok, setError(tr("Can not open the file: %1").arg(file.fileName())), );

    file.close();
    QDomNodeList radialGradients=doc.elementsByTagName("radialGradient");
    for (int i = 0;i < radialGradients.length(); i++) {
        if (radialGradients.at(i).isElement()) {
            QDomElement tag = radialGradients.at(i).toElement();
            if (tag.hasAttribute("xml:id")) {
                QString id = tag.attribute("xml:id");
                tag.removeAttribute("xml:id");
                tag.setAttribute("id",id);
            }
        }
    }
    file.open(QIODevice::WriteOnly);
    ok = file.write(doc.toByteArray());
    CHECK_EXT( ok, setError(EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );
}

void WidgetScreenshotExportToPdfTask::run() {
    SAFE_POINT_EXT(settings.isPDFFormat(),
                   setError(WRONG_FORMAT_MESSAGE.arg(settings.format).arg("WidgetScreenshotExportToPDFTask")), );

    QPainter painter;
    QPrinter printer;
    printer.setOutputFileName(settings.fileName);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(&printer);
    widget->render(&painter);
    CHECK_OPERATION( painter.end(), setError(EXPORT_FAIL_MESSAGE.arg(settings.fileName)));
}

void WidgetScreenshotExportToBitmapTask::run() {
    SAFE_POINT_EXT( settings.isBitmapFormat(),
                    setError(WRONG_FORMAT_MESSAGE.arg(settings.format).arg("WidgetScreenshotExportToBitmapTask")), );

    QImage image = QPixmap::grabWidget(widget, widget->rect()).toImage();

    image = image.scaled(settings.imageSize, Qt::KeepAspectRatio);

    CHECK_OPERATION( image.save(settings.fileName, qPrintable(settings.format), settings.imageQuality),
                     setError(EXPORT_FAIL_MESSAGE.arg(settings.fileName)));
}

WidgetScreenshotImageExportTaskFactory::WidgetScreenshotImageExportTaskFactory(QWidget *widget)
    : ImageExportTaskFactory(),
      widget(widget)
{
    shortDescription = tr("Screenshot");
}

int WidgetScreenshotImageExportTaskFactory::getImageWidth() const {
    return widget->width();
}

int WidgetScreenshotImageExportTaskFactory::getImageHeight() const {
    return widget->height();
}

Task* WidgetScreenshotImageExportTaskFactory::getExportToSVGTask(const ImageExportTaskSettings &settings) const {
    return new WidgetScreenshotExportToSvgTask(widget, settings);
}
Task* WidgetScreenshotImageExportTaskFactory::getExportToPDFTask(const ImageExportTaskSettings &settings) const {
    return new WidgetScreenshotExportToPdfTask(widget, settings);
}
Task* WidgetScreenshotImageExportTaskFactory::getExportToBitmapTask(const ImageExportTaskSettings &settings) const {
    return new WidgetScreenshotExportToBitmapTask(widget, settings);
}

} // namespace
