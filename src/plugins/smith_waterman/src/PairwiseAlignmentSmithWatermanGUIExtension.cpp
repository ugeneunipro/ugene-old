/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "PairwiseAlignmentSmithWatermanGUIExtension.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2Algorithm/PairwiseAlignmentRegistry.h>
#include <U2Algorithm/PairwiseAlignmentTask.h>

#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QFormLayout>

namespace U2 {

PairwiseAlignmentSmithWatermanMainWidget::PairwiseAlignmentSmithWatermanMainWidget(QWidget* parent, QVariantMap* s, WidgetType widgetType) :
    PairwiseAlignmentMainWidget(parent, s, widgetType) {
    switch (widgetType) {
    //add subwidgets
    //fill custom settings
    case OptionsPanelWidgetType: {
        QVBoxLayout* optionsPanelLayout = new QVBoxLayout;
        optionsPanelLayout->setContentsMargins(0, 5, 0, 0);
        optionsPanelLayout->setSpacing(5);

        QLabel* algorithmVersionLabel = new QLabel(tr("Algorithm version:"));
        algorithmVersion = new QComboBox();

        QLabel* scoringMatrixLabel = new QLabel(tr("Scoring matrix:"));
        scoringMatrix = new QComboBox();

        QGroupBox* gapGroup = new QGroupBox(tr("Gap penalty"));
        QFormLayout* gapLayout = new QFormLayout;
        gapLayout->setContentsMargins(0, 5, 0, 0);
        gapLayout->setSpacing(0);
        QLabel* gapOpenLabel = new QLabel(tr("Open:"));
        QLabel* gapExtdLabel = new QLabel(tr("Extension:"));

        gapOpen = new QDoubleSpinBox;
        gapOpen->setMinimum(SW_MIN_GAP_OPEN);
        gapOpen->setMaximum(SW_MAX_GAP_OPEN);

        gapExtd = new QDoubleSpinBox;
        gapExtd->setMinimum(SW_MIN_GAP_EXTD);
        gapExtd->setMaximum(SW_MAX_GAP_EXTD);

        gapLayout->addRow(gapOpenLabel, gapOpen);
        gapLayout->addRow(gapExtdLabel, gapExtd);
        gapGroup->setLayout(gapLayout);


        optionsPanelLayout->addWidget(algorithmVersionLabel);
        optionsPanelLayout->addWidget(algorithmVersion);
        optionsPanelLayout->addWidget(scoringMatrixLabel);
        optionsPanelLayout->addWidget(scoringMatrix);
        optionsPanelLayout->addWidget(gapGroup);
        this->setLayout(optionsPanelLayout);


        DNAAlphabet* al = U2AlphabetUtils::getById(s->value(PA_ALPHABET, "").toString());
        SAFE_POINT(NULL != al, "Alphabet not found.", );
        SubstMatrixRegistry* matrixReg = AppContext::getSubstMatrixRegistry();
        SAFE_POINT(matrixReg, "SubstMatrixRegistry is NULL.", );
        QStringList matrixList = matrixReg->selectMatrixNamesByAlphabet(al);
        scoringMatrix->addItems(matrixList);
        if (s->contains(PA_SW_SCORING_MATRIX_NAME)) {
            scoringMatrix->setCurrentIndex(scoringMatrix->findText(s->value(PA_SW_SCORING_MATRIX_NAME, QString()).toString()));
        }

        QStringList alg_lst = AppContext::getPairwiseAlignmentRegistry()->getAlgorithm("Smith-Waterman")->getRealizationsList();
        algorithmVersion->addItems(alg_lst);
        if (s->contains(PA_SW_REALIZATION_NAME)) {
            algorithmVersion->setCurrentIndex(algorithmVersion->findText(s->value(PA_SW_REALIZATION_NAME, QString()).toString()));
        }

        if (s->contains(PA_SW_GAP_OPEN) && s->value(PA_SW_GAP_OPEN, 0).toInt() >= SW_MIN_GAP_OPEN && s->value(PA_SW_GAP_OPEN, 0).toInt() <= SW_MAX_GAP_OPEN) {
            gapOpen->setValue(-s->value(PA_SW_GAP_OPEN, 0).toInt());
        } else {
            gapOpen->setValue(SW_DEFAULT_GAP_OPEN);
        }

        if (s->contains(PA_SW_GAP_EXTD) && s->value(PA_SW_GAP_EXTD, 0).toInt() >= SW_MIN_GAP_EXTD && s->value(PA_SW_GAP_EXTD, 0).toInt() <= SW_MAX_GAP_EXTD) {
            gapExtd->setValue(-s->value(PA_SW_GAP_EXTD, 0).toInt());
        } else {
            gapExtd->setValue(SW_DEFAULT_GAP_EXTD);
        }


        innerSettings.insert(PA_SW_GAP_OPEN, -gapOpen->value());
        innerSettings.insert(PA_SW_GAP_EXTD, -gapExtd->value());
        innerSettings.insert(PA_REALIZATION_NAME, algorithmVersion->currentText());
        innerSettings.insert(PA_SW_REALIZATION_NAME, algorithmVersion->currentText());
        innerSettings.insert(PA_SW_SCORING_MATRIX_NAME, scoringMatrix->currentText());


        break;
    }
    case DialogWidgetType: {
        break;
    }
    default: {
        assert(0);
        break;
    }
    }   //switch (type)
}

PairwiseAlignmentSmithWatermanMainWidget::~PairwiseAlignmentSmithWatermanMainWidget() {
    getPairwiseAlignmentCustomSettings(true);
}

QMap<QString, QVariant> PairwiseAlignmentSmithWatermanMainWidget::getPairwiseAlignmentCustomSettings(bool append = false) {
    fillInnerSettings();
    return PairwiseAlignmentMainWidget::getPairwiseAlignmentCustomSettings(append);
}

void PairwiseAlignmentSmithWatermanMainWidget::fillInnerSettings() {
    innerSettings.insert(PA_SW_GAP_OPEN, -gapOpen->value());
    innerSettings.insert(PA_SW_GAP_EXTD, -gapExtd->value());
    innerSettings.insert(PA_REALIZATION_NAME, algorithmVersion->currentText());
    innerSettings.insert(PA_SW_REALIZATION_NAME, algorithmVersion->currentText());
    innerSettings.insert(PA_SW_SCORING_MATRIX_NAME, scoringMatrix->currentText());
}

PairwiseAlignmentSmithWatermanGUIExtensionFactory::PairwiseAlignmentSmithWatermanGUIExtensionFactory(SW_AlgType _algType) :
    PairwiseAlignmentGUIExtensionFactory(), algType(_algType) {
}

PairwiseAlignmentMainWidget* PairwiseAlignmentSmithWatermanGUIExtensionFactory::createMainWidget(QWidget* parent, QVariantMap* s, WidgetType widgetType) {
    if (mainWidgets.contains(parent)) {
        return mainWidgets.value(parent, NULL);
    }
    PairwiseAlignmentSmithWatermanMainWidget* newMainWidget = new PairwiseAlignmentSmithWatermanMainWidget(parent, s, widgetType);
    connect(newMainWidget, SIGNAL(destroyed(QObject*)), SLOT(sl_widgetDestroyed(QObject*)));
    mainWidgets.insert(parent, newMainWidget);
    return newMainWidget;
}

}   //namespace
