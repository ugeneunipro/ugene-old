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

#ifndef _U2_GUI_DIALOG_UTILS_H_
#define _U2_GUI_DIALOG_UTILS_H_

#include <U2Core/U2OpStatus.h>
#include <QtGui/QMessageBox>
#include <U2Test/GUITestBase.h>

namespace U2 {

class GUIDialogUtils {
public:
    static void openExportProjectDialog(U2OpStatus &os);
    GENERATE_GUI_ACTION(OpenExportProjectDialogGUIAction, openExportProjectDialog);

    static void checkExportProjectDialog(U2OpStatus &os, const QString& projectName);
    GENERATE_GUI_ACTION_1(CheckExportProjectDialogGUIAction, checkExportProjectDialog);

    static void fillInExportProjectDialog(U2OpStatus &os, const QString &projectFolder, const QString &projectName);
    GENERATE_GUI_ACTION_2(FillInExportProjectDialogGUIAction, fillInExportProjectDialog);


    class ClickMessageBoxButtonGUIAction : public GUITest {
    public:
        ClickMessageBoxButtonGUIAction(QMessageBox::StandardButton _b) : b(_b){}
    protected:
        virtual void execute(U2OpStatus &os);
        QMessageBox::StandardButton b;
    };

    static void openSaveProjectAsDialog(U2OpStatus &os);
    GENERATE_GUI_ACTION(OpenSaveProjectAsDialogGUIAction, openSaveProjectAsDialog);

    class FillInSaveProjectAsDialogGUIAction : public GUITest {
    public:
        FillInSaveProjectAsDialogGUIAction(const QString &_projectName, const QString &_projectFolder, const QString &_projectFile, bool _pressCancel = false)
            : projectName(_projectName), projectFolder(_projectFolder), projectFile(_projectFile), pressCancel(_pressCancel){}
    protected:
        virtual void execute(U2OpStatus &os);
        QString projectName, projectFolder, projectFile;
        bool pressCancel;
    };
};

} // namespace

#endif
