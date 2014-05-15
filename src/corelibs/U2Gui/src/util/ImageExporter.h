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

#ifndef _U2_IMAGE_EXPORTER_H_
#define _U2_IMAGE_EXPORTER_H_

#include <U2Core/global.h>

#include <QtGui/QWidget>

namespace U2 {

class U2GUI_EXPORT ImageExporter : public QObject {
    Q_OBJECT
public:
    enum ImageScalingPolicy {
        Constant,
        Resizable
    };
    enum FormatPolicy {
        SupportVectorFormats,
        IgnoreVectorFormats
    };

    ImageExporter(ImageScalingPolicy scalingPolicy = Constant, FormatPolicy formatPolicy = IgnoreVectorFormats);

    const QString& getExportDescription() const { return shortDescription; }
    QWidget* getSettingsWidget() { return settingsWidget; }

    /**
     * In successors exportToBitmap function must be implemented.
     * If format policy supports vector formats, exportToSVG and exportToPDF functions must be implemented as well.
     */
    virtual bool exportToSVG(const QString &filename) const;
    virtual bool exportToPDF(const QString &filename, const QString &format) const;
    virtual bool exportToBitmap(const QString &filename, const QString &format, const QSize &size, int quality) const;

    /**
     * Redefinition of the following functions is required if scaling policy is set to Resizable.
     */
    virtual int getImageWidth() const {return 0;}
    virtual int getImageHeight() const {return 0;}

    ImageScalingPolicy getScalingPolicy() const { return scalingPolicy; }
    FormatPolicy getFormatPolicy() const { return formatPolicy; }

    virtual bool isAvailable() const { return true; }

public slots:
    void sl_setSettingsWidgetVisible(bool visible);

protected:
    virtual void initSettingsWidget() = 0;

    QWidget*    settingsWidget;
    QString     shortDescription;

    ImageScalingPolicy scalingPolicy;
    FormatPolicy formatPolicy;

}; // class ImageExporter

class WidgetScreenshotImageExporter : public ImageExporter {
    Q_OBJECT
public:
    WidgetScreenshotImageExporter(QWidget *widget, ImageScalingPolicy scalingPolicy = Constant, FormatPolicy formatPolicy = IgnoreVectorFormats);

    bool exportToSVG(const QString &filename) const;
    bool exportToPDF(const QString &filename, const QString &format) const;
    bool exportToBitmap(const QString &filename, const QString &format, const QSize& size, int quality = -1) const;

    int getImageWidth() const { return widget->width(); }
    int getImageHeight() const { return widget->height(); }

protected:
    void initSettingsWidget() {}

private:
    QWidget *widget;
}; // class WidgetScreenshotImageExporter

} // namespace

#endif // _U2_IMAGE_EXPORTER_H_
