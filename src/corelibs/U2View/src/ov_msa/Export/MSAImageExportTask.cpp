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

#include <QCheckBox>
#include <QSvgGenerator>

#include <U2Core/L10n.h>

#include <U2Core/QObjectScopedPointer.h>

#include "MSAImageExportTask.h"
#include "ui_MSAExportSettings.h"
#include "../MSASelectSubalignmentDialog.h"

namespace U2 {

MSAImageExportTask::MSAImageExportTask(MSAEditorUI *ui,
                                       const MSAImageExportSettings &msaSettings,
                                       const ImageExportTaskSettings &settings)
    : ImageExportTask(settings),
      ui(ui),
      msaSettings(msaSettings) {
    SAFE_POINT_EXT(ui != NULL, setError(tr("MSA Editor UI is NULL")), );
}

MSAImageExportToBitmapTask::MSAImageExportToBitmapTask(MSAEditorUI *ui,
                                                       const MSAImageExportSettings& msaSettings,
                                                       const ImageExportTaskSettings &settings)
    : MSAImageExportTask(ui,
                         msaSettings,
                         settings) {
}

void MSAImageExportToBitmapTask::run() {
    SAFE_POINT_EXT(settings.isBitmapFormat(),
                   setError(WRONG_FORMAT_MESSAGE.arg(settings.format).arg("MSAImageExportToBitmapTask")), );

    SAFE_POINT_EXT( ui->getEditor() != NULL, setError(L10N::nullPointerError("MSAEditor")), );
    MAlignmentObject *mObj =  ui->getEditor()->getMSAObject();
    SAFE_POINT_EXT( mObj != NULL, setError(L10N::nullPointerError("MAlignmentObject")), );
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
               setError(tr("Alignment is too big. ") + EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );
    CHECK_EXT( pixmap.save(settings.fileName, qPrintable(settings.format), settings.imageQuality),
               setError(tr("Cannot save the file. ") + EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );
}

QPixmap MSAImageExportToBitmapTask::mergePixmaps(const QPixmap &seqPix,
                                              const QPixmap &namesPix,
                                              const QPixmap &consPix,
                                              const QPixmap &rulerPix)
{
    CHECK( namesPix.width() + seqPix.width() < IMAGE_SIZE_LIMIT &&
           consPix.height() + rulerPix.height() + seqPix.height() < IMAGE_SIZE_LIMIT, QPixmap());
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

MSAImageExportToSvgTask::MSAImageExportToSvgTask(MSAEditorUI *ui,
                                                 const MSAImageExportSettings& msaSettings,
                                                 const ImageExportTaskSettings &settings)
    : MSAImageExportTask(ui,
                         msaSettings,
                         settings) {
}

void MSAImageExportToSvgTask::run() {
    SAFE_POINT_EXT(settings.isSVGFormat(),
                   setError(WRONG_FORMAT_MESSAGE.arg(settings.format).arg("MSAImageExportToSvgTask")), );

    MSAEditor* editor = ui->getEditor();
    SAFE_POINT_EXT( editor != NULL, setError(L10N::nullPointerError("MSAEditor")), );
    MAlignmentObject *mObj =  editor->getMSAObject();
    SAFE_POINT_EXT( mObj != NULL, setError(L10N::nullPointerError("MAlignmentObject")), );

    StateLocker stateLocker(mObj);
    Q_UNUSED(stateLocker);

    int ok = msaSettings.exportAll || (!msaSettings.region.isEmpty() && !msaSettings.seqIdx.isEmpty());
    SAFE_POINT_EXT( ok, setError(tr("Nothing to export")), );

    QSvgGenerator generator;
    generator.setFileName(settings.fileName);

    MSAEditorNameList* nameListArea = ui->getEditorNameList();
    SAFE_POINT_EXT(nameListArea != NULL, setError(L10N::nullPointerError("MSAEditorNameList")), );
    MSAEditorConsensusArea* consArea = ui->getConsensusArea();
    SAFE_POINT_EXT(consArea != NULL, setError(L10N::nullPointerError("MSAEditorConsensusArea")), );

    int namesWidth = nameListArea->width();
    int consHeight = consArea->getRullerLineYRange().startPos;
    int rulerHeight = consArea->getRullerLineYRange().length;

    int w = msaSettings.includeSeqNames * namesWidth +
            editor->getColumnWidth() * (msaSettings.exportAll ? editor->getAlignmentLen() : msaSettings.region.length);
    int h = msaSettings.includeConsensus * consHeight +
            msaSettings.includeRuler * rulerHeight +
            editor->getRowHeight() * (msaSettings.exportAll ? editor->getNumSequences() : msaSettings.seqIdx.size());
    SAFE_POINT_EXT(qMax(w, h) < IMAGE_SIZE_LIMIT, setError(tr("The image size is too big.") + EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );

    generator.setSize(QSize(w, h));
    generator.setViewBox(QRect(0, 0, w, h));
    generator.setTitle(tr("SVG %1").arg(mObj->getGObjectName()));
    generator.setDescription(tr("SVG image of multiple alignment created by Unipro UGENE"));

    QPainter p;
    p.begin(&generator);

    if ((msaSettings.includeConsensus || msaSettings.includeRuler) && (msaSettings.includeSeqNames)) {
        // fill an empty space in top left corner with white color
        p.fillRect(QRect(0, 0, namesWidth, msaSettings.includeConsensus * consHeight + msaSettings.includeRuler * rulerHeight), Qt::white);
    }
    p.translate( msaSettings.includeSeqNames * namesWidth, 0);
    paintConsensus(p);
    p.translate( 0, msaSettings.includeConsensus * consHeight );
    paintRuler(p);
    p.translate( -1 * msaSettings.includeSeqNames * namesWidth, msaSettings.includeRuler * rulerHeight);
    paintSeqNames(p);
    p.translate( msaSettings.includeSeqNames * namesWidth, 0);
    paintContent(p);
    p.end();
}


MSAImageExportController::MSAImageExportController(MSAEditorUI *ui)
    : ImageExportController( ExportImageFormatPolicy( EnableRasterFormats | SupportSvg) ),
      ui(ui)
{
    SAFE_POINT(ui != NULL, L10N::nullPointerError("MSAEditorUI"), );
    shortDescription = tr("Alignment");
    initSettingsWidget();
    checkRegionToExport();
}

MSAImageExportController::~MSAImageExportController() {
    delete settingsUi;
}

void MSAImageExportController::sl_showSelectRegionDialog() {
    QObjectScopedPointer<SelectSubalignmentDialog> dialog = new SelectSubalignmentDialog(ui, msaSettings.region, msaSettings.seqIdx);
    dialog->exec();
    CHECK(!dialog.isNull(), );

    if (dialog->result() == QDialog::Accepted) {
        msaSettings.region = dialog->getRegion();
        msaSettings.seqIdx = dialog->getSelectedSeqIndexes();
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
    checkRegionToExport();
}

void MSAImageExportController::sl_regionChanged() {
    bool customRegionIsSelected = (settingsUi->comboBox->currentIndex() == 1);
    msaSettings.exportAll = !customRegionIsSelected;
    if (customRegionIsSelected && msaSettings.region.isEmpty()) {
        sl_showSelectRegionDialog();
    } else {
        checkRegionToExport();
    }
}

void MSAImageExportController::initSettingsWidget() {
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
    if (!ui->isCollapsibleMode()) {
        for (qint64 i = selection.y(); i < selection.height() + selection.y(); i++) {
            msaSettings.seqIdx.append( i );
        }
    } else {
        MSACollapsibleItemModel* model = ui->getCollapseModel();
        SAFE_POINT(model != NULL, tr("MSA Collapsible Model is NULL"), );
        for (qint64 i = selection.y(); i < selection.height() + selection.y(); i++) {
                msaSettings.seqIdx.append(model->mapToRow(i));
        }
    }
}

Task* MSAImageExportController::getExportToBitmapTask(const ImageExportTaskSettings &settings) const {
    msaSettings.includeConsensus = settingsUi->exportConsensus->isChecked();
    msaSettings.includeRuler = settingsUi->exportRuler->isChecked();
    msaSettings.includeSeqNames = settingsUi->exportSeqNames->isChecked();
    updateSeqIdx();

    return new MSAImageExportToBitmapTask(ui, msaSettings, settings);
}

Task* MSAImageExportController::getExportToSvgTask(const ImageExportTaskSettings &settings) const {
    msaSettings.includeConsensus = settingsUi->exportConsensus->isChecked();
    msaSettings.includeRuler = settingsUi->exportRuler->isChecked();
    msaSettings.includeSeqNames = settingsUi->exportSeqNames->isChecked();
    updateSeqIdx();

    return new MSAImageExportToSvgTask(ui, msaSettings, settings);
}

void MSAImageExportController::sl_onFormatChanged(const QString& newFormat) {
    format = newFormat;
    checkRegionToExport();
}

void MSAImageExportController::checkRegionToExport() {
    bool exportToSvg = format.contains("svg", Qt::CaseInsensitive);
    bool isRegionOk = fitsInLimits();
    disableMessage = isRegionOk ? "" : tr("Warning: selected region is too big to be exported. You can try to zoom out the alignment or select another region.");
    if(isRegionOk && exportToSvg) {
        isRegionOk = canExportToSvg();
        disableMessage = isRegionOk ? "" : tr("Warning: selected region is too big to be exported. You can try to select another region.");
    }

    emit si_disableExport( !isRegionOk );
    emit si_showMessage( disableMessage );
}

namespace {
//400000 characters convert to 200 mb file in SVG format
const qint64 MaxSvgCharacters = 400000;
//SVG renderer can crash on regions large than 40000000
const qint64 MaxSvgImageSize = 40000000;
}

bool MSAImageExportController::fitsInLimits() const {
    MSAEditor* editor = ui->getEditor();
    SAFE_POINT(editor != NULL, L10N::nullPointerError("MSAEditor"), false);
    qint64 imageWidth = (msaSettings.exportAll ? editor->getAlignmentLen() : msaSettings.region.length) * editor->getColumnWidth();
    qint64 imageHeight = (msaSettings.exportAll ? editor->getNumSequences() : msaSettings.seqIdx.size()) * editor->getRowHeight();
    if (imageWidth > IMAGE_SIZE_LIMIT || imageHeight > IMAGE_SIZE_LIMIT) {
        return false;
    }
    if (format.contains("svg", Qt::CaseInsensitive) && imageWidth * imageHeight > MaxSvgImageSize) {
        return false;
    }
    return true;
}

bool MSAImageExportController::canExportToSvg() const {
    MSAEditor* editor = ui->getEditor();
    SAFE_POINT(editor != NULL, L10N::nullPointerError("MSAEditor"), false);
    int charactersNumber = msaSettings.exportAll ? (editor->getNumSequences() * editor->getAlignmentLen()) : (msaSettings.region.length * msaSettings.seqIdx.size());
    return charactersNumber < MaxSvgCharacters;
}

void MSAImageExportController::updateSeqIdx() const {
    CHECK(msaSettings.exportAll, );
    if (!ui->isCollapsibleMode()) {
        msaSettings.seqIdx.clear();
        for (qint64 i = 0; i < ui->getEditor()->getNumSequences(); i++) {
            msaSettings.seqIdx.append(i);
        }
        msaSettings.region = U2Region(0, ui->getEditor()->getAlignmentLen());
    }

    CHECK(ui->isCollapsibleMode(), );

    MSACollapsibleItemModel* model = ui->getCollapseModel();
    SAFE_POINT(model != NULL, tr("MSA Collapsible Model is NULL"), );
    msaSettings.seqIdx.clear();
    for (qint64 i = 0; i < ui->getEditor()->getNumSequences(); i++) {
        if (model->rowToMap(i, true) != -1) {
            msaSettings.seqIdx.append(i);
        }
    }
}

} // namespace
