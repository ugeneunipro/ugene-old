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


#include "ConvertAceToSqliteDialogFiller.h"
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#endif
#include "api/GTLineEdit.h"
#include "api/GTWidget.h"
#include "runnables/qt/MessageBoxFiller.h"
namespace U2{

#define GT_CLASS_NAME "GTUtilsDialog::SaveProjectAsDialogFiller"
#define GT_METHOD_NAME "run"
void ConvertAceToSqliteDialogFiller::run(){
    QString button;
    switch (action) {
        case NOT_SET:
            button = "";
            break;
        case REPLACE:
            button = "Replace";
            break;
        case APPEND:
            button = "Append";
            break;
        case CANCEL:
            button = "Cancel";
            break;
    }
    MessageBoxDialogFiller *mbf = new MessageBoxDialogFiller(os, button);
    if (NOT_SET == action) {
        //when launching filler 2 or more times messagebox is caught by wrong filler
        //GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, mbf);
    } else {
        GTUtilsDialog::waitForDialog(os, mbf);
    }
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit* leDest = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leDest",dialog));
    GT_CHECK(leDest, "destination URL lineedit not found");
    GTLineEdit::setText(os, leDest, leDestUrl);

    GTWidget::click(os, GTWidget::findButtonByText(os, "OK",dialog));
    GTGlobals::sleep(300);

    dialog = QApplication::activeModalWidget();
    if(dialog!=NULL){
        GTWidget::click(os, GTWidget::findButtonByText(os, "Cancel",dialog));
    }

}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
