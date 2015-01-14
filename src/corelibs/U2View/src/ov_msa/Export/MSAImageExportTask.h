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

#ifndef _U2_MSA_IMAGE_EXPORT_TASK_H_
#define _U2_MSA_IMAGE_EXPORT_TASK_H_

#include <U2Gui/ImageExportTask.h>
#include <U2Core/U2Region.h>
#include <QtGui/QPixmap>

class Ui_MSAExportSettings;

namespace U2 {

class MSAEditorUI;

class MSAImageExportSettings {
public:

    MSAImageExportSettings(bool exportAll = true,
                           bool includeSeqNames = false,
                           bool includeConsensus = false,
                           bool includeRuler = true)
        : exportAll(exportAll),
          includeSeqNames(includeSeqNames),
          includeConsensus(includeConsensus),
          includeRuler(includeRuler) {}

    MSAImageExportSettings(const U2Region &region,
                           const QList<qint64> &seqIdx,
                           bool includeSeqNames = false,
                           bool includeConsensus = false,
                           bool includeRuler = true)
        : exportAll(false),
          region(region),
          seqIdx(seqIdx),
          includeSeqNames(includeSeqNames),
          includeConsensus(includeConsensus),
          includeRuler(includeRuler) {}

    bool exportAll;
    U2Region region;
    QList<qint64>      seqIdx;

    bool includeSeqNames;
    bool includeConsensus;
    bool includeRuler;
};

class MSAImageExportToBitmapTask : public ImageExportTask {
    Q_OBJECT
public:
    MSAImageExportToBitmapTask(MSAEditorUI *ui,
                               const MSAImageExportSettings& msaSettings,
                               const ImageExportTaskSettings &settings);
    void run();

private:
    void paintSeqNames(QPixmap& pixmap);
    void paintConsensus(QPixmap& pixmap);
    void paintRuler(QPixmap& pixmap);
    bool paintContent(QPixmap& pixmap);

    QPixmap mergePixmaps(const QPixmap& seqPix, const QPixmap& namesPix,
                         const QPixmap& consPix, const QPixmap& rulerPix);

private:
    MSAEditorUI* ui;
    MSAImageExportSettings  msaSettings;
};

class MSAImageExportTaskFactory : public ImageExportTaskFactory {
    Q_OBJECT
public:
    MSAImageExportTaskFactory(MSAEditorUI *ui);
    ~MSAImageExportTaskFactory();

public slots:
    void sl_showSelectRegionDialog();
    void sl_regionChanged();

protected:
    void initSettingsWidget();

    Task* getExportToBitmapTask(const ImageExportTaskSettings &settings) const;

private:
    MSAEditorUI* ui;
    Ui_MSAExportSettings    *settingsUi;
    mutable MSAImageExportSettings      msaSettings;
};

} // namespace

#endif // _U2_MSA_IMAGE_EXPORT_TASK_H_
