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

#ifndef _U2_PROJECT_UTILS_H_
#define _U2_PROJECT_UTILS_H_

#include "api/GTGlobals.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsDialogRunnables.h"
#include <U2Core/GUrl.h>
#include <QtGui/QMessageBox>

namespace U2 {

class GUITest;
class Document;

class GTUtilsProject {
public:
    class OpenFileSettings {
    public:
        enum OpenMethod {DragDrop} openMethod;
    };

    /*
        opens files using settings, checks if the document is loaded
    */
    static void openFiles(U2OpStatus &os, const QList<QUrl> &urls, const OpenFileSettings& s = OpenFileSettings());
    static void openFiles(U2OpStatus &os, const GUrl &path, const OpenFileSettings& s = OpenFileSettings());

    enum CheckType {Exists, Empty, NotExists};
    static void checkProject(U2OpStatus &os, CheckType checkType = Exists);

protected:
    static void openFilesDrop(U2OpStatus &os, const QList<QUrl>& urls);
};

} // U2

#endif
