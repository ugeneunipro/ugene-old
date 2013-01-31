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

#ifndef _U2_GSCROLL_BAR_H_
#define _U2_GSCROLL_BAR_H_

#include <U2Core/global.h>
#include <QtGui/QScrollBar>

namespace U2 {

class U2GUI_EXPORT GScrollBar : public QScrollBar {
public:
    GScrollBar(Qt::Orientation o, QWidget *p=0) : QScrollBar(o, p){}

    void setupRepeatAction(SliderAction action, int thresholdTime=100, int repeatTime=50) {
        QScrollBar::setRepeatAction(action, thresholdTime, repeatTime);
    }

    SliderAction getRepeatAction() const {return repeatAction();}
};


} //namespace

#endif
