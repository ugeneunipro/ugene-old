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

#ifndef _U2_IMAGE_EXPORT_TASK_H
#define _U2_IMAGE_EXPORT_TASK_H

#include <U2Core/global.h>
#include <U2Core/Task.h>

#include <QtCore/QSize>

namespace U2 {

class U2GUI_EXPORT ImageExportTaskSettings {
public:
    ImageExportTaskSettings(const QString &fileName = QString(),
                            const QString &format = QString(),
                            const QSize &size = QSize(),
                            const int quality = -1);

    bool isBitmapFormat() const;
    bool isSVGFormat() const;
    bool isPDFFormat() const;

    QString fileName;
    QString format;
    QSize   imageSize;
    int     imageQuality;

    static const QString SVG_FORMAT;
    static const QString PS_FORMAT;
    static const QString PDF_FORMAT;
};


class U2GUI_EXPORT ImageExportTask : public Task {
    Q_OBJECT
public:
    ImageExportTask(const ImageExportTaskSettings &settings)
        : Task(tr("Image export task"), TaskFlag_RunInMainThread),
          settings(settings) {}
    virtual void run() = 0;
    ReportResult report();
protected:
    ImageExportTaskSettings settings;
    static const QString WRONG_FORMAT_MESSAGE;
    static const QString EXPORT_FAIL_MESSAGE;
};


class U2GUI_EXPORT ImageExportTaskFactory : public QObject {
public:
    ImageExportTaskFactory();

    Task*   getTaskInstance(const ImageExportTaskSettings &settings) const;

    const QString& getExportDescription() const { return shortDescription; }
    QWidget* getSettingsWidget();

    virtual int getImageWidth() const { return 0; }
    virtual int getImageHeight() const { return 0; }

protected:
    virtual void initSettingsWidget() = 0;

    virtual Task* getExportToSVGTask(const ImageExportTaskSettings &) const { return NULL; }
    virtual Task* getExportToPDFTask(const ImageExportTaskSettings &) const { return NULL; }
    virtual Task* getExportToBitmapTask(const ImageExportTaskSettings &) const { return NULL; }

    QWidget*    settingsWidget;
    QString     shortDescription;
};

} // namespace

#endif // _U2_IMAGE_EXPORT_TASK_H
