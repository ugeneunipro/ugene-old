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

#ifndef _U2_GT_RUNNABLES_WIZARD_FILLER_H_
#define _U2_GT_RUNNABLES_WIZARD_FILLER_H_

#include "GTUtilsDialog.h"
#include <QPushButton>
#include <QToolButton>

namespace U2 {

class WizardFiller : public Filler {
public:
    struct value{
        value(int _page):page(_page),isSet(false){}
        virtual void func(){}
        int page;
        bool isSet;
    };

    struct pairValLabel{
        pairValLabel(QString _label, value* _val):label(_label),val(_val){}
        QString label;
        value* val;
    };
    typedef QList<pairValLabel> pairValList;

    struct lineEditValue: public value{
        lineEditValue(QString _lineValue, bool _isPath, int _page=-1):value(_page),lineValue(_lineValue),isPath(_isPath){}
        QString lineValue;
        bool isPath;

    };

    struct spinBoxValue: public value{
        spinBoxValue(int _spinValue, int _page=-1):value(_page),spinValue(_spinValue){}
        int spinValue;
    };

    struct doubleSpinBoxValue: public value{
        doubleSpinBoxValue(double _spinValue, int _page=-1):value(_page),spinValue(_spinValue){}
        double spinValue;
    };

    struct ComboBoxValue: public value{
        ComboBoxValue(int _comboValue, int _page=-1):value(_page),comboValue(_comboValue){}
        int comboValue;
    };

    WizardFiller(U2OpStatus &_os, QString name):Filler(_os, name){}

protected:
    void setParameters(U2OpStatus &_os,pairValList list, int i);
    void fill(U2OpStatus &_os,pairValList list);
    QPushButton* getButtonByText(U2OpStatus &_os, QString text);
    QPushButton* getNextButton(U2OpStatus &_os);
    QPushButton* getCancelButton(U2OpStatus &_os);
    QPushButton* getSetupButton(U2OpStatus &_os);
    QPushButton* getFinishButton(U2OpStatus &_os);
    QToolButton* getExpandButton(U2OpStatus &_os);
    QString actualizePath(lineEditValue *);
};

class ConfigureTuxedoWizardFiller : public WizardFiller {
public:
    enum analysis_type {full,single_sample,no_new_transkripts};
    enum reads_type {singleReads,pairedReads};
    ConfigureTuxedoWizardFiller(U2OpStatus &_os,analysis_type _analys, reads_type _reads)
        : WizardFiller(_os, "Configure Tuxedo Pipeline"),analys(_analys),reads(_reads){}
    virtual void run();
private:
    analysis_type analys;
    reads_type reads;
};

class TuxedoWizardFiller : public WizardFiller {
public:
    TuxedoWizardFiller(U2OpStatus &_os, QString _path1, QString _name1,QString _path2, QString _name2, pairValList _list):
        WizardFiller(_os, "Tuxedo Wizard"),
        list(_list),
        path1(_path1),
        name1(_name1),
        path2(_path2),
        name2(_name2){}
    virtual void run();
private:
    pairValList list;
    QString path1,name1,path2,name2;
};
}

#endif
