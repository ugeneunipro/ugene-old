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
#include "StartupDialogFiller.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#endif

#include "api/GTWidget.h"
#include "runnables/qt/MessageBoxFiller.h"

namespace U2{

#define GT_CLASS_NAME "GTUtilsDialog::StartupDialogFiller"
#define GT_METHOD_NAME "run"

StartupDialogFiller::StartupDialogFiller(U2OpStatus &os, QString _path, bool _isPathValid)
    : Filler(os,"StartupDialog"), path(_path), isPathValid(_isPathValid)
{

}

void StartupDialogFiller::run(){
    GTGlobals::sleep(1000);
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (path != GUITest::sandBoxDir) {
        QLineEdit *pathEdit = getPathEdit(dialog);
        CHECK(NULL != pathEdit, );
        QString rightPath = pathEdit->text();
        pathEdit->setText(path);

        if (!isPathValid) {
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
            use(dialog);
            CHECK_OP(os, );

            pathEdit->setText(rightPath);
        }
    }

    use(dialog);
}

void StartupDialogFiller::clickButton(QWidget *dialog, const QString &text) {
    QList<QPushButton*> list= dialog->findChildren<QPushButton*>();
    bool clicked = false;
    QString s;
    foreach(QPushButton* but, list){
        if (but->text().contains(text)) {
            GTWidget::click(os,but);
            clicked = true;
        }
    }
    GT_CHECK(clicked, "Can not find a button: " + text);
}

void StartupDialogFiller::use(QWidget *dialog) {
    clickButton(dialog, "OK");
}

QLineEdit * StartupDialogFiller::getPathEdit(QWidget *dialog) {
    QList<QLineEdit*> list = dialog->findChildren<QLineEdit*>();
    GT_CHECK_RESULT(!list.isEmpty(), "No line edit", NULL);

    return list.first();
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
