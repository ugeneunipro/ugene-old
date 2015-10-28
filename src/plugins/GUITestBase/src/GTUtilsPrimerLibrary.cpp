/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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
#include "GTUtilsPrimerLibrary.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTTableView.h"
#include "api/GTWidget.h"
#include "runnables/ugene/plugins/pcr/AddPrimerDialogFiller.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsPrimerLibrary"

QWidget * GTUtilsPrimerLibrary::openLibrary(U2OpStatus &os) {
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Primer" << "Primer library");
    return GTUtilsMdi::activeWindow(os);
}

void GTUtilsPrimerLibrary::clickButton(U2OpStatus &os, Button button) {
    GTWidget::click(os, getButton(os, button));
}

QAbstractButton * GTUtilsPrimerLibrary::getButton(U2OpStatus &os, Button button) {
    QDialogButtonBox *box = GTUtilsDialog::buttonBox(os, GTWidget::findWidget(os, "PrimerLibraryWidget"));
    switch (button) {
        case Add:
            return box->buttons()[1];
        case Edit:
            return box->buttons()[2];
        case Close:
            return box->button(QDialogButtonBox::Close);
        case Remove:
            return box->buttons()[3];
        case Import:
            return box->buttons()[4];
        case Export:
            return box->buttons()[5];
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

#define GT_METHOD_NAME "getPrimerSequence"
QString GTUtilsPrimerLibrary::getPrimerSequence(U2OpStatus &os, const QString &name) {
    for (int i = 0; i < GTTableView::rowCount(os, table(os)); i++) {
        if (name == GTTableView::data(os, table(os), i, 0)) {
            return getPrimerSequence(os, i);
        }
    }
    GT_CHECK_RESULT(false, QString("Primer with name '%1' not found").arg(name), "");
}
#undef GT_METHOD_NAME

QPoint GTUtilsPrimerLibrary::getPrimerPoint(U2OpStatus &os, int number) {
    return GTTableView::getCellPoint(os, table(os), number, 0);
}

void GTUtilsPrimerLibrary::clickPrimer(U2OpStatus &os, int number) {
    GTMouseDriver::moveTo(os, getPrimerPoint(os, number));
    GTMouseDriver::click(os);
}

void GTUtilsPrimerLibrary::clearLibrary(U2OpStatus &os) {
    const int size = librarySize(os);
    CHECK(size > 0, );
    GTWidget::click(os, table(os));
    selectAll(os);
    clickButton(os, Remove);
}

void GTUtilsPrimerLibrary::addPrimer(U2OpStatus &os, const QString &name, const QString &data) {
    AddPrimerDialogFiller::Parameters parameters;
    parameters.name = name;
    parameters.primer = data;
    GTUtilsDialog::waitForDialog(os, new AddPrimerDialogFiller(os, parameters));
    clickButton(os, Add);
    GTGlobals::sleep(50);
}

#define GT_METHOD_NAME "selectPrimers"
void GTUtilsPrimerLibrary::selectPrimers(U2OpStatus &os, const QList<int> &numbers) {
    const int size = librarySize(os);

#ifdef Q_OS_MAC
    int key = GTKeyboardDriver::key["cmd"];
#else
    int key = GTKeyboardDriver::key["ctrl"];
#endif

    GTKeyboardDriver::keyPress(os, key);
    foreach (int number, numbers) {
        GT_CHECK(number < size, "Primer number is out of range");
        GTUtilsPrimerLibrary::clickPrimer(os, number);
    }
    GTKeyboardDriver::keyClick(os, key);
}
#undef GT_METHOD_NAME

void GTUtilsPrimerLibrary::selectAll(U2OpStatus &os) {
    GTWidget::click(os, table(os));
    GTKeyboardUtils::selectAll(os);
}

QTableView * GTUtilsPrimerLibrary::table(U2OpStatus &os) {
    return dynamic_cast<QTableView*>(GTWidget::findWidget(os, "primerTable"));
}

#undef GT_CLASS_NAME

} // U2
