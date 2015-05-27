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

#include <ui/ui_ExpertDiscoveryAdvSetDialog.h>

#include <QDoubleValidator>
#include <QLineEdit>

namespace U2 {

class ExpertDiscoveryAdvSetDialog : public QDialog, public Ui_ExpertDiscoveryAdvSetDialog{
    Q_OBJECT
public:
    ExpertDiscoveryAdvSetDialog(QWidget *parent,
        double& dIntProbability,
        double& dIntFisher,
        int&    nMinComplexity,
        int&    nMaxComplexity,
        double& dMinPosCorrelation,
        double& dMaxPosCorrelation,
        double& dMinNegCorrelation,
        double& dMaxNegCorrelation,
        bool&   bCorrelationImportant);

    virtual void accept();

private:
    double& dIntProbability;
    double& dIntFisher;
    int&    nMinComplexity;
    int&    nMaxComplexity;
    double& dMinPosCorrelation;
    double& dMaxPosCorrelation;
    double& dMinNegCorrelation;
    double& dMaxNegCorrelation;
    bool&   bCorrelationImportant;

    bool check(const QLineEdit* lineE);

    void hideParameters();

protected slots:
    void sl_checkCorrel();
};

}//namespace
