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

#include "NCBISearchDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/DownloadRemoteFileDialogFiller.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QTreeWidget>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QTreeWidget>
#endif

#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTSpinBox.h"
#include "api/GTKeyboardDriver.h"
namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::NCBISearchDialogFiller"
#define GT_METHOD_NAME "run"
void NCBISearchDialogFiller::run(){

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    GTGlobals::sleep(500);

    QLineEdit* queryEditLE = qobject_cast<QLineEdit*>(GTWidget::findWidget(os,"queryEditLineEdit", dialog));
    GT_CHECK(queryEditLE != NULL,"queryEdit line not found");
    GTLineEdit::setText(os, queryEditLE, query);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep(5000);


    if (resultLimit!=-1){
        QSpinBox* resultLimitBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "resultLimitBox", dialog));
        GTSpinBox::setValue(os, resultLimitBox, resultLimit, GTGlobals::UseKeyBoard);
        GTWidget::click(os, GTWidget::findWidget(os, "searchButton"));
        GTGlobals::sleep(5000);
        int i = getResultNumber();
        GT_CHECK(i==resultLimit,QString("unexpected number of results. Expected: %1, found: %2").arg(resultLimit).arg(i))
    }

    if (doubleEnter) {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
        GTGlobals::sleep(5000);
        GTWidget::click(os, GTWidget::findWidget(os, "closeButton"));
        return;
    }

    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFiller(os,"",0,false,QString(),GTGlobals::UseMouse,1));
    QTreeWidget* w = dialog->findChild<QTreeWidget*>("treeWidget");
    GT_CHECK(w, "treeWidget not found");

    GTWidget::click(os, w, Qt::LeftButton, QPoint(10,35));//fast fix, clicking first result
    GTWidget::click(os, GTWidget::findWidget(os, "downloadButton"));

    GTWidget::click(os, GTWidget::findWidget(os, "closeButton"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "shownCorrect"
bool NCBISearchDialogFiller::shownCorrect(){
    return true;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "NCBISearchDialogFiller::getResultNumber"
int NCBISearchDialogFiller::getResultNumber(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK_RESULT(dialog, "activeModalWidget is NULL",-1);

    QTreeWidget* w = dialog->findChild<QTreeWidget*>("treeWidget");
    GT_CHECK_RESULT(w, "treeWidget not found",-1);
    return w->topLevelItemCount();
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
