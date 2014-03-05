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

#ifndef _U2_GT_RADIOBUTTON_H_
#define _U2_GT_RADIOBUTTON_H_

#include "api/GTGlobals.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QRadioButton>
#else
#include <QtWidgets/QRadioButton>
#endif

namespace U2 {

class GTRadioButton {
public:
    // fails if the radioButton is NULL or can't click
    static void click(U2OpStatus& os, QRadioButton *radioButton);
    static QRadioButton* getRadioButtonByText(U2OpStatus& os, QString text, QWidget* parent=NULL);
};

}
#endif // _U2_GT_RADIOBUTTON_H_
