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
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTextEdit>

#include "CreateElementWithScriptDialogFiller.h"
#include "api/GTLineEdit.h"
#include "api/GTTextEdit.h"
#include "primitives/GTWidget.h"
#include "base_dialogs/MessageBoxFiller.h"

namespace U2{

#define GT_CLASS_NAME "CreateElementWithScriptDialogFiller"
#define GT_METHOD_NAME "run"
void CreateElementWithScriptDialogFiller::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit* nameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameEdit", dialog));
    GT_CHECK(nameEdit, "nameEdit not found");
    GTLineEdit::setText(os, nameEdit, name);

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "ScriptEditorDialogFiller"
#define GT_METHOD_NAME "run"
void ScriptEditorDialogFiller::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit* scriptPathEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "scriptPathEdit", dialog));
    GT_CHECK(scriptPathEdit, "scriptPath lineEdit not found");
    GTLineEdit::setText(os, scriptPathEdit, url);

    QTextEdit* edit = NULL;

    foreach(QTextEdit* textEdit, dialog->findChildren<QTextEdit*>()){
        if (!textEdit->isReadOnly())
            edit = textEdit;
    }
    GT_CHECK(edit, "textEdit not found");
    GTTextEdit::setText(os, edit, text);

    GTGlobals::sleep(3000);
    if(checkSyntax){
        GTUtilsDialog::waitForDialog(os,new MessageBoxDialogFiller(os, QMessageBox::Ok, checkSyntaxResult));
        GTWidget::click(os, GTWidget::findWidget(os, "checkButton", dialog));
        GTGlobals::sleep(1000);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "ScriptEditorDialogFiller"
#define GT_METHOD_NAME "run"
void ScriptEditorDialogSyntaxChecker::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QTextEdit* edit = NULL;

    foreach(QTextEdit* textEdit, dialog->findChildren<QTextEdit*>()){
        if (!textEdit->isReadOnly())
            edit = textEdit;
    }
    GT_CHECK(edit, "textEdit not found");
    GTTextEdit::setText(os, edit, text);

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os,QMessageBox::Ok, message));
    GTWidget::click(os, GTWidget::findWidget(os, "checkButton", dialog));

    GTGlobals::sleep(1000);

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
