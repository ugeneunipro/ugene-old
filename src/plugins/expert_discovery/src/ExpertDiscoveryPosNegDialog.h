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

#ifndef _EXPERT_DISCOVERY_POS_NEG_DIALOG_H_
#define _EXPERT_DISCOVERY_POS_NEG_DIALOG_H_

#include <ui/ui_ExpertDiscoveryPosNegDialog.h>

namespace U2 {

class ExpertDiscoveryPosNegDialog : public QDialog, public Ui_ExpertDiscoveryPosNegDialog{
    Q_OBJECT
public:
    ExpertDiscoveryPosNegDialog(QWidget *parent);

    virtual void accept();

    QString getFirstFileName() const {return firstFileName;}
    QString getSecondFileName() const {return secondFileName;}
    bool isGenerateNegative() const {return generateNeg;}
    int getNegativePerPositive() const {return negativePerPositive;}
protected slots:
    void sl_openFirstFile();
    void sl_openSecondFile();
    void sl_oneSequence();

private:
    QString firstFileName;
    QString secondFileName;

    bool generateNeg;
    int negativePerPositive;

    QString filter;

};

} // namespace

#endif // _EXPERT_DISCOVERY_POS_NEG_DIALOG_H_

