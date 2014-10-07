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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QListWidget>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QListWidget>
#endif

#include "GTDatabaseConfig.h"
#include "GTUtilsProjectTreeView.h"
#include "SharedConnectionsDialogFiller.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTRadioButton.h"
#include "runnables/qt/MessageBoxFiller.h"

namespace U2 {

SharedConnectionsDialogFiller::Action::Action(Type type, QString itemName)
: type(type), itemName(itemName), expectedResult(OK)
{
    dbName = GTDatabaseConfig::database();
}

SharedConnectionsDialogFiller::SharedConnectionsDialogFiller(U2OpStatus &os, const QList<Action> &actions, const QFlags<Behavior> &behavior)
    : Filler(os, "SharedConnectionsDialog"), actions(actions), behavior(behavior)
{

}

namespace {

QListWidgetItem * findConnection(U2OpStatus &os, QListWidget *list, const QString &name, GTGlobals::FindOptions options = GTGlobals::FindOptions()) {
    QList<QListWidgetItem*> items = list->findItems(name, Qt::MatchExactly);
    if (1 != items.size()) {
        if (options.failIfNull) {
            os.setError("List item not found");
        }
        return NULL;
    }

    return items.first();
}

void clickConnection(U2OpStatus &os, QListWidget *list, const QString &name) {
    QListWidgetItem *item = findConnection(os, list, name);
    CHECK_OP(os, );

    QRect rect = list->visualItemRect(item);
    QPoint point = list->mapToGlobal(rect.center());
    GTMouseDriver::moveTo(os, point);
    GTMouseDriver::click(os);
}

void checkDocument(U2OpStatus &os, const QString &name, bool mustBe) {
    QModelIndex idx = GTUtilsProjectTreeView::findIndex(os, name, GTGlobals::FindOptions(false));
    if (!mustBe) {
        CHECK_SET_ERR(!idx.isValid(), "Document is not deleted");
    }
}

void waitForConnection(U2OpStatus &os, const SharedConnectionsDialogFiller::Action &action) {
    switch (action.expectedResult) {
        case SharedConnectionsDialogFiller::Action::OK:
            break;
        case SharedConnectionsDialogFiller::Action::WRONG_DATA:
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "OK", "Unable to connect"));
            break;
        case SharedConnectionsDialogFiller::Action::INITIALIZE:
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
            break;
        case SharedConnectionsDialogFiller::Action::DONT_INITIALIZE:
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
            break;
        case SharedConnectionsDialogFiller::Action::VERSION:
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "OK", "recent version of UGENE"));
            break;
        case SharedConnectionsDialogFiller::Action::LOGIN:
            break;
        default:
            os.setError("Unknown expected result");
    }
}

void establishConnection(U2OpStatus &os, const SharedConnectionsDialogFiller::Action &action,
    const QFlags<SharedConnectionsDialogFiller::Behavior> &behavior)
{
    GTGlobals::sleep(1000);
    QWidget *cnctBtn = GTWidget::findWidget(os,"pbConnect");
    QWidget *dcntBtn = GTWidget::findWidget(os,"pbDisconnect");
    QWidget *editBtn = GTWidget::findWidget(os,"pbEdit");
    QListWidget *list = dynamic_cast<QListWidget*>(GTWidget::findWidget(os, "lwConnections"));

    waitForConnection(os, action);
    CHECK_OP(os, );

    GTWidget::click(os, cnctBtn, Qt::LeftButton, QPoint(), false);//no processing events after clicking
    CHECK(behavior.testFlag(SharedConnectionsDialogFiller::SAFE), );

    GTGlobals::sleep(2000);

    if (SharedConnectionsDialogFiller::Action::OK == action.expectedResult ||
        SharedConnectionsDialogFiller::Action::INITIALIZE == action.expectedResult) {
        if (SharedConnectionsDialogFiller::Action::INITIALIZE == action.expectedResult) {
            GTGlobals::sleep(10000);
        }
        CHECK_SET_ERR(!cnctBtn->isEnabled(), "connect button enabled");
        CHECK_SET_ERR(!editBtn->isEnabled(), "edit button enabled");
        CHECK_SET_ERR(dcntBtn->isEnabled(), "disconnect button disabled");

        // Check connection icon
        QListWidgetItem *item = findConnection(os, list, action.itemName);
        CHECK_OP(os, );
        CHECK_SET_ERR(!item->icon().isNull(), "no icon");

        // Check project view
        checkDocument(os, action.dbName, true);
    }
}

void deleteConnection(U2OpStatus &os, const SharedConnectionsDialogFiller::Action &action) {
    QListWidget *list = dynamic_cast<QListWidget*>(GTWidget::findWidget(os, "lwConnections"));

    GTWidget::click(os, GTWidget::findWidget(os,"pbDelete"));
    GTGlobals::sleep(2000);

    // Check connection item
    QListWidgetItem *item = findConnection(os, list, action.itemName, GTGlobals::FindOptions(false));
    CHECK_SET_ERR(NULL == item, "Item is not deleted");

    // Check project view
    checkDocument(os, action.dbName, false);
}

void stopConnection(U2OpStatus &os, const SharedConnectionsDialogFiller::Action &action) {
    QWidget *cnctBtn = GTWidget::findWidget(os,"pbConnect");
    QWidget *dcntBtn = GTWidget::findWidget(os,"pbDisconnect");
    QWidget *editBtn = GTWidget::findWidget(os,"pbEdit");

    GTWidget::click(os, GTWidget::findWidget(os,"pbDisconnect"));
    GTGlobals::sleep(2000);

    CHECK_SET_ERR(cnctBtn->isEnabled(), "connect button disabled");
    CHECK_SET_ERR(editBtn->isEnabled(), "edit button disabled");
    CHECK_SET_ERR(!dcntBtn->isEnabled(), "disconnect button enabled");

    // Check project view
    checkDocument(os, action.dbName, false);
}

}

#define GT_CLASS_NAME "GTUtilsDialog::SharedConnectionsDialogFiller"
#define GT_METHOD_NAME "run"

void SharedConnectionsDialogFiller::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");
    QListWidget *list = dynamic_cast<QListWidget*>(GTWidget::findWidget(os, "lwConnections"));
    CHECK_SET_ERR(NULL != list, "NULL list");
    GTGlobals::sleep(1000);

    bool connected = false;
    foreach (const Action &action, actions) {
        CHECK_SET_ERR(!connected, "The dialog must be closed but not all actions are processed");
        switch (action.type) {
            case Action::ADD:
                GTWidget::click(os, GTWidget::findWidget(os,"pbAdd"));
                break;
            case Action::CLICK:
                clickConnection(os, list, action.itemName);
                break;
            case Action::EDIT:
                GTWidget::click(os, GTWidget::findWidget(os,"pbEdit"));
                break;
            case Action::DELETE:
                deleteConnection(os, action);
                CHECK_OP(os, );
                break;
            case Action::CONNECT:
                establishConnection(os, action, behavior);
                CHECK_OP(os, );
                if (Action::OK == action.expectedResult) {
                    connected = true;
                }
                break;
            case Action::DISCONNECT:
                stopConnection(os, action);
                CHECK_OP(os, );
                break;
            default:
                os.setError("Unknown action type");
                return;
        }
        CHECK_OP(os, );
    }

    if (!connected) {
        GTWidget::click(os, GTWidget::findButtonByText(os, "Close"));
    }
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

} // U2
