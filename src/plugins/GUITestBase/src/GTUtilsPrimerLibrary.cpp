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

#include <QTableView>

#include <U2Gui/MainWindow.h>

#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "api/GTMenu.h"
#include "api/GTTableView.h"
#include "api/GTWidget.h"

#include "GTUtilsPrimerLibrary.h"

namespace U2 {

QWidget * GTUtilsPrimerLibrary::openLibrary(U2OpStatus &os) {
    GTMenu::clickMenuItemByText(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Primer" << "Primer Library");
    return GTUtilsMdi::activeWindow(os);
}

void GTUtilsPrimerLibrary::clickButton(U2OpStatus &os, Button button) {
    GTWidget::click(os, getButton(os, button));
}

QAbstractButton * GTUtilsPrimerLibrary::getButton(U2OpStatus &os, Button button) {
    QDialogButtonBox *box = GTUtilsDialog::buttonBox(os, GTWidget::findWidget(os, "PrimerLibraryWidget"));
    QList<QAbstractButton*> buttons = box->buttons();
    switch (button) {
        case Add:
            return box->buttons()[1];
        case Edit:
            return box->buttons()[2];
        case Close:
            return box->button(QDialogButtonBox::Close);
        case Remove:
            return box->buttons()[3];
        default:
            return NULL;
    }
}

int GTUtilsPrimerLibrary::librarySize(U2OpStatus &os) {
    return GTTableView::rowCount(os, table(os));
}

QString GTUtilsPrimerLibrary::getPrimerSequence(U2OpStatus &os, int number) {
    return GTTableView::data(os, table(os), number, 4);
}

QPoint GTUtilsPrimerLibrary::getPrimerPoint(U2OpStatus &os, int number) {
    return GTTableView::getCellPoint(os, table(os), number, 0);
}

QTableView * GTUtilsPrimerLibrary::table(U2OpStatus &os) {
    return dynamic_cast<QTableView*>(GTWidget::findWidget(os, "primerTable"));
}

} // U2
