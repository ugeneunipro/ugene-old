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

#ifndef _U2_GT_RUNNABLES_WIZARD_FILLER_H_
#define _U2_GT_RUNNABLES_WIZARD_FILLER_H_

#include "GTUtilsDialog.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#else
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#endif

namespace U2 {

class WizardFiller : public Filler {
public:
    WizardFiller(U2OpStatus &_os, QString name, QList<QStringList> _inputFiles = QList<QStringList>(), QMap<QString, QVariant> _map = (QMap<QString, QVariant>())):
        Filler(_os, name), inputFiles(_inputFiles), map(_map){}
    WizardFiller(U2OpStatus &_os, QString name, QStringList _inputFiles, QMap<QString, QVariant> _map = (QMap<QString, QVariant>())):
        Filler(_os, name), inputFiles(QList<QStringList>()<<_inputFiles), map(_map){}
    WizardFiller(U2OpStatus &_os, QString name, CustomScenario* c): Filler(_os, name, c){}
    void commonScenario();

    static QToolButton* getExpandButton(U2OpStatus &_os);


    static void setInputFiles();

private:
    QList<QStringList> inputFiles;
    QMap<QString, QVariant> map;

};


}

#endif
