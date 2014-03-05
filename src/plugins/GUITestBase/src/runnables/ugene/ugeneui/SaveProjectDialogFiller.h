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

#ifndef SAVEPROJECTDIALOGFILLER_H
#define SAVEPROJECTDIALOGFILLER_H

#include "GTUtilsDialog.h"
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QDialogButtonBox>
#endif

namespace U2{
class SaveProjectDialogFiller: public Filler{
public:
    SaveProjectDialogFiller(U2OpStatus &os, QDialogButtonBox::StandardButton _b):
        Filler(os,"SaveProjectDialog"),b(_b){}
    virtual void run();
private:
    QDialogButtonBox::StandardButton b;
};
}
#endif // SAVEPROJECTDIALOGFILLER_H
