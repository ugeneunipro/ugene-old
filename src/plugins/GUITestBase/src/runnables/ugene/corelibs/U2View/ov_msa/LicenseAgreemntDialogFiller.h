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

#ifndef LICENSEAGREEMNTDIALOGFILLER_H
#define LICENSEAGREEMNTDIALOGFILLER_H

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPushButton>
#include <QtGui/QApplication>
#else
#include <QtWidgets/QPushButton>
#include <QtWidgets/QApplication>
#endif

#include "GTUtilsDialog.h"
#include "api/GTWidget.h"

namespace U2 {

class LicenseAgreemntDialogFiller : public Filler {
public:
    LicenseAgreemntDialogFiller(U2OpStatus &os) : Filler(os, "LicenseDialog"){}
    virtual void run(){
        QWidget* dialog = QApplication::activeModalWidget();
        CHECK_SET_ERR(dialog, "activeModalWidget is NULL");
        GTGlobals::sleep(1000);

        QPushButton* accept = dialog->findChild<QPushButton*>("acceptButton");
        GTWidget::click(os, accept);
    }
private:

};

}
#endif // LICENSEAGREEMNTDIALOGFILLER_H
