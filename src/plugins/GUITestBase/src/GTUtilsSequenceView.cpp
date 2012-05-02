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

#include "GTUtilsSequenceView.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMenu.h"
#include "GTUtilsMdi.h"
#include <QtGui/QMainWindow>
#include "U2Gui/MainWindow.h"
#include "U2View/ADVConstants.h"
#include "GTUtilsDialog.h"
#include "GTUtilsProjectTreeView.h"
#include <U2Core/AppContext.h>
#include <U2View/DetView.h>
#include <U2Gui/MainWindow.h>
#include <QClipboard>
#include <QtGui/QApplication>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>

namespace U2 {

#define GT_CLASS_NAME "GTSequenceReader"
#define GT_METHOD_NAME "run"
class GTSequenceReader : public Runnable {
public:
    GTSequenceReader(U2OpStatus &_os, QString *_str):os(_os), str(_str){}
    void run()
    {
        QWidget *widget = QApplication::activeModalWidget();

        QPlainTextEdit *textEdit = widget->findChild<QPlainTextEdit*>();
        GT_CHECK(textEdit != NULL, "PlainTextEdit not found");

        *str = textEdit->toPlainText();

        QPushButton *cancel = widget->findChild<QPushButton*>();
        GT_CHECK(cancel != NULL, "Button \"cancel\" not found");
        GTWidget::click(os, cancel);
    }

private:
    U2OpStatus &os;
    QString *str;
};
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsSequenceView"
#define GT_METHOD_NAME "getSequenceAsString"

void GTUtilsSequenceView::getSequenceAsString(U2OpStatus &os, QString &sequence)
{
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GT_CHECK(mdiWindow != NULL, "MDI window == NULL");

    QMessageBox msgBox;

    QPoint posToActiveWindow = mdiWindow->mapToGlobal(mdiWindow->rect().center());
    GTMouseDriver::moveTo(os, posToActiveWindow);

    while (!QString(qApp->widgetAt(posToActiveWindow)->metaObject()->className()).contains("U2::PanViewRenderArea")) {
        posToActiveWindow.setY(posToActiveWindow.y() - 20);
        GTMouseDriver::moveTo(os, posToActiveWindow);
        GTGlobals::sleep(100);
    }

    GTMouseDriver::moveTo(os, posToActiveWindow);
    GTMouseDriver::click(os);

    GTUtilsDialog::selectSequenceRegionDialogFiller filler(os);
    GTUtilsDialog::preWaitForDialog(os, &filler);

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);

    GTUtilsDialog::PopupChooser chooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTSequenceReader reader(os, &sequence);
    GTUtilsDialog::preWaitForDialog(os, &reader);

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getBeginOfSequenceAsString"

QString GTUtilsSequenceView::getBeginOfSequenceAsString(U2OpStatus &os, int length)
{
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GT_CHECK_RESULT(mdiWindow != NULL, "MDI window == NULL", NULL);

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    GTUtilsDialog::selectSequenceRegionDialogFiller filler(os, length);
    GTUtilsDialog::preWaitForDialog(os, &filler);

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000); // don't touch
    QString sequence;
    GTUtilsDialog::PopupChooser chooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTSequenceReader reader(os, &sequence);
    GTUtilsDialog::preWaitForDialog(os, &reader);

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep(1000);

    return sequence;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getEndOfSequenceAsString"

QString GTUtilsSequenceView::getEndOfSequenceAsString(U2OpStatus &os, int length)
{
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GT_CHECK_RESULT(mdiWindow != NULL, "MDI window == NULL", NULL);

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    GTUtilsDialog::selectSequenceRegionDialogFiller filler(os, length, false);
    GTUtilsDialog::preWaitForDialog(os, &filler);

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000); // don't touch

    QString sequence;
    GTUtilsDialog::PopupChooser chooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTSequenceReader reader(os, &sequence);
    GTUtilsDialog::preWaitForDialog(os, &reader);

    GTMouseDriver::click(os);
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep(1000);

    return sequence;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getLengthOfSequence"

int GTUtilsSequenceView::getLengthOfSequence(U2OpStatus &os)
{
    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK_RESULT(mw != NULL, "MainWindow == NULL", 0);

    MWMDIWindow *mdiWindow = mw->getMDIManager()->getActiveWindow();
    GT_CHECK_RESULT(mdiWindow != NULL, "MDI window == NULL", 0);

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    int length = -1;
    GTUtilsDialog::selectSequenceRegionDialogFiller filler(os, &length);
    GTUtilsDialog::preWaitForDialog(os, &filler);
    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);

    return length;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkSequence"
void GTUtilsSequenceView::checkSequence(U2OpStatus &os, const QString &expectedSequence)
{
    QString actualSequence;
    getSequenceAsString(os, actualSequence);

    GT_CHECK(expectedSequence == actualSequence, "Actual sequence does not match with expected sequence");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openSequenceView"
void GTUtilsSequenceView::openSequenceView(U2OpStatus &os, const QString &sequenceName){
	QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, sequenceName);
	GTMouseDriver::moveTo(os, itemPos);
	GTMouseDriver::click(os, Qt::RightButton);

	GTUtilsDialog::PopupChooser chooser(os, QStringList() << "submenu_open_view" << "action_open_view", GTGlobals::UseMouse);
	GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
	GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addSequenceView"
void GTUtilsSequenceView::addSequenceView(U2OpStatus &os, const QString &sequenceName){
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, sequenceName);
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsDialog::PopupChooser chooser(os, QStringList() << "submenu_add_view" << "action_add_view", GTGlobals::UseMouse);
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} // namespace U2
