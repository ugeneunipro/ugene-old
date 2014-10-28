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

#include "UHMM3PhmmerDialogFiller.h"

#include "api/GTLineEdit.h"
#include "api/GTWidget.h"
#include "api/GTTabBar.h"

#include <QApplication>
#include <QTabWidget>

namespace U2 {

#define GT_CLASS_NAME "UHMM3PhmmerDialogFiller"
#define GT_METHOD_NAME "run"

void UHMM3PhmmerDialogFiller::run(){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    GTGlobals::sleep(1000);
    QTabWidget* tabWidget = GTWidget::findExactWidget<QTabWidget*>(os, "mainTabWidget", dialog);

    tabWidget->setCurrentIndex(0);
    QLineEdit* queryLineEdit = GTWidget::findExactWidget<QLineEdit*>(os, "queryLineEdit", dialog);
    GTLineEdit::setText(os, queryLineEdit, input);

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
