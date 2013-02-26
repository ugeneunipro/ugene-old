/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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
#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include "U2View/MSAEditorDataList.h"

namespace U2 {

static const int ITEMS_SPACING = 10;
static const int TITLE_SPACING = 5;

static inline QVBoxLayout * initLayout(QWidget * w) {
    QVBoxLayout * layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignTop);

    w->setLayout(layout);
    return layout;
}

SeqStatisticsWidget::SeqStatisticsWidget(MSAEditor* m):msa(m){
    connect(msa, SIGNAL(si_refrenceSeqChanged(const QString &)), SLOT(sl_refSetChangedOutside(const QString &)));
    Q_ASSERT(m != NULL);
    const MSAEditorAlignmentDependentWidget* similarityWidget= m->getUI()->getSimilarityWidget();
    statisticsIsShown = false;
    if(NULL != similarityWidget) {
        const SimilarityStatisticsSettings* s = static_cast<const SimilarityStatisticsSettings*>(similarityWidget->getSettings());
        if(NULL != s) {
            settings = new SimilarityStatisticsSettings(*s);
        }
        else {
            settings = new SimilarityStatisticsSettings();
        }
        statisticsIsShown = !similarityWidget->isHidden();
    }
    else {
        settings = new SimilarityStatisticsSettings();
        settings->excludeGaps = false;
        settings->autoUpdate = true;
        settings->ma = m->getMSAObject();
        settings->usePercents = true;
        settings->ui = m->getUI();
    }
    ui = settings->ui;

    QVBoxLayout* mainLayout = initLayout(this);
    mainLayout->setSpacing(0);

    QWidget * similarityGroup = new ShowHideSubgroupWidget("REFRENCE", tr("Sequences similarity"), createSimilaritySettings(), true);
    mainLayout->addWidget(similarityGroup);
}

QWidget* SeqStatisticsWidget::createSimilaritySettings(){
    QWidget * group = new QWidget(this);
    QVBoxLayout * layout = initLayout(group);

    showStatisticsCheck = new QCheckBox(tr("Show similarity statistics"), this);
    layout->addSpacing(TITLE_SPACING);
    layout->addWidget(showStatisticsCheck);
    layout->addSpacing(TITLE_SPACING);
    algoLabel = new QLabel(tr("Similarity algorithm:"));
    layout->addWidget(algoLabel);
    algoCombo = new QComboBox(group);
    algoCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    QStringList algoList = AppContext::getMSADistanceAlgorithmRegistry()->getAlgorithmIds();
    algoCombo->addItems(algoList);
    layout->addWidget(algoCombo);

    autoUpdateCheck = new QCheckBox(tr("Automatic updating"), this);
    layout->addWidget(autoUpdateCheck);
    
    profileGroup = new QGroupBox(tr("Profile mode"));
    QVBoxLayout * profileLayout = new QVBoxLayout();
    countsCheck = new QRadioButton(tr("Counts"), this);
    profileLayout->addWidget(countsCheck);
    percentsCheck = new QRadioButton(tr("Percents"), this);
    profileLayout->addWidget(percentsCheck);
    excludeGaps = new QCheckBox(tr("Exclude gaps"), this);
    profileLayout->addWidget(excludeGaps);
    profileGroup->setLayout(profileLayout);
    layout->addSpacing(ITEMS_SPACING);
    layout->addWidget(profileGroup);
    refSeqWarning = new QLabel(QString("<FONT COLOR=#FF0000>%1</FONT>").arg(tr("Please, select reference sequence")));
    layout->addWidget(refSeqWarning);
    sl_onRefSeqChanged(msa->getRefSeqName());

    restoreSettings();

    connect(algoCombo,            SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_onAlgoChanged(const QString &)));
    connect(excludeGaps,          SIGNAL(stateChanged (int)),       SLOT(sl_onGapsChanged(int)));
    connect(countsCheck,          SIGNAL(clicked(bool)),            SLOT(sl_onUnitsChanged(bool)));
    connect(percentsCheck,        SIGNAL(clicked(bool)),            SLOT(sl_onUnitsChanged(bool)));
    connect(showStatisticsCheck,  SIGNAL(stateChanged (int)),       SLOT(sl_onShowStatisticsChanged(int)));
    connect(autoUpdateCheck,      SIGNAL(stateChanged (int)),       SLOT(sl_onAutoUpdateChanged(int)));
    bool res = connect(msa,                  SIGNAL(si_refrenceSeqChanged(const QString&)), SLOT(sl_onRefSeqChanged(const QString&)));

    return group;
}

void SeqStatisticsWidget::restoreSettings() {
    showStatisticsCheck->setCheckState(statisticsIsShown ? Qt::Checked : Qt::Unchecked);
    percentsCheck->setChecked(settings->usePercents);
    countsCheck->setChecked(!settings->usePercents);
    excludeGaps->setCheckState(settings->excludeGaps ? Qt::Checked : Qt::Unchecked);
    autoUpdateCheck->setCheckState(settings->autoUpdate ? Qt::Checked : Qt::Unchecked);
    int index = algoCombo->findText(settings->algoName);
    if(0 <= index) {
        algoCombo->setCurrentIndex(index);
    }
    else {
        settings->algoName = algoCombo->currentText();
    }
    if(!statisticsIsShown) {
        hideSimilaritySettings();
    }
}

void SeqStatisticsWidget::sl_onAlgoChanged(const QString &algoName) {
    settings->algoName   = algoName;
    ui->setSimilaritySettings(settings);
}
void SeqStatisticsWidget::sl_onGapsChanged(int state) {
    settings->excludeGaps = (Qt::Checked == state);
    ui->setSimilaritySettings(settings);
}
void SeqStatisticsWidget::sl_onUnitsChanged( bool ) {
    settings->usePercents = percentsCheck->isChecked();
    ui->setSimilaritySettings(settings);
}
void SeqStatisticsWidget::sl_onAutoUpdateChanged(int state) {
    settings->autoUpdate = (Qt::Checked == state);
    ui->setSimilaritySettings(settings);
}
void SeqStatisticsWidget::sl_onRefSeqChanged(const QString& str) {
    if(str == "(None)") {
        refSeqWarning->show();
    }
    else {
        refSeqWarning->hide();
    }
}

void SeqStatisticsWidget::hideSimilaritySettings()
{
    algoLabel->setEnabled(false);
    algoCombo->setEnabled(false);
    autoUpdateCheck->setEnabled(false);
    profileGroup->setEnabled(false);
    refSeqWarning->hide();
}

void SeqStatisticsWidget::showSimilaritySettings()
{
    algoLabel->setEnabled(true);
    algoCombo->setEnabled(true);
    autoUpdateCheck->setEnabled(true);
    profileGroup->setEnabled(true);
    refSeqWarning->show();
}

void SeqStatisticsWidget::sl_onShowStatisticsChanged(int state)
{
    if(Qt::Checked == state) {
        statisticsIsShown = true;
        showSimilaritySettings();
        ui->showSimilarity();
    }
    else {
        statisticsIsShown = false;
        hideSimilaritySettings();
        ui->hideSimilarity();
        return;
    }

}


}