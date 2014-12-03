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

#include <QApplication>

#include <U2Gui/MainWindow.h>

#include "GTUtilsDialog.h"
#include "api/GTMenu.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"

#include "GTUtilsExternalTools.h"

namespace U2 {

class RemoveToolScenario : public CustomScenario {
public:
    RemoveToolScenario(const QString &toolName)
    : CustomScenario(), toolName(toolName)
    {

    }

    void run(U2OpStatus &os) {
        AppSettingsDialogFiller::clearToolPath(os, toolName);

        QWidget *dialog = QApplication::activeModalWidget();
        CHECK_SET_ERR(dialog, "activeModalWidget is NULL");
        GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
    }

private:
    const QString toolName;
};

class SetToolUrlScenario : public CustomScenario {
public:
    SetToolUrlScenario(const QString &toolName, const QString &url)
        : CustomScenario(), toolName(toolName), url(url)
    {

    }

    void run(U2OpStatus &os) {
        AppSettingsDialogFiller::setExternalToolPath(os, toolName, url);

        QWidget *dialog = QApplication::activeModalWidget();
        CHECK_SET_ERR(dialog, "activeModalWidget is NULL");
        GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
    }

private:
    const QString toolName;
    const QString url;
};

#define GT_CLASS_NAME "GTUtilsExternalTools"

#define GT_METHOD_NAME "removeTool"
void GTUtilsExternalTools::removeTool(U2OpStatus &os, const QString &toolName) {
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new RemoveToolScenario(toolName)));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setToolUrl"
void GTUtilsExternalTools::setToolUrl(U2OpStatus &os, const QString &toolName, const QString &url) {
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new SetToolUrlScenario(toolName, url)));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} // U2
