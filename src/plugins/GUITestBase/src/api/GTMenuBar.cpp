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

#include "GTMenuBar.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include <QtGui/QMenuBar>

namespace U2 {

void GTMenuBar::clickCornerMenu(U2OpStatus &os, QMenuBar* mBar, GTMenuBar::Action action) {

    QWidget* cWidget = mBar->cornerWidget();
    CHECK_SET_ERR(cWidget != NULL, "Corner widget is NULL");
    QRect r = cWidget->rect();

    int num = (int)action;
    CHECK_SET_ERR(num>=0 && num<(int)Count, "");

    CHECK_SET_ERR((int)Count != 0, "Count == 0");
    int oneWidth = r.width()/(int)Count;

    QPoint need((num+1)*oneWidth - oneWidth/2, r.height()/2);
    QPoint p = cWidget->mapToGlobal(need);

    GTMouseDriver::moveTo(os, p);
    GTGlobals::sleep(200);
    GTMouseDriver::click(os);
    GTGlobals::sleep(500);
}

}
