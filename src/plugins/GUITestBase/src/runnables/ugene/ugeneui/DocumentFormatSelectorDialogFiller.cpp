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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QLabel>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#endif

#include "api/GTComboBox.h"
#include "api/GTRadioButton.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"

#include "DocumentFormatSelectorDialogFiller.h"

namespace U2{

#define GT_CLASS_NAME "DocumentFormatSelectorDialogFiller"

#define GT_METHOD_NAME "getButton"
QRadioButton* DocumentFormatSelectorDialogFiller::getButton(U2OpStatus &os){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK_RESULT(dialog, "activeModalWidget is NULL", NULL);
    QRadioButton* result = GTWidget::findExactWidget<QRadioButton*>(os, format, dialog, GTGlobals::FindOptions(false));

    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "run"
void DocumentFormatSelectorDialogFiller::commonScenario()
{
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");
    GTGlobals::sleep(500);

    QRadioButton* radio = getButton(os);
    if (NULL != radio) {
        GTRadioButton::click(os, radio);
    } else {
        QRadioButton* chooseFormatManuallyRadio = GTWidget::findExactWidget<QRadioButton*>(os, "chooseFormatManuallyRadio", dialog);
        GTRadioButton::click(os, chooseFormatManuallyRadio);
        GTGlobals::sleep();

        QComboBox* userSelectedFormat = GTWidget::findExactWidget<QComboBox*>(os, "userSelectedFormat", dialog);
        GTComboBox::setIndexWithText(os, userSelectedFormat, format, true, GTGlobals::UseMouse);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "cancel button is NULL");
    GTWidget::click(os, button);

}
#undef GT_CLASS_NAME
#undef GT_METHOD_NAME

}
