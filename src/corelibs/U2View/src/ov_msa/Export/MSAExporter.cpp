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

#include "MSAExporter.h"
#include "../MSAEditorOverviewArea.h"
#include "../Overview/MSAGraphOverview.h"
#include "../Overview/MSASimpleOverview.h"

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorNameList.h>
#include <U2View/MSAEditorConsensusArea.h>
#include <U2View/MSAEditorSequenceArea.h>

#include <QtGui/QCheckBox>

namespace U2 {

MSAExporter::MSAExporter(MSAEditorUI *ui)
    : ImageExporter(),
      ui(ui)
{
    SAFE_POINT(ui != NULL, tr("MSAEditorUI is NULL"), );
    shortDescription = tr("Whole alignment");
    initSettingsWidget();
}

bool MSAExporter::exportToBitmap(const QString &filename, const QString &format, const QSize &size, int quality) const {
    Q_UNUSED(size);
    QPixmap seqPixmap;
    QPixmap namesPix;
    QPixmap consPix;
    QPixmap rulerPix;

    if (exportSeqNames->isChecked()) {
        MSAEditorNameList* namesArea = ui->getEditorNameList();
        namesPix = namesArea->getView();
    }
    if (exportConsensus->isChecked()) {
        MSAEditorConsensusArea* consArea = ui->getConsensusArea();
        consArea->paintFullConsensusToPixmap(consPix);
    }
    if (exportRuler->isChecked()) {
        MSAEditorConsensusArea* consArea = ui->getConsensusArea();
        consArea->paintFullRulerToPixmap(rulerPix);
    }

    MSAEditorSequenceArea* seqArea = ui->getSequenceArea();
    seqArea->paintWholeAlignmentToPixmap(seqPixmap);

    QPixmap pixmap = QPixmap(namesPix.width() + seqPixmap.width(), consPix.height() + rulerPix.height() + seqPixmap.height());
    pixmap.fill(Qt::white);
    QPainter p(&pixmap);

    p.translate(namesPix.width(), 0);
    p.drawPixmap(consPix.rect(), consPix);
    p.translate(0, consPix.height());
    p.drawPixmap(rulerPix.rect(), rulerPix);
    p.translate(-namesPix.width(), rulerPix.height());
    p.drawPixmap(0, 0, namesPix.width(), seqArea->height(), namesPix);
    p.translate(namesPix.width(), 0);
    p.drawPixmap(seqPixmap.rect(), seqPixmap);
    p.end();

    return pixmap.save(filename, qPrintable(format), quality);
}

void MSAExporter::initSettingsWidget() {
    settingsWidget = new QWidget();

    exportSeqNames = new QCheckBox(tr("Include sequences names"), settingsWidget);
    exportConsensus = new QCheckBox(tr("Include consensus"), settingsWidget);
    exportRuler = new QCheckBox(tr("Include ruler"), settingsWidget);

    // default checking
    exportRuler->setChecked(true);

    QVBoxLayout *layout = new QVBoxLayout(settingsWidget);
    layout->addWidget(exportSeqNames);
    layout->addWidget(exportConsensus);
    layout->addWidget(exportRuler);

    settingsWidget->setLayout(layout);
}

//---------------------------------------------------------------------------------------------------------------------

MSASelectionExporter::MSASelectionExporter(MSAEditorUI *ui)
    : ImageExporter(),
      ui(ui)
{
    SAFE_POINT(ui != NULL, tr("MSAEditorUI is NULL"), );
    shortDescription = tr("Selected region");
}

bool MSASelectionExporter::exportToBitmap(const QString &filename, const QString &format, const QSize &size, int quality) const {
    Q_UNUSED(size);
    QPixmap pixmap;
    MSAEditorSequenceArea* seqArea = ui->getSequenceArea();
    seqArea->paintSelectedRegionToPixmap(pixmap);
    return pixmap.save(filename, qPrintable(format), quality);
}

bool MSASelectionExporter::isAvailable() {
    SAFE_POINT(ui->getSequenceArea() != NULL, tr("Sequence area is NULL"), false);
    if (ui->getSequenceArea()->getSelection().isNull()) {
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------

MSAOverviewExporter::MSAOverviewExporter(MSAEditorOverviewArea *overviewArea)
    : ImageExporter(),
      overviewArea(overviewArea)
{
    SAFE_POINT(overviewArea != NULL, tr("MSAEditorOverviewArea is NULL"), );
    shortDescription = tr("Overview");
    initSettingsWidget();
}

bool MSAOverviewExporter::exportToBitmap(const QString &fileName, const QString &format, const QSize &size, int quality) const {
    Q_UNUSED(size);
    QPixmap pixmap;

    if (exportSimpleOverview->isChecked()) {
        pixmap = overviewArea->getSimpleOverview()->getView();
    }
    if (exportGraphOverview->isChecked()) {
        pixmap = overviewArea->getGraphOverview()->getView();
    }
    if (exportBothOverviews->isChecked()) {
        QRect rect = overviewArea->getGraphOverview()->rect();
        pixmap = QPixmap(rect.width(), rect.height() + overviewArea->getSimpleOverview()->height());

        QPainter p(&pixmap);
        p.drawPixmap(overviewArea->getSimpleOverview()->rect(),
                     overviewArea->getSimpleOverview()->getView());
        rect.moveTop(overviewArea->getSimpleOverview()->height());
        p.drawPixmap(rect,
                     overviewArea->getGraphOverview()->getView());
        p.end();
    }

    return pixmap.save(fileName, qPrintable(format), quality);
}

int MSAOverviewExporter::getImageWidth() const {
    return overviewArea->width();
}

int MSAOverviewExporter::getImageHeight() const {
    int h = 0;
    if (exportSimpleOverview->isChecked()) {
        h += overviewArea->getSimpleOverview()->height();
    }
    if (exportGraphOverview->isChecked()) {
        h += overviewArea->getGraphOverview()->height();
    }
    if (exportBothOverviews->isChecked()) {
        h += overviewArea->getSimpleOverview()->height() + overviewArea->getGraphOverview()->height();
    }
    return h;
}

void MSAOverviewExporter::initSettingsWidget() {
    exportSimpleOverview = new QRadioButton(tr("Export simple overview"));
    exportGraphOverview = new QRadioButton(tr("Export graph overview"));
    exportBothOverviews = new QRadioButton(tr("Export both overview"));

    QButtonGroup* group = new QButtonGroup();
    group->addButton(exportSimpleOverview);
    group->addButton(exportGraphOverview);
    group->addButton(exportBothOverviews);
    group->setExclusive(true);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(exportSimpleOverview);
    layout->addWidget(exportGraphOverview);
    layout->addWidget(exportBothOverviews);

    MSASimpleOverview* so = overviewArea->getSimpleOverview();
    if (!so->isValid()) {
        exportSimpleOverview->setDisabled(true);
        exportBothOverviews->setDisabled(true);
        exportGraphOverview->setChecked(true);
    } else {
        exportBothOverviews->setChecked(true);
    }

    settingsWidget = new QWidget();
    settingsWidget->setLayout(layout);
}

} // namespace
