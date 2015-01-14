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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QCheckBox>
#else
#include <QtWidgets/QCheckBox>
#endif

#include "MSAImageExportTask.h"
#include "../MSASelectSubalignmentDialog.h"

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorNameList.h>
#include <U2View/MSAEditorConsensusArea.h>
#include <U2View/MSAEditorSequenceArea.h>

#include "ui_MSAExportSettings.h"

namespace U2 {

MSAImageExportToBitmapTask::MSAImageExportToBitmapTask(MSAEditorUI *ui,
                                                       const MSAImageExportSettings &msaSettings,
                                                       const ImageExportTaskSettings &settings)
    : ImageExportTask(settings),
      ui(ui),
      msaSettings(msaSettings)
{
    SAFE_POINT_EXT(ui != NULL, setError(tr("MSA Editor UI is NULL")), );
}

void MSAImageExportToBitmapTask::run() {
    SAFE_POINT_EXT(settings.isBitmapFormat(),
                   setError(WRONG_FORMAT_MESSAGE.arg(settings.format).arg("MSAImageExportToBitmapTask")), );

    SAFE_POINT_EXT( ui->getEditor() != NULL, setError(tr("MSA Editor is NULL")), );
    MAlignmentObject *mObj =  ui->getEditor()->getMSAObject();
    SAFE_POINT_EXT( mObj != NULL, setError(tr("Alignment object is NULL")), );
    StateLock *lock = new StateLock();
    mObj->lockState(lock);

    bool exportAll = msaSettings.exportAll;

    int ok = exportAll || (!msaSettings.region.isEmpty() && !msaSettings.seqIdx.isEmpty());
    CHECK_OPERATION( ok, mObj->unlockState(lock));
    SAFE_POINT_EXT( ok, setError(tr("Nothing to export")), );

    QPixmap seqPixmap;
    QPixmap namesPix;
    QPixmap consPix;
    QPixmap rulerPix;

    ok = paintContent(seqPixmap);
    CHECK_OPERATION( ok, mObj->unlockState(lock));
    CHECK_EXT( ok, setError(tr("Alignment is too big. ") + EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );

    paintSeqNames(namesPix);
    paintConsensus(consPix);
    paintRuler(rulerPix);
    mObj->unlockState(lock);

    QPixmap pixmap = mergePixmaps(seqPixmap, namesPix, consPix, rulerPix);
    CHECK_EXT( !pixmap.isNull(),
               setError(EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );
    CHECK_EXT( pixmap.save(settings.fileName, qPrintable(settings.format), settings.imageQuality),
               setError(EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );
}

void MSAImageExportToBitmapTask::paintSeqNames(QPixmap &pixmap) {
    if (msaSettings.includeSeqNames) {
        MSAEditorNameList* namesArea = ui->getEditorNameList();
        SAFE_POINT_EXT( ui->getEditor() != NULL, setError(tr("MSA Editor is NULL")), );
        if (msaSettings.exportAll && msaSettings.seqIdx.size() != ui->getEditor()->getNumSequences()) {
            msaSettings.seqIdx.clear();
            for (qint64 i = 0; i < ui->getEditor()->getNumSequences(); i++) {
                msaSettings.seqIdx.append(i);
            }
        }
        pixmap = namesArea->drawNames(msaSettings.seqIdx);
    }
}

void MSAImageExportToBitmapTask::paintConsensus(QPixmap &pixmap) {
    CHECK( msaSettings.includeConsensus, );
    MSAEditorConsensusArea* consArea = ui->getConsensusArea();
    SAFE_POINT_EXT( consArea != NULL, setError(tr("MSA Consensus area is NULL")), );
    if (msaSettings.exportAll) {
        consArea->paintFullConsensusToPixmap(pixmap);
        return;
    }
    consArea->paintParsialConsenusToPixmap(pixmap, msaSettings.region, msaSettings.seqIdx);
}

void MSAImageExportToBitmapTask::paintRuler(QPixmap &pixmap) {
    if (msaSettings.includeRuler) {
        MSAEditorConsensusArea* consArea = ui->getConsensusArea();
        if (msaSettings.exportAll) {
            SAFE_POINT_EXT( ui->getEditor() != NULL, setError(tr("MSA Editor is NULL")), );
            msaSettings.region = U2Region( 0, ui->getEditor()->getAlignmentLen() - 1);
        }
        consArea->paintPartOfARuler(pixmap, msaSettings.region);
    }
}

bool MSAImageExportToBitmapTask::paintContent(QPixmap &pixmap) {
    MSAEditorSequenceArea* seqArea = ui->getSequenceArea();
    if (msaSettings.exportAll) {
        return seqArea->paintAllToPixmap(pixmap);
    } else {
        return seqArea->paintToPixmap(pixmap, msaSettings.region, msaSettings.seqIdx);
    }
}

QPixmap MSAImageExportToBitmapTask::mergePixmaps(const QPixmap &seqPix,
                                              const QPixmap &namesPix,
                                              const QPixmap &consPix,
                                              const QPixmap &rulerPix)
{
    CHECK( namesPix.width() + seqPix.width() < 32768 &&
           consPix.height() + rulerPix.height() + seqPix.height() < 32768, QPixmap());
    QPixmap pixmap = QPixmap(namesPix.width() + seqPix.width(),
                             consPix.height() + rulerPix.height() + seqPix.height());

    pixmap.fill(Qt::white);
    QPainter p(&pixmap);

    p.translate(namesPix.width(), 0);
    p.drawPixmap(consPix.rect(), consPix);
    p.translate(0, consPix.height());
    p.drawPixmap(rulerPix.rect(), rulerPix);
    p.translate(-namesPix.width(), rulerPix.height());
    p.drawPixmap(namesPix.rect(), namesPix);
    p.translate(namesPix.width(), 0);
    p.drawPixmap(seqPix.rect(), seqPix);
    p.end();

    return pixmap;
}


MSAImageExportTaskFactory::MSAImageExportTaskFactory(MSAEditorUI *ui)
    : ImageExportTaskFactory(),
      ui(ui)
{
    SAFE_POINT(ui != NULL, tr("MSA editor is NULL"), );
    shortDescription = tr("Alignment");
    initSettingsWidget();
}

MSAImageExportTaskFactory::~MSAImageExportTaskFactory() {
    delete settingsUi;
}

void MSAImageExportTaskFactory::sl_showSelectRegionDialog() {
    SelectSubalignmentDialog dialog(ui);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted) {
        msaSettings.region = dialog.getRegion();
        msaSettings.seqIdx = dialog.getSelectedSeqIndexes();
        if (settingsUi->comboBox->currentIndex() != 1/*customIndex*/) {
            settingsUi->comboBox->setCurrentIndex( 1/*customIndex*/);
            msaSettings.exportAll = false;
        }
    } else {
        if (msaSettings.region.isEmpty()) {
            settingsUi->comboBox->setCurrentIndex( 0/*wholeAlIndex*/);
            msaSettings.exportAll = true;
        }
    }
}

void MSAImageExportTaskFactory::sl_regionChanged() {
    bool customRegionIsSelected = (settingsUi->comboBox->currentIndex() == 1);
    msaSettings.exportAll = !customRegionIsSelected;
    if (customRegionIsSelected && msaSettings.region.isEmpty()) {
            sl_showSelectRegionDialog();
    }
}

void MSAImageExportTaskFactory::initSettingsWidget() {
    settingsUi = new Ui_MSAExportSettings;
    settingsWidget = new QWidget();
    settingsUi->setupUi(settingsWidget);

    connect(settingsUi->selectRegionButton, SIGNAL(clicked()), SLOT(sl_showSelectRegionDialog()));
    connect(settingsUi->comboBox, SIGNAL(currentIndexChanged(int)), SLOT(sl_regionChanged()));

    SAFE_POINT( ui->getSequenceArea() != NULL, tr("MSA sequence area is NULL"), );
    MSAEditorSelection selection = ui->getSequenceArea()->getSelection();
    CHECK( !selection.isNull(), );
    msaSettings.region = U2Region( selection.x(), selection.width());
    msaSettings.seqIdx.clear();
    for (qint64 i = selection.y(); i < selection.height() + selection.y(); i++) {
        msaSettings.seqIdx.append( i );
    }
}

Task* MSAImageExportTaskFactory::getExportToBitmapTask(const ImageExportTaskSettings &settings) const {
    msaSettings.includeConsensus = settingsUi->exportConsensus->isChecked();
    msaSettings.includeRuler = settingsUi->exportRuler->isChecked();
    msaSettings.includeSeqNames = settingsUi->exportSeqNames->isChecked();

    return new MSAImageExportToBitmapTask(ui, msaSettings, settings);
}

} // namespace
