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

#include "RangeSelectorFiller.h"
#include "api/GTLineEdit.h"
#include "api/GTWidget.h"

#include <QtGui/QApplication>
#include <QtGui/QPushButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ZoomToRangeDialogFiller"
#define GT_METHOD_NAME "run"
void ZoomToRangeDialogFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *from = dialog->findChild<QLineEdit*>("start_edit_line");
    QLineEdit *to = dialog->findChild<QLineEdit*>("end_edit_line");
    GT_CHECK(from != NULL, "\"start_edit_line\" to  not found");
    GT_CHECK(to != NULL, "\"end_edit_line\" to  not found");

    GTLineEdit::setText(os, from, QString::number(minVal));
    GTLineEdit::setText(os, to, QString::number(maxVal));

    QPushButton *okButton = dialog->findChild<QPushButton*>("ok_button");
    GT_CHECK(okButton != NULL, "Button \"Ok\" to  not found");

    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
