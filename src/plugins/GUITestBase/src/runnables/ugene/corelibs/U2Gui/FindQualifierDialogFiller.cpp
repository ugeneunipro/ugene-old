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

#include "FindQualifierDialogFiller.h"

#include "api/GTLineEdit.h"
#include "api/GTWidget.h"
#include "api/GTRadioButton.h"

#include "runnables/qt/MessageBoxFiller.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::FindQualifierFiller"
#define GT_METHOD_NAME "run"
void FindQualifierFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit* nameEdit = dialog->findChild<QLineEdit*>("nameEdit");
    GT_CHECK(nameEdit != NULL, "nameEdit not found");
    GTLineEdit::setText(os, nameEdit, settings.name);

    QLineEdit* valueEdit = dialog->findChild<QLineEdit*>("valueEdit");
    GT_CHECK(valueEdit != NULL, "valueEdit not found");
    GTLineEdit::setText(os, valueEdit, settings.value);

    if (settings.exactMatch) {
        QRadioButton* exactButton = dialog->findChild<QRadioButton*>("exactButton");
        GT_CHECK(exactButton != NULL, "exactButton not found");
        GTRadioButton::click(os, exactButton);
    } else {
        QRadioButton* containsButton = dialog->findChild<QRadioButton*>("containsButton");
        GT_CHECK(containsButton != NULL, "exactButton not found");
        GTRadioButton::click(os, containsButton);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");

    if (settings.expectTheEndOfTree) {
        GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    }

    if (settings.nextQualifier && settings.nextCount > 0) {
        QPushButton* nextButton = box->button(QDialogButtonBox::Ok);
        GT_CHECK(nextButton != NULL, "\"Next\" button is NULL");
        for (int i = 0; i < settings.nextCount; i++) {
            GTWidget::click(os, nextButton);
        }
    }

    if (settings.selectAll) {
        QPushButton* selectAllButton = box->button(QDialogButtonBox::Yes);
        GT_CHECK(selectAllButton != NULL, "\"Select all\" button is NULL");
        GTWidget::click(os, selectAllButton);
    }
    GTGlobals::sleep();

    QPushButton* cancelButton = box->button(QDialogButtonBox::Cancel);
    GT_CHECK(cancelButton != NULL, "\"Cancel\" button is NULL");
    GTWidget::click(os, cancelButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
