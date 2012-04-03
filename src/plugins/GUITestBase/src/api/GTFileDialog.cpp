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

#include "GTFileDialog.h"
#include "GTMenu.h"
#include "GTKeyboardDriver.h"
#include "GTMouseDriver.h"
#include "GTComboBox.h"
#include "GTUtilsDialog.h"
#include "api/GTGlobals.h"
#include <U2Gui/MainWindow.h>
#include <QtGui/QApplication>
#include <QtGui/QLineEdit>
#include <QtGui/QTreeView>
#include <QtGui/QFileSystemModel>
#include <QtGui/QHeaderView>
#include <QtGui/QFileDialog>
#include <QtGui/QPushButton>

#define FILE_NAME_LINE_EDIT "fileNameEdit"

namespace U2 {

class GTFileDialogUtils : public Runnable {
public:
    enum Button {Open, Cancel};
    enum ViewMode {List, Detail};

    GTFileDialogUtils(U2OpStatus&, const QString&, const QString&, const QString&, Button, GTGlobals::UseMethod);

    void openFileDialog();
    void run();

private:
    void setFilter();
    void setPath();
    void selectFile();
    void clickButton(Button);
    void setViewMode(ViewMode);

    QWidget *fileDialog;
    U2OpStatus &os;
    QString path, fileName, filters;
    Button button;
    GTGlobals::UseMethod method;
};

GTFileDialogUtils::GTFileDialogUtils(U2OpStatus &_os, const QString &_path, const QString &_fileName,
                                     const QString &_filters, Button _button, GTGlobals::UseMethod _method) :
    os(_os),
    path(_path),
    fileName(_fileName),
    filters(_filters),
    button(_button),
    method(_method)
{
}

void GTFileDialogUtils::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    CHECK_SET_ERR (dialog != NULL && QString(dialog->metaObject()->className()) == "QFileDialog",
                   "Error: file dialog not found in GTFileDialog::run()");

    fileDialog = dialog;

    GTGlobals::sleep(200);
    setPath();
    GTGlobals::sleep(200);
    clickButton(Open);
    GTGlobals::sleep(200);
    setFilter();
    GTGlobals::sleep(200);
    setViewMode(Detail);
    GTGlobals::sleep(200);
    selectFile();
    GTGlobals::sleep(2000);
    clickButton(button);
}

void GTFileDialogUtils::openFileDialog()
{
    QMenu *menu;
    QStringList itemPath;
    itemPath << ACTION_PROJECTSUPPORT__OPEN_PROJECT;

    switch(method) {
    case GTGlobals::UseMouse:
        menu = GTMenu::showMainMenu(os, MWMENU_FILE, method);
        GTMenu::clickMenuItem(os, menu, itemPath);
        break;

    case GTGlobals::UseKey:
        GTKeyboardDriver::keyClick(os, 'O', GTKeyboardDriver::key["ctrl"]);
        break;
    }
}

void GTFileDialogUtils::setPath()
{
    QLineEdit* lineEdit = fileDialog->findChild<QLineEdit*>(FILE_NAME_LINE_EDIT);
    CHECK_SET_ERR(lineEdit != 0, QString("Error: line edit \"1\" not found in GTFileDialog::setPath()").arg(FILE_NAME_LINE_EDIT));

    QPoint linePos;

    switch(method) {
    case GTGlobals::UseMouse:
        if (! lineEdit->hasFocus()) {
            linePos = fileDialog->mapToGlobal(lineEdit->geometry().center());
            GTMouseDriver::moveTo(os, linePos);
            GTMouseDriver::click(os);
        }
        break;

    case GTGlobals::UseKey:
        while (! lineEdit->hasFocus()) {
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["tab"]);
            GTGlobals::sleep(100);
        }
        break;
    }

    GTKeyboardDriver::keySequence(os, path);
    while (lineEdit->text() != path) {
        GTGlobals::sleep(100);
    }
}

void GTFileDialogUtils::setFilter()
{
    QComboBox *cmb = fileDialog->findChild<QComboBox*>("fileTypeCombo");
    CHECK_SET_ERR (cmb != NULL, "Error: combobox, which contains files filters, not found in GTFileDialogUtils::setFilter()");

    int index = -1;
    for (int i = 0; i < cmb->count(); i++ ) {
        if (cmb->itemText(i).contains(filters)) {
            index = i;
            break;
        }
    }

    CHECK_SET_ERR (index != -1, QString("Error: item \"%1\" in combobox not found in GTFileDialogUtils::setFilter()").arg(filters));

    GTComboBox::setCurrentIndex(os, cmb, index/*, method*/);
}

void GTFileDialogUtils::selectFile()
{
    QTreeView *w = fileDialog->findChild<QTreeView*>("treeView");
    CHECK_SET_ERR (w != NULL, "Error: widget, which contains list of file, not found in GTFileDialogUtils::selectFile()");

    QFileSystemModel *model = qobject_cast<QFileSystemModel*>(w->model());
    QModelIndex index = model->index(path + fileName);
    QPoint indexCenter;

    switch(method) {
    case GTGlobals::UseKey:
//        while (! w->hasFocus()) {
//            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["tab"]);
//            GTGlobals::sleep(100);
//        }

//        while (qobject_cast<QFileDialog*>(fileDialog)->selectedFiles().indexOf(path + fileName) == -1) {
//            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
//            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
//            GTGlobals::sleep(1000);
//        }
//        break;
/// TODO: above don't work, if show only one file

    case GTGlobals::UseMouse:
        if (! w->rect().contains(w->visualRect(index))) {
            GTMouseDriver::moveTo(os, w->mapToGlobal(w->geometry().center()));
            GTMouseDriver::click(os);
        }
        while (! w->rect().contains(w->visualRect(index))) {
            GTMouseDriver::scroll(os, -1);
            GTGlobals::sleep(100);
        }

        indexCenter = w->visualRect(index).center();
        indexCenter.setY(indexCenter.y() + w->header()->rect().height());
        GTMouseDriver::moveTo(os, w->mapToGlobal(indexCenter));
        GTMouseDriver::click(os);
        break;
    }

    GTGlobals::sleep(100);
}

void GTFileDialogUtils::clickButton(Button btn)
{
    QList<QPushButton*> buttons = fileDialog->findChildren<QPushButton *>();
    QMap<Button, QString> button;
    button[Open] = "Open";
    button[Cancel] = "Cancel";
    QPushButton *button_to_click = NULL;

    foreach(QPushButton *b, buttons) {
        if (b->text().contains(button[btn])) {
            button_to_click = b;
            break;
        }
    }
    CHECK_SET_ERR (button_to_click != NULL, "Error: button not foud in GTFileDialog::clickButton()");

    while (! button_to_click->isEnabled()) {
        GTGlobals::sleep(100);
    }

    QPoint btn_pos;
    int key = 0, key_pos;

    switch(method) {
    case GTGlobals::UseKey:
        key_pos = button_to_click->text().indexOf('&');
        if (key_pos != -1) {
            key = (button_to_click->text().at(key_pos + 1)).toAscii();
            GTKeyboardDriver::keyClick(os, key, GTKeyboardDriver::key["alt"]);
            break;
        }

    case GTGlobals::UseMouse:
        btn_pos = button_to_click->mapToGlobal(button_to_click->rect().center());
        GTMouseDriver::moveTo(os, btn_pos);
        GTMouseDriver::click(os);
        GTMouseDriver::click(os); //second click is needed for Linux
        break;
    }
}

void GTFileDialogUtils::setViewMode(ViewMode v)
{
    QMap<ViewMode, QString> button;
    button[List] = "listModeButton";
    button[Detail] = "detailModeButton";
    QWidget *w = fileDialog->findChild<QWidget*>(button[v]);

    CHECK_SET_ERR (w != NULL, "Error: view mode button not found in GTFileDialogUtils::setViewMode()");

    switch(method) {
    case GTGlobals::UseMouse:
        GTMouseDriver::moveTo(os, w->mapToGlobal(w->rect().center()));
        GTMouseDriver::click(os);
        break;

    case GTGlobals::UseKey:
        while (! w->hasFocus()) {
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["tab"]);
            GTGlobals::sleep(100);
        }
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
        break;
    }

    GTGlobals::sleep(100);
}

void GTFileDialog::openFile(U2OpStatus &os, const QString &path, const QString &fileName,
                            const QString &filters, Button button, GTGlobals::UseMethod m)
{
    QString _path = QDir::cleanPath(QDir::currentPath() + "/" + path);
    if (_path.at(_path.count() - 1) != '/') {
        _path += '/';
    }

    GTFileDialogUtils ob(os, _path, fileName, filters, (GTFileDialogUtils::Button)button, m);
    if (m == GTGlobals::UseMouse) {
        GTUtilsDialog::preWaitForDialog(os, &ob);
    }

    ob.openFileDialog();

    if (m == GTGlobals::UseKey) {
        GTUtilsDialog::waitForDialog(os, &ob);
    }
}

} // namespace
