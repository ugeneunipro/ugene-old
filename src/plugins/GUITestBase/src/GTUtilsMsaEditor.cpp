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

#include <U2View/MSAEditorOverviewArea.h>
#include <U2View/MSAGraphOverview.h>

#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditor.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsMsaEditor"

#define GT_METHOD_NAME "getGraphOverviewTopLeftPixelColor"
QColor GTUtilsMsaEditor::getGraphOverviewPixelColor(U2OpStatus &os, const QPoint &point) {
    return GTWidget::getColor(os, getGraphOverview(os), point);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getGraphOverview"
MSAGraphOverview *GTUtilsMsaEditor::getGraphOverview(U2OpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);

    MSAGraphOverview *result = GTWidget::findExactWidget<MSAGraphOverview *>(os, MSAEditorOverviewArea::OVERVIEW_AREA_OBJECT_NAME + QString("_graph"), activeWindow);
    GT_CHECK_RESULT(NULL != result, "MSAGraphOverview is not found", NULL);
    return result;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
