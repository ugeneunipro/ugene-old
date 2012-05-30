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

#include "SmithWatermanDialogBaseFiller.h"
#include "api/GTWidget.h"
#include "api/GTTabWidget.h"
#include "api/GTTextEdit.h"

#include <QtGui/QApplication>
#include <QtGui/QAbstractButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::SmithWatermanDialogFiller"
#define GT_METHOD_NAME "run"
void SmithWatermanDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (button == Cancel) {
        QAbstractButton *cancelButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "bttnCancel", dialog));
        GTWidget::click(os, cancelButton);
        return;
    }

    QTabWidget *tabWidget = qobject_cast<QTabWidget*>(GTWidget::findWidget(os, "tabWidget", dialog));
    GTTabWidget::setCurrentIndex(os, tabWidget, 0);

    RegionSelector *regionSelector = qobject_cast<RegionSelector*>(GTWidget::findWidget(os, "range_selector", dialog));
    GTRegionSelector::setRegion(os, regionSelector, s);

    QTextEdit* textEdit = qobject_cast<QTextEdit*>(GTWidget::findWidget(os, "teditPattern", dialog));
    GTTextEdit::setText(os, textEdit, pattern);

    QAbstractButton *searchButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "bttnRun", dialog));
    GTWidget::click(os, searchButton);
    GTGlobals::sleep();
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
