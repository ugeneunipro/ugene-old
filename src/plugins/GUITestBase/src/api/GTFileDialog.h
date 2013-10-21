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

#ifndef GTFILE_DIALOG_H
#define GTFILE_DIALOG_H

#include "GTGlobals.h"
#include "GTUtilsDialog.h"

namespace U2 {

class GTFileDialogUtils : public Filler {
friend class GTFileDialogUtils_list;
public:
    enum Button {Open, Cancel, Save, Choose};
    enum ViewMode {List, Detail};

    GTFileDialogUtils(U2OpStatus&, const QString&, const QString&, const QString& = "*.*", Button b = Open, GTGlobals::UseMethod = GTGlobals::UseMouse);
    GTFileDialogUtils(U2OpStatus&, const QString&, const QString&, qint64* _size);
    GTFileDialogUtils(U2OpStatus&, const QString& filePath, GTGlobals::UseMethod method = GTGlobals::UseMouse);
    void openFileDialog();
    void run();

private:

    void setFilter();
    void setPath();
    void setName();
    void selectFile();
    void clickButton(Button);
    void setViewMode(ViewMode);
    qint64 getSize();

    QWidget *fileDialog;
    QString path, fileName, filters;
    Button button;
    GTGlobals::UseMethod method;
    bool isForGetSize;
    qint64* size;
};
class GTFileDialogUtils_list : public GTFileDialogUtils{
public:
    GTFileDialogUtils_list(U2OpStatus&, const QString&, const QStringList&);
    void setNameList(U2OpStatus &, const QStringList &);
    void run();
private:
    QString path;
    QStringList fileNameList;
    void selectFile();
};

class GTFileDialog {
public:
    enum Button {Open, Cancel};
    static void openFile(U2OpStatus &os, const QString &path, const QString &fileName,
        const QString &filters = "*.*", Button button = Open, GTGlobals::UseMethod m = GTGlobals::UseMouse);
    static void openFileList(U2OpStatus &, const QString &, const QStringList &);
    static qint64 getSize(U2OpStatus &os, const QString &path, const QString &fileName);
};

} // namespace

#endif // GTFILE_DIALOG_H
