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

#ifndef _U2_GT_UTILS_MDI_H_
#define _U2_GT_UTILS_MDI_H_

#include "api/GTGlobals.h"
#include "api/GTMenuBar.h"
#include "api/GTWidget.h"
#include <QPoint>

namespace U2 {
class GTUtilsMdi {
public:
    static void click(U2OpStatus &os, GTGlobals::WindowAction action);
	static QPoint getMdiItemPosition(U2OpStatus &os, const QString& windowName);
	static void selectRandomRegion(U2OpStatus &os, const QString& windowName);

    // fails if MainWindow is NULL or because of FindOptions settings
    static QWidget* activeWindow(U2OpStatus &os, const GTWidget::FindOptions& = GTWidget::FindOptions());

    // finds a window with a given window title in MDIManager windows
    // fails if windowName is empty or because of FindOptions settings
    static QWidget* findWindow(U2OpStatus &os, const QString& windowName, const GTWidget::FindOptions& = GTWidget::FindOptions());
};

} // namespace

#endif
