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

#include "api/GTLineEdit.h"
#include "api/GTWidget.h"

#include "UHMM3SearchDialogFiller.h"
namespace U2 {

#define GT_CLASS_NAME "UHMM3SearchDialogFiller"
#define GT_METHOD_NAME "run"

void UHMM3SearchDialogFiller::commonScenario(){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit* queryHmmFileEdit = GTWidget::findExactWidget<QLineEdit*>(os, "queryHmmFileEdit", dialog);
    GTLineEdit::setText(os, queryHmmFileEdit, profile);

    QLineEdit* newFilePathle = GTWidget::findExactWidget<QLineEdit*>(os, "newFilePath", dialog);
    GTLineEdit::setText(os, newFilePathle, newFilePath);

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
