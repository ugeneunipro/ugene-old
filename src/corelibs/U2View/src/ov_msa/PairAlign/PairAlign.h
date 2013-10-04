/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PAIR_ALIGN_H
#define _U2_PAIR_ALIGN_H

#include "ui/ui_PairwiseAlignmentOptionsPanelWidget.h"
#include "../SequenceSelectorWidgetController.h"

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtGui/QWidget>
#include <QtGui/QToolButton>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <U2View/MSAEditor.h>

#include <U2Gui/SuggestCompleter.h>

namespace U2 {

class PairwiseAlignmentMainWidget;
class PairwiseAlignmentTaskSettings;
class MAlignment;
class MAlignmentModInfo;
class ShowHideSubgroupWidget;
class MSADistanceAlgorithm;

class U2VIEW_EXPORT PairAlign : public QWidget, public Ui_PairwiseAlignmentOptionsPanelWidget {
    Q_OBJECT
    Q_DISABLE_COPY(PairAlign)

public:
    PairAlign(MSAEditor* _msa);
    ~PairAlign();

private:
    void initLayout();
    void initParameters();
    void connectSignals();
    void checkState();
    void updatePercentOfSimilarity();
    bool checkSequenceNames();

private slots:
    void sl_algorithmSelected(const QString& algorithmName);
    void sl_subwidgetStateChanged(const QString& id);
    void sl_inNewWindowCheckBoxChangeState(bool newState);
    void sl_selectFileButtonClicked();
    void sl_alignButtonPressed();
    void sl_outputFileChanged(const QString& newText);

    void sl_distanceCalculated();
    void sl_alignComplete();
    void sl_selectorTextChanged();

private:
    MSAEditor* msa;
    PairwiseAlignmentWidgetsSettings * pairwiseAlignmentWidgetsSettings;
    MSADistanceAlgorithm* distanceCalcTask;

    PairwiseAlignmentMainWidget* settingsWidget;       //created by factories on demand

    ShowHideSubgroupWidget* showHideSequenceWidget;
    ShowHideSubgroupWidget* showHideSettingsWidget;
    ShowHideSubgroupWidget* showHideOutputWidget;

    SequenceSelectorWidgetController *firstSeqSelectorWC;
    SequenceSelectorWidgetController *secondSeqSelectorWC;

    bool showSequenceWidget;
    bool showAlgorithmWidget;
    bool showOutputWidget;

    bool firstSequenceSelectionOn;
    bool secondSequenceSelectionOn;

    bool sequencesChanged;
    bool sequenceNamesIsOk;
    bool alphabetIsOk;      //no RNA alphabet allowed
    bool canDoAlign;
};

} // namespace

#endif // PAIR_ALIGN_H
