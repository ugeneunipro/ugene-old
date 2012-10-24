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
#include "api/GTKeyboardUtils.h"
#include "api/GTMenu.h"
#include "GTUtilsMdi.h"
#include "GTUtilsDialog.h"
#include "GTUtilsProjectTreeView.h"
#include "runnables/ugene/corelibs/U2Gui/RangeSelectionDialogFiller.h"
#include "runnables/qt/PopupChooser.h"

#include <U2Core/AppContext.h>
#include <U2View/ADVConstants.h>
#include <U2View/DetView.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QClipboard>
#include <QtGui/QMainWindow>
#include <QtGui/QApplication>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>

#include <U2View/ADVSingleSequenceWidget.h>

namespace U2 {

#define GT_CLASS_NAME "GTSequenceReader"
#define GT_METHOD_NAME "run"
class GTSequenceReader : public Filler {
public:
    GTSequenceReader(U2OpStatus &_os, QString *_str):Filler(_os, ""), str(_str){}
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

    QWidget *mdiSequenceWidget = mdiWindow->findChild<ADVSingleSequenceWidget*>();
    GTWidget::click(os, mdiSequenceWidget);

    Runnable *filler = new selectSequenceRegionDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler);

    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);

    Runnable *chooser = new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, chooser);
    Runnable *reader = new GTSequenceReader(os, &sequence);
    GTUtilsDialog::waitForDialog(os, reader);

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

    Runnable *filler = new selectSequenceRegionDialogFiller(os, length);
    GTUtilsDialog::waitForDialog(os, filler);
    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);

    GTGlobals::sleep(1000); // don't touch
    QString sequence;
    Runnable *chooser = new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, chooser);
    Runnable *reader = new GTSequenceReader(os, &sequence);
    GTUtilsDialog::waitForDialog(os, reader);

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

    Runnable *filler = new selectSequenceRegionDialogFiller(os, length, false);
    GTUtilsDialog::waitForDialog(os, filler);

    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000); // don't touch

    QString sequence;
    Runnable *chooser = new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, chooser);
    Runnable *reader = new GTSequenceReader(os, &sequence);
    GTUtilsDialog::waitForDialog(os, reader);

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

    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    int length = -1;
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, &length));
    GTKeyboardUtils::selectAll(os);
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

#define GT_METHOD_NAME "selectSequenceRegion"
void GTUtilsSequenceView::selectSequenceRegion(U2OpStatus &os, int from, int to)
{
    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK(mw != NULL, "MainWindow == NULL");

    MWMDIWindow *mdiWindow = mw->getMDIManager()->getActiveWindow();
    GT_CHECK(mdiWindow != NULL, "MDI window == NULL");

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, from, to));

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openSequenceView"
void GTUtilsSequenceView::openSequenceView(U2OpStatus &os, const QString &sequenceName){
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "submenu_open_view" << "action_open_view", GTGlobals::UseMouse));

    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, sequenceName);
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::click(os, Qt::RightButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addSequenceView"
void GTUtilsSequenceView::addSequenceView(U2OpStatus &os, const QString &sequenceName){
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "submenu_add_view" << "action_add_view", GTGlobals::UseMouse));

    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, sequenceName);
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::click(os, Qt::RightButton);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} // namespace U2
