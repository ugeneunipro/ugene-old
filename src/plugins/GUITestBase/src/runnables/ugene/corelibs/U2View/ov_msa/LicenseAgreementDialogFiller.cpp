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

#include <QPushButton>
#include <QApplication>

#include "LicenseAgreementDialogFiller.h"
#include "utils/GTThread.h"
#include <primitives/GTWidget.h>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::GenerateAlignmentProfileDialogFiller"

LicenseAgreemntDialogFiller::LicenseAgreemntDialogFiller(U2OpStatus &os) :
    Filler(os, "LicenseDialog")
{

}

#define GT_METHOD_NAME "commonScenario"
void LicenseAgreemntDialogFiller::commonScenario() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");
    GTGlobals::sleep();

    QPushButton* accept = dialog->findChild<QPushButton*>("acceptButton");
    GTWidget::click(os, accept);
    GTThread::waitForMainThread(os);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
