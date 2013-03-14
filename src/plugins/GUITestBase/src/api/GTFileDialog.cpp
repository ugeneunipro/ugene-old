/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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
#include "api/GTGlobals.h"
#include "api/GTLineEdit.h"

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

#define GT_CLASS_NAME "GTFileDialogUtils"

GTFileDialogUtils::GTFileDialogUtils(U2OpStatus &_os, const QString &_path, const QString &_fileName,
                                     const QString &_filters, Button _button, GTGlobals::UseMethod _method) :
    Filler(_os, "QFileDialog"),
//    path(_path),
    fileName(_fileName),
    filters(_filters),
    button(_button),
    method(_method),
    isForGetSize(false)
{
    path = QDir::cleanPath(QDir::currentPath() + "/" + _path);
    if (path.at(path.count() - 1) != '/') {
        path += '/';
    }
}

GTFileDialogUtils::GTFileDialogUtils(U2OpStatus &_os, const QString &_path, const QString &_fileName,
                                     qint64 *_size):
    Filler(_os, "QFileDialog"),
    fileName(_fileName),
    button(Cancel),
    method(GTGlobals::UseMouse),
    isForGetSize(true),
    size(_size)
{
    path = QDir::cleanPath(QDir::currentPath() + "/" + _path);
    if (path.at(path.count() - 1) != '/') {
        path += '/';
}
}
#define GT_METHOD_NAME "run"
void GTFileDialogUtils::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL && QString(dialog->metaObject()->className()) == "QFileDialog",
                   "file dialog not found");

    fileDialog = dialog;
    GTGlobals::sleep(200);
    setPath();
    GTGlobals::sleep(200);
    clickButton(Open);
    GTGlobals::sleep(200);
    //setFilter();
    if(button == Save){//saving file
        setName();
    }
    else{//opening file or getting size
        GTGlobals::sleep(200);
        setViewMode(Detail);
        GTGlobals::sleep(200);
        selectFile();
        GTGlobals::sleep(200);
    }

    if (isForGetSize){
        qint64 i = getSize();
        *size = i;
    }

    clickButton(button);

}
#undef GT_METHOD_NAME

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
    GTGlobals::sleep(500);
}

#define GT_METHOD_NAME "setPath"
void GTFileDialogUtils::setPath()
{
    QLineEdit* lineEdit = fileDialog->findChild<QLineEdit*>(FILE_NAME_LINE_EDIT);
    GT_CHECK(lineEdit != 0, QString("line edit \"1\" not found").arg(FILE_NAME_LINE_EDIT));

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
    for (int i = 0; i < 20; i++) { // wait 2 sec
        if (lineEdit->text() == path) {
            return;
        }
        GTGlobals::sleep(100);
    }

    GT_CHECK(lineEdit->text() == path, "Can't open file \"" + lineEdit->text() + "\"");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setName"
void GTFileDialogUtils::setName()
{
    QLineEdit* lineEdit = fileDialog->findChild<QLineEdit*>(FILE_NAME_LINE_EDIT);
    GT_CHECK(lineEdit != 0, QString("line edit \"1\" not found").arg(FILE_NAME_LINE_EDIT));

    GTLineEdit::setText(os, lineEdit,fileName);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setFilter"
void GTFileDialogUtils::setFilter()
{
    QComboBox *cmb = fileDialog->findChild<QComboBox*>("fileTypeCombo");
    GT_CHECK(cmb != NULL, "combobox, which contains files filters, not found");

    int index = -1;
    for (int i = 0; i < cmb->count(); i++ ) {
        if (cmb->itemText(i).contains(filters)) {
            index = i;
            break;
        }
    }

    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(filters));

    GTComboBox::setCurrentIndex(os, cmb, index/*, method*/);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectFile"
void GTFileDialogUtils::selectFile()
{
    QTreeView *w = fileDialog->findChild<QTreeView*>("treeView");
    GT_CHECK(w != NULL, "widget, which contains list of file, not found");

    QFileSystemModel *model = qobject_cast<QFileSystemModel*>(w->model());
    QModelIndex index = model->index(path + fileName);
    GT_CHECK(index.isValid(), "File <" + path + fileName + "> not found");

    QPoint indexCenter;

    switch(method) {
    case GTGlobals::UseKey:
        while (! w->hasFocus()) {
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["tab"]);
            GTGlobals::sleep(100);
        }

        while (qobject_cast<QFileDialog*>(fileDialog)->selectedFiles().indexOf(path + fileName) == -1) {
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
            GTGlobals::sleep(100);
        }
        break;

    case GTGlobals::UseMouse:
        if (! w->viewport()->rect().contains(w->visualRect(index))) {
            GTMouseDriver::moveTo(os, w->mapToGlobal(w->geometry().center()));
            GTMouseDriver::click(os);
        }
        while (! w->viewport()->rect().contains(w->visualRect(index))) {
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
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickButton"
void GTFileDialogUtils::clickButton(Button btn)
{
    QList<QPushButton*> buttons = fileDialog->findChildren<QPushButton *>();
    QMap<Button, QString> button;
    button[Open] = "Open";
    button[Cancel] = "Cancel";
    button[Save] = "Save";
    QPushButton *button_to_click = NULL;

    foreach(QPushButton *b, buttons) {
        if (b->text().contains(button[btn])) {
            button_to_click = b;
            break;
        }
    }
    GT_CHECK(button_to_click != NULL, "button not found");

    while (! button_to_click->isEnabled()) {
        GTGlobals::sleep(100);
    }

    QPoint btn_pos;
    int key = 0, key_pos;

    switch(method) {
    case GTGlobals::UseKey:
        key_pos = button_to_click->text().indexOf('&');
        if (key_pos != -1) {
            key = (button_to_click->text().at(key_pos + 1)).toLatin1();
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
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setViewMode"
void GTFileDialogUtils::setViewMode(ViewMode v)
{
    QMap<ViewMode, QString> button;
    button[List] = "listModeButton";
    button[Detail] = "detailModeButton";
    QWidget *w = fileDialog->findChild<QWidget*>(button[v]);

    GT_CHECK(w != NULL, "view mode button not found");

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
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getsize"
qint64 GTFileDialogUtils::getSize()
{
    QTreeView *w = fileDialog->findChild<QTreeView*>("treeView");
    GT_CHECK_RESULT(w != NULL, "widget, which contains list of file, not found",0);

    QFileSystemModel *model = qobject_cast<QFileSystemModel*>(w->model());
    QModelIndex index = model->index(path + fileName);
    GT_CHECK_RESULT(index.isValid(), "File <" + path + fileName + "> not found",0);

    qint64 size;
    size = model->size(index);
    GTGlobals::sleep(100);

    return size;
}
#undef GT_METHOD_NAME

void GTFileDialog::openFile(U2OpStatus &os, const QString &path, const QString &fileName,
                            const QString &filters, Button button, GTGlobals::UseMethod m)
{
    GTFileDialogUtils *ob = new GTFileDialogUtils(os, path, fileName, filters, (GTFileDialogUtils::Button)button, m);
    GTUtilsDialog::waitForDialog(os, ob);

    ob->openFileDialog();

    GTGlobals::sleep();
}

qint64 GTFileDialog::getSize(U2OpStatus &os,const QString &path, const QString &fileName)
{
    qint64 sizePtr = 0;
    GTFileDialogUtils *ob = new GTFileDialogUtils(os, path, fileName, &sizePtr);
    GTUtilsDialog::waitForDialog(os, ob);

    ob->openFileDialog();

    GTGlobals::sleep();

    return sizePtr;
}

#undef GT_CLASS_NAME

} // namespace
