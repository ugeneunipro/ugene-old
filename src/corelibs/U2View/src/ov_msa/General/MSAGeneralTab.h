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

#ifndef _U2_MSA_GENERAL_TAB_H_
#define _U2_MSA_GENERAL_TAB_H_

#include "ui_GeneralTabOptionsPanelWidget.h"

#include "../MsaOpSavableTab.h"

namespace U2 {

class MAlignment;
class MAlignmentModInfo;
class MSAEditor;

class MSAGeneralTab : public QWidget, public Ui_GeneralTabOptionsPanelWidget {
    Q_OBJECT
public:
    MSAGeneralTab(MSAEditor* _msa);

signals:
    void si_algorithmChanged(const QString& algoId);
    void si_thresholdChanged(int val);
    void si_copyFormatChanged(const QString& formatId);
    void si_copyFormatted();

public slots:
    void sl_alignmentChanged(const MAlignment& al, const MAlignmentModInfo& modInfo);
    void sl_algorithmChanged(const QString& algoId);
    void sl_thresholdChanged(int value);
    void sl_algorithmSelectionChanged(int index);
    void sl_thresholdSliderChanged(int value);
    void sl_thresholdSpinBoxChanged(int value);
    void sl_thresholdResetClicked(bool newState);
    void sl_copyFormatSelectionChanged(int value);
    void sl_copyFormatted();
    void sl_copyFormatStatusChanged(bool enabled);

private:
    void connectSignals();
    void initializeParameters();
    void updateState();
    void updateThresholdState(bool enable, int minVal = 0, int maxVal = 0, int value = 0);
    void initConsensusTypeCombo();

    MSAEditor* msa;
    QString curAlphabetId;
    MsaOpSavableTab savableTab;
};

} // namespace U2

#endif // _U2_MSA_GENERAL_TAB_H_
