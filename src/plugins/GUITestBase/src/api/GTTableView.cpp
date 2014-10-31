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

#include "api/GTWidget.h"

#include "GTTableView.h"

namespace U2{

#define GT_CLASS_NAME "GTSpinBox"
#define GT_METHOD_NAME "getCellPosition"
QPoint GTTableView::getCellPosition(U2OpStatus &os, QTableView *table, int column, int row){
    GT_CHECK_RESULT(table,"table view is NULL",QPoint());
    QPoint p(table->columnViewportPosition(column)+table->columnWidth(column)/2,
             table->rowViewportPosition(row)+table->rowHeight(row)*1.5);
    QPoint pGlob = table->mapToGlobal(p);
    return pGlob;
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTSpinBox"
#define GT_METHOD_NAME "getCellPoint"
QPoint GTTableView::getCellPoint(U2OpStatus &os, QTableView *table, int row, int column) {
    QModelIndex idx = table->model()->index(row, column);
    table->scrollTo(idx);
    QRect rect = table->visualRect(idx);
    QWidget *content = GTWidget::findWidget(os, "qt_scrollarea_viewport", table);
    return content->mapToGlobal(rect.center());
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTSpinBox"
#define GT_METHOD_NAME "rowCount"
int GTTableView::rowCount(U2OpStatus &os, QTableView *table) {
    return table->model()->rowCount(QModelIndex());
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTSpinBox"
#define GT_METHOD_NAME "rowCount"
QString GTTableView::data(U2OpStatus &os, QTableView *table, int column, int row) {
    QModelIndex idx = table->model()->index(row, column);
    return table->model()->data(idx, Qt::DisplayRole).toString();
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
