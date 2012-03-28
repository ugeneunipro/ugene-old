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

#include "GTWidget.h"
#include "GTMouseDriver.h"
#include <QtGui/QWidget>
#include "GTGlobals.h"

namespace U2 {

void GTWidget::click(U2OpStatus &os, QWidget *w) {

    GTMouseDriver::moveTo(os, w->mapToGlobal(w->rect().center()));
    GTMouseDriver::click(os);
}

void GTWidget::setFocus(U2OpStatus &os, QWidget *w) {

    CHECK_SET_ERR(w != NULL, "GTWidget::setFocus: widget is NULL");

    GTWidget::click(os, w);
    GTGlobals::sleep(1000);

    CHECK_SET_ERR(w->hasFocus(), "Can't set focus on widget");
}

} //namespace
