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

#include "CreateElementWithScriptDialogFiller.h"
#include "api/GTLineEdit.h"
#include "api/GTWidget.h"
#include "runnables/qt/MessageBoxFiller.h"

#include <QtGui/QApplication>
#include <QtGui/QTextEdit>



namespace U2{

#define GT_CLASS_NAME "CreateElementWithScriptDialogFiller"
#define GT_METHOD_NAME "run"
void CreateElementWithScriptDialogFiller::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit* nameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameEdit", dialog));
    GT_CHECK(nameEdit, "nameEdit not found");
    GTLineEdit::setText(os, nameEdit, name);

    GTWidget::click(os, GTWidget::findWidget(os, "okButton", dialog));
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
    GT_CHECK(edit, "textEdit not found")
    edit->setText(text);

    GTGlobals::sleep(1000);
    if(checkSyntax){
        GTUtilsDialog::waitForDialog(os,new MessageBoxDialogFiller(os, QMessageBox::Ok, checkSyntaxResult));
        GTWidget::click(os, GTWidget::findWidget(os, "checkButton", dialog));
        GTGlobals::sleep(1000);
    }
    GTWidget::click(os, GTWidget::findWidget(os, "okButton", dialog));
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
    GT_CHECK(edit, "textEdit not found")
    edit->setText(text);

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os,QMessageBox::Ok, message));
    GTWidget::click(os, GTWidget::findWidget(os, "checkButton", dialog));

    GTGlobals::sleep(1000);
    GTWidget::click(os, GTWidget::findWidget(os, "okButton", dialog));
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
