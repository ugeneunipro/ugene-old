/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>
#else
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QVBoxLayout>
#endif

#include <U2Core/U2SafePoints.h>

#include "DotPlotImageExportTask.h"
#include "DotPlotWidget.h"

namespace U2 {

void DotPlotImageExportToBitmapTask::run() {
    SAFE_POINT_EXT(settings.isBitmapFormat(),
                   setError(WRONG_FORMAT_MESSAGE.arg(settings.format).arg("DotPlotImageExportToBitmapTask")), );

    QPixmap *im = new QPixmap(settings.imageSize);
    im->fill(Qt::white);
    QPainter painter(im);
    dotplotWidget->drawAll(painter, settings.imageSize, dpExportSettings);

    CHECK_EXT( im->save(settings.fileName, qPrintable(settings.format), settings.imageQuality),
               setError(EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );
}

DotPlotImageExportTaskFactory::DotPlotImageExportTaskFactory(DotPlotWidget *wgt)
    : ImageExportTaskFactory(),
      dotplotWidget(wgt)
{
    SAFE_POINT( dotplotWidget != NULL, tr("Dotplot widget is NULL"), );
    shortDescription = tr("DotPlot");
    initSettingsWidget();
}

int DotPlotImageExportTaskFactory::getImageWidth() const {
    return dotplotWidget->width();
}

int DotPlotImageExportTaskFactory::getImageHeight() const {
    return dotplotWidget->height();
}

void DotPlotImageExportTaskFactory::initSettingsWidget() {
    QVBoxLayout* layout = new QVBoxLayout();

    includeAreaSelection = new QCheckBox(tr("Include area selection"));
    includeRepeatSelection = new QCheckBox(tr("Include repeat selection"));

    includeAreaSelection->setChecked(false);
    includeRepeatSelection->setChecked(false);

    includeAreaSelection->setEnabled(dotplotWidget->hasSelectedArea());
    includeRepeatSelection->setEnabled(dotplotWidget->nearestRepeat != NULL);

    layout->addWidget(includeAreaSelection);
    layout->addWidget(includeRepeatSelection);

    settingsWidget = new QWidget();
    settingsWidget->setLayout(layout);
}

Task* DotPlotImageExportTaskFactory::getExportToBitmapTask(const ImageExportTaskSettings &settings) const {
    DotPlotImageExportSettings dpSettings(includeAreaSelection->isChecked(),
                                          includeRepeatSelection->isChecked());
    return new DotPlotImageExportToBitmapTask(dotplotWidget, dpSettings, settings);
}

} // namespace
