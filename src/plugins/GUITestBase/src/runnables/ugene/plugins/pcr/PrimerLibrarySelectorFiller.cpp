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

#include "GTUtilsPrimerLibrary.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"

#include "PrimerLibrarySelectorFiller.h"

namespace U2 {

PrimerLibrarySelectorFiller::PrimerLibrarySelectorFiller(U2OpStatus &os, int number, bool doubleClick)
: Filler(os, "PrimerLibrarySelector"), number(number), doubleClick(doubleClick)
{

}

#define GT_CLASS_NAME "GTUtilsDialog::PrimerLibrarySelectorFiller"
#define GT_METHOD_NAME "run"
void PrimerLibrarySelectorFiller::commonScenario() {
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QAbstractButton *okButton = GTUtilsDialog::buttonBox(os, dialog)->button(QDialogButtonBox::Ok);
    CHECK_SET_ERR(!okButton->isEnabled(), "the OK button is enabled");

    int lastPrimer = GTUtilsPrimerLibrary::librarySize(os) - 1;
    GTMouseDriver::moveTo(os, GTUtilsPrimerLibrary::getPrimerPoint(os, lastPrimer));
    GTMouseDriver::click(os);
    CHECK_SET_ERR(okButton->isEnabled(), "the OK button is disabled");

    if (doubleClick) {
        GTMouseDriver::doubleClick(os);
    } else {
        GTWidget::click(os, okButton);
    }
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

} // U2