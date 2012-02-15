/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_HBAR_H_
#define _U2_HBAR_H_

#include <U2Core/global.h>
#include <QtGui/QToolBar>

namespace U2 {

///////////////////////////////////////////////////////////////////////////////////////////
// Header widget toolbar

class U2GUI_EXPORT HBar : public QToolBar {
public:
    HBar(QWidget* w) : QToolBar(w){}
protected:
    void paintEvent(QPaintEvent* ) {
        //do not draw any special toolbar control -> make is merged with parent widget
    }
};

} //namespace

#endif
