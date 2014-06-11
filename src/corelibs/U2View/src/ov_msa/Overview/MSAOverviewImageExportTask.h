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

#ifndef _U2_MSA_OVERVIEW_IMAGE_EXPORT_TASK_H_
#define _U2_MSA_OVERVIEW_IMAGE_EXPORT_TASK_H_

#include <U2Gui/ImageExportTask.h>

class QCheckBox;

namespace U2 {

class MSASimpleOverview;
class MSAGraphOverview;

class MSAOverviewImageExportSettings {
public:
    MSAOverviewImageExportSettings(bool exportSimpleOverview = false,
                                   bool exportGraphOverview = true)
        : exportSimpleOverview(exportSimpleOverview),
          exportGraphOverview(exportGraphOverview) {}

    bool exportSimpleOverview;
    bool exportGraphOverview;
};


class MSAOverviewImageExportToBitmapTask : public ImageExportTask {
public:
    MSAOverviewImageExportToBitmapTask(MSASimpleOverview *simpleOverview, MSAGraphOverview *graphOverview,
                                       const MSAOverviewImageExportSettings &overviewSettings,
                                       const ImageExportTaskSettings& settings);
    void run();
private:
    MSASimpleOverview*  simpleOverview;
    MSAGraphOverview*   graphOverview;
    MSAOverviewImageExportSettings  overviewSettings;
};


class MSAOverviewImageExportTaskFactory : public ImageExportTaskFactory {
public:
    MSAOverviewImageExportTaskFactory(MSASimpleOverview *simpleOverview, MSAGraphOverview *graphOverview);

    int getImageWidth() const;
    int getImageHeight() const;

protected:
    void initSettingsWidget();

    Task* getExportToBitmapTask(const ImageExportTaskSettings &settings) const;

private:
    MSASimpleOverview*  simpleOverview;
    MSAGraphOverview*   graphOverview;

    QCheckBox*   exportSimpleOverview;
    QCheckBox*   exportGraphOverview;
};

} // namespace

#endif // _U2_MSA_OVERVIEW_IMAGE_EXPORT_TASK_H_
