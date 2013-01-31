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

#ifndef _U2_CONSENSUS_SELECTOR_DIALOG_CONTROLLER_H_
#define _U2_CONSENSUS_SELECTOR_DIALOG_CONTROLLER_H_

#include "ui/ui_ConsensusSelectorDialog.h"

#include <U2Core/global.h>
#include <U2Algorithm/MSAConsensusAlgorithm.h>

namespace U2{

class U2VIEW_EXPORT ConsensusSelectorDialogController : public QDialog, Ui_ConsensusSelectorDialog { 
    Q_OBJECT
public:
    ConsensusSelectorDialogController(const QString& selectedAlgorithmId, ConsensusAlgorithmFlags flags, QWidget *p = NULL);

    const QString& getSelectedAlgorithmId() const {return selectedAlgorithmId;}

    // enables threshold selector group, sets threshold range to 'r' 
    // and current value for slider and spin box to 'value'
    void enableThresholdSelector(int minVal, int maxVal, int value, const QString& suffix);
    
    // disables threshold selector group
    void disableThresholdSelector();
    
    int getThresholdValue() const;

signals:
    void si_algorithmChanged(const QString& algoId);
    
    void si_thresholdChanged(int val);

private slots:
    void sl_algorithmSelectionChanged(int);
    void sl_thresholdSliderChanged(int);
    void sl_thresholdSpinChanged(int);
    void sl_resetThresholdClicked(bool);

private:
    void updateSelectedAlgorithmDesc();

    QString selectedAlgorithmId;
};

}

#endif
