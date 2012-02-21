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

namespace U2 {

class GUIDialogUtils {
public:
    static void openExportProjectDialog(U2OpStatus &os);
    static void checkExportProjectDialog(U2OpStatus &os, const QString& projectName);

    static void clickMessageBoxButton(U2OpStatus &os, QMessageBox::StandardButton b);

    static void fillInSaveProjectAsDialog(U2OpStatus &os, const QString &projectName, const QString &projectFolder, const QString &projectFile, bool pressCancel = false);
};

} // namespace

#endif
