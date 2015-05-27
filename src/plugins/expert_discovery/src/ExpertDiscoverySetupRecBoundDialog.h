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

#pragma once

#include <ui/ui_ExpertDiscoverySetupRecBoundDialog.h>

#include <vector>

#include "ExpertDiscoveryGraphs.h"

namespace U2 {



class ExpertDiscoverySetupRecBoundDialog : public QDialog, public Ui_EDSetupRecBoundDlg{
    Q_OBJECT
public:
    ExpertDiscoverySetupRecBoundDialog(double dRecognizationBound,
        const std::vector<double>& vPosScore,
        const std::vector<double>& vNegScore);

    virtual void accept();

    double getRecognizationBound() const { return recognizationBound; }

private:
    double recognizationBound;
    double probNegRec;
    double probPosRej;
    const std::vector<double>& posScore;
    const std::vector<double>& negScore;

    void updateProbs();

    ExpertDiscoveryRecognitionErrorGraphWidget*     graphWidget;


protected slots:
    void sl_recBoundChaged(double val);
    void sl_optRecBound();

    void sl_intervalChanged(int val);
    void sl_recalculateValues();

    void sl_showWarning(bool isShown);
};

}//namespace
