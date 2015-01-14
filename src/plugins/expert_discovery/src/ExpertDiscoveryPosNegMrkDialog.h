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

#ifndef _EXPERT_DISCOVERY_POS_NEG_MRK_DIALOG_H_
#define _EXPERT_DISCOVERY_POS_NEG_MRK_DIALOG_H_

#include <ui/ui_ExpertDiscoveryPosNegMrkDialog.h>

namespace U2 {

class ExpertDiscoveryPosNegMrkDialog : public QDialog, public Ui_ExpertDiscoveryPosNegMrkDialog{
    Q_OBJECT
public:
    ExpertDiscoveryPosNegMrkDialog(QWidget *parent);

    virtual void accept();

    QString getFirstFileName() const {return firstFileName;}
    QString getSecondFileName() const {return secondFileName;}
    QString getThirdFileName() const {return thirdFileName;}
    bool isGenerateDescr() const {return generateDescr;}
    bool isAppendToCurrentMarkup() const {return addToCurrentCheck->isChecked();}
    bool isNucleotidesMarkup() const {return lettersCheck->isChecked();}
protected slots:
    void sl_openFirstFile();
    void sl_openSecondFile();
    void sl_openThirdFile();
    void sl_oneSequence();
    void sl_lettersMarkup();

private:
    QString firstFileName;
    QString secondFileName;
    QString thirdFileName;

    bool generateDescr;

    QString filter;

};

} // namespace

#endif // _EXPERT_DISCOVERY_POS_NEG_MRK_DIALOG_H_

