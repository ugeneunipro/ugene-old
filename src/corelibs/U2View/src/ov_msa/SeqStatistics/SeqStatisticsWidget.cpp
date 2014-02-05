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

#include "SeqStatisticsWidget.h"

#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Core/AppContext.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2SafePoints.h>
#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include "U2View/MSAEditorDataList.h"

namespace U2 {

static inline QVBoxLayout * initLayout(QWidget * w) {
    QVBoxLayout * layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignTop);

    w->setLayout(layout);
    return layout;
}

SeqStatisticsWidget::SeqStatisticsWidget(MSAEditor* m):msa(m){
    SAFE_POINT(NULL != m, QString("Invalid parameter were passed into constructor SeqStatisticsWidget"), );

    copySettings();
   
    QVBoxLayout* mainLayout = initLayout(this);
    mainLayout->setSpacing(0);

    distancesStatisticsGroup = new QWidget(this);
    ui.setupUi(distancesStatisticsGroup);
    QWidget * similarityGroup = new ShowHideSubgroupWidget("REFERENCE", tr("Distances column"), distancesStatisticsGroup, true);
    updateWidgetsSettings();
    mainLayout->addWidget(similarityGroup);
}

void SeqStatisticsWidget::copySettings() {
    const MSAEditorAlignmentDependentWidget* similarityWidget= msa->getUI()->getSimilarityWidget();
    statisticsIsShown = false;
    if(NULL != similarityWidget) {
        const SimilarityStatisticsSettings* s = static_cast<const SimilarityStatisticsSettings*>(similarityWidget->getSettings());
        if(NULL != s) {
            settings = new SimilarityStatisticsSettings(*s);
        } else {
            settings = new SimilarityStatisticsSettings();
        }
        statisticsIsShown = !similarityWidget->isHidden();
    }
    else {
        settings = new SimilarityStatisticsSettings();
        settings->excludeGaps = false;
        settings->autoUpdate = true;
        settings->ma = msa->getMSAObject();
        settings->usePercents = true;
        settings->ui = msa->getUI();
    }
    msaUI = settings->ui;
}

void SeqStatisticsWidget::updateWidgetsSettings(){
    QStringList algoList = AppContext::getMSADistanceAlgorithmRegistry()->getAlgorithmIds();
    ui.algoComboBox->addItems(algoList);
    ui.algoComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

    ui.refSeqWarning->setText(tr("Hint: select a reference above"));
    ui.refSeqWarning->setStyleSheet(
        "color: green;"
        "font: bold;");
    ui.refSeqWarning->setWordWrap(true);

    ui.dataState->setText(tr("Press button to update"));

    restoreSettings();

    connectSlots();
}

void SeqStatisticsWidget::connectSlots() {
    connect(ui.algoComboBox,             SIGNAL(currentIndexChanged(const QString &)),  SLOT(sl_onAlgoChanged(const QString &)));
    connect(ui.excludeGapsCheckBox,      SIGNAL(stateChanged (int)),                    SLOT(sl_onGapsChanged(int)));
    connect(ui.countsButton,             SIGNAL(clicked(bool)),                         SLOT(sl_onUnitsChanged(bool)));
    connect(ui.percentsButton,           SIGNAL(clicked(bool)),                         SLOT(sl_onUnitsChanged(bool)));
    connect(ui.updateButton,             SIGNAL(pressed()),                             SLOT(sl_onUpdateClicked()));
    connect(ui.showDistancesColumnCheck, SIGNAL(stateChanged (int)),                    SLOT(sl_onShowStatisticsChanged(int)));
    connect(ui.autoUpdateCheck,          SIGNAL(stateChanged (int)),                    SLOT(sl_onAutoUpdateChanged(int)));
    connect(msa,                         SIGNAL(si_referenceSeqChanged(qint64)),        SLOT(sl_onRefSeqChanged(qint64)));
}

void SeqStatisticsWidget::restoreSettings() {
    ui.showDistancesColumnCheck->setCheckState(statisticsIsShown ? Qt::Checked : Qt::Unchecked);
    ui.percentsButton->setChecked(settings->usePercents);
    ui.countsButton->setChecked(!settings->usePercents);
    ui.excludeGapsCheckBox->setCheckState(settings->excludeGaps ? Qt::Checked : Qt::Unchecked);
    ui.autoUpdateCheck->setCheckState(settings->autoUpdate ? Qt::Checked : Qt::Unchecked);
    ui.updateButton->setEnabled(!settings->autoUpdate);
    ui.dataState->setEnabled(!settings->autoUpdate);

    int index = ui.algoComboBox->findText(settings->algoName);
    if(0 <= index) {
        ui.algoComboBox->setCurrentIndex(index);
    } else {
        settings->algoName = ui.algoComboBox->currentText();
    }
    if(!statisticsIsShown) {
        hideSimilaritySettings();
    } else {
        sl_onRefSeqChanged(msa->getReferenceRowId());
    }
}

void SeqStatisticsWidget::sl_onAlgoChanged(const QString &algoName) {
    settings->algoName = algoName;
    msaUI->setSimilaritySettings(settings);
}

void SeqStatisticsWidget::sl_onGapsChanged(int state) {
    settings->excludeGaps = (Qt::Checked == state);
    msaUI->setSimilaritySettings(settings);
}

void SeqStatisticsWidget::sl_onUnitsChanged( bool ) {
    settings->usePercents = ui.percentsButton->isChecked();
    msaUI->setSimilaritySettings(settings);
}

void SeqStatisticsWidget::sl_onAutoUpdateChanged(int state) {
    bool autoUpdateEnabled = Qt::Checked == state;
    settings->autoUpdate = autoUpdateEnabled;
    ui.updateButton->setEnabled(!autoUpdateEnabled);
    ui.dataState->setEnabled(!autoUpdateEnabled);
    msaUI->setSimilaritySettings(settings);
}

void SeqStatisticsWidget::sl_onRefSeqChanged(qint64 referenceRowId) {
    if(MAlignmentRow::invalidRowId() == referenceRowId) {
        ui.refSeqWarning->show();
    } else {
        ui.refSeqWarning->hide();
    }
}

void SeqStatisticsWidget::sl_onShowStatisticsChanged(int state) {
    if(Qt::Checked == state) {
        showSimilaritySettings();
    } else {
        hideSimilaritySettings();
    }
}

void SeqStatisticsWidget::sl_onUpdateClicked() {
    msaUI->refreshSimilarityColumn();
}

void SeqStatisticsWidget::hideSimilaritySettings() {
    statisticsIsShown = false;
    ui.optionsWidget->setEnabled(false);
    ui.refSeqWarning->hide();
    msaUI->hideSimilarity();
}

void SeqStatisticsWidget::showSimilaritySettings() {
    statisticsIsShown = true;
    ui.optionsWidget->setEnabled(true);
    ui.refSeqWarning->show();
    msaUI->showSimilarity();
    msaUI->setSimilaritySettings(settings);
    sl_onRefSeqChanged(msa->getReferenceRowId());
}

}
