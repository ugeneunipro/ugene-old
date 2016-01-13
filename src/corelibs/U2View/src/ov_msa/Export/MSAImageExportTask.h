/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorNameList.h>
#include <U2View/MSAEditorConsensusArea.h>
#include <U2View/MSAEditorSequenceArea.h>

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

class MSAImageExportTask : public ImageExportTask {
    Q_OBJECT
public:
    MSAImageExportTask(MSAEditorUI *ui,
                       const MSAImageExportSettings &msaSettings,
                       const ImageExportTaskSettings &settings);
    virtual void run() = 0;
protected:
    // P - QPainter or QPixmap
    template<class P>
    void paintSeqNames(P& p) {
        if (msaSettings.includeSeqNames) {
            MSAEditorNameList* namesArea = ui->getEditorNameList();
            SAFE_POINT_EXT( ui->getEditor() != NULL, setError(tr("MSA Editor is NULL")), );
            namesArea->drawNames(p, msaSettings.seqIdx);
        }
    }

    template<class P>
    void paintConsensus(P& p) {
        CHECK( msaSettings.includeConsensus, );
        MSAEditorConsensusArea* consArea = ui->getConsensusArea();
        SAFE_POINT_EXT( consArea != NULL, setError(tr("MSA Consensus area is NULL")), );
        if (msaSettings.exportAll) {
            consArea->paintFullConsensus(p);
            return;
        }
        consArea->paintConsenusPart(p, msaSettings.region, msaSettings.seqIdx);
    }

    template<class P>
    void paintRuler(P& p) {
        if (msaSettings.includeRuler) {
            MSAEditorConsensusArea* consArea = ui->getConsensusArea();
            consArea->paintRulerPart(p, msaSettings.region);
        }
    }

    template<class P>
    bool paintContent(P& p) {
        MSAEditorSequenceArea* seqArea = ui->getSequenceArea();
            return seqArea->drawContent(p, msaSettings.region, msaSettings.seqIdx);
    }

    MSAEditorUI* ui;
    MSAImageExportSettings  msaSettings;
};

class MSAImageExportToBitmapTask : public MSAImageExportTask {
    Q_OBJECT
public:
    MSAImageExportToBitmapTask(MSAEditorUI *ui,
                               const MSAImageExportSettings& msaSettings,
                               const ImageExportTaskSettings &settings);
    void run();

private:
    QPixmap mergePixmaps(const QPixmap& seqPix, const QPixmap& namesPix,
                         const QPixmap& consPix, const QPixmap& rulerPix);
};

class MSAImageExportToSvgTask : public MSAImageExportTask {
    Q_OBJECT
public:
    MSAImageExportToSvgTask(MSAEditorUI *ui,
                            const MSAImageExportSettings& msaSettings,
                            const ImageExportTaskSettings &settings);
    void run();
};

class MSAImageExportController : public ImageExportController {
    Q_OBJECT
public:
    MSAImageExportController(MSAEditorUI *ui);
    ~MSAImageExportController();

public slots:
    void sl_showSelectRegionDialog();
    void sl_regionChanged();

protected:
    void initSettingsWidget();

    Task* getExportToBitmapTask(const ImageExportTaskSettings &settings) const;
    Task* getExportToSvgTask(const ImageExportTaskSettings &) const;

private slots:
    void sl_onFormatChanged(const QString&);
private:
    void checkRegionToExport();
    bool fitsInLimits() const;
    bool canExportToSvg() const;
    void updateSeqIdx() const;

    MSAEditorUI* ui;
    Ui_MSAExportSettings    *settingsUi;
    mutable MSAImageExportSettings      msaSettings;
    QString format;
};

} // namespace

#endif // _U2_MSA_IMAGE_EXPORT_TASK_H_
