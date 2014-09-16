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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#else
#include <QtWidgets/QApplication>
#endif

#include <U2Gui/ImportOptionsWidget.h>

#include "ImportOptionsWidgetFiller.h"
#include "ItemToImportEditDialogFiller.h"
#include "api/GTWidget.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ItemToImportEditDialogFiller"

ItemToImportEditDialogFiller::ItemToImportEditDialogFiller(U2OpStatus& os, const QVariantMap& data) :
    Filler(os, "ItemToImportEditDialog"),
    data(data)
{
}

#define GT_METHOD_NAME "run"
void ItemToImportEditDialogFiller::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(NULL != dialog, "activeModalWidget is NULL");

    ImportOptionsWidget* optionsWidget = qobject_cast<ImportOptionsWidget*>(GTWidget::findWidget(os, "optionsWidget", dialog));
    GT_CHECK(NULL != optionsWidget, "optionsWidget is NULL");

    ImportOptionsWidgetFiller::fill(os, optionsWidget, data);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
