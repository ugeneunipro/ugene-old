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

#include <QApplication>
#include <QTreeWidget>

#include "drivers/GTKeyboardDriver.h"
#include "api/GTTreeWidget.h"
#include "api/GTWidget.h"

#include "DashboardsManagerDialogFiller.h"

namespace U2 {
using namespace HI;

#define GT_CLASS_NAME "DashboardsManagerDialogFiller"
#define GT_METHOD_NAME "commonScenario"
void DashboardsManagerDialogFiller::commonScenario(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectDashboards"
void DashboardsManagerDialogFiller::selectDashboards(U2OpStatus &os, QStringList names){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QTreeWidget* listWidget = GTWidget::findExactWidget<QTreeWidget*>(os, "listWidget", dialog);
    foreach (QString name, names) {
        QTreeWidgetItem* item = GTTreeWidget::findItem(os, listWidget, name);
        GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["ctrl"]);
        GTTreeWidget::click(os, item);
        GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isDashboardPresent"
bool DashboardsManagerDialogFiller::isDashboardPresent(U2OpStatus &os, QString name){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK_RESULT(dialog, "activeModalWidget is NULL", false);

    QTreeWidget* listWidget = GTWidget::findExactWidget<QTreeWidget*>(os, "listWidget", dialog);
    QTreeWidgetItem* item = GTTreeWidget::findItem(os, listWidget, name, NULL, 0, GTGlobals::FindOptions(false));
    return item != NULL;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
