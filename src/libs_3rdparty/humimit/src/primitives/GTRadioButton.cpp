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

#include <U2Core/U2IdTypes.h>
#include "GTRadioButton.h"
#include <primitives/GTWidget.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/AppContext.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMainWindow>
#else
#include <QtWidgets/QMainWindow>
#endif


#include <drivers/GTMouseDriver.h>
#include <drivers/GTKeyboardDriver.h>

namespace HI {
#define GT_CLASS_NAME "GTRadioButton"

#define GT_METHOD_NAME "click"
void GTRadioButton::click(U2::U2OpStatus& os, QRadioButton *radioButton) {
    GT_CHECK(radioButton != NULL, "RadioButton is NULL");
    if(radioButton->isChecked() == true){
        return;
    }

    QPoint buttonPos = radioButton->mapToGlobal(radioButton->rect().topLeft());
    buttonPos = QPoint(buttonPos.x() + 10, buttonPos.y() + 10); // moved to clickable area

    GTMouseDriver::moveTo(os, buttonPos);
    GTMouseDriver::click(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getRadioButtonByText"
QRadioButton* GTRadioButton::getRadioButtonByText(U2::U2OpStatus &os, QString text, QWidget *parent){
    if(parent==NULL){
        parent= U2::AppContext::getMainWindow()->getQMainWindow();
    }
    QList<QRadioButton*> radioList = getAllButtonsByText(os, text, parent);
    GT_CHECK_RESULT(radioList.size() > 1, "Several radioButtons contain this text", NULL);
    GT_CHECK_RESULT(radioList.size() == 0, "No radioButtons with this text found", NULL);

    return radioList[0];
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getAllButtonsByText"
QList<QRadioButton*> GTRadioButton::getAllButtonsByText(U2::U2OpStatus &os, QString text, QWidget *parent){
    Q_UNUSED(os);
    QList<QRadioButton*> result;
    if(parent==NULL){
        parent= U2::AppContext::getMainWindow()->getQMainWindow();
    }
    QList<QRadioButton*> radioList = parent->findChildren<QRadioButton*>();
    foreach(QRadioButton* but, radioList){
        QString s = but->text().toLower();
        if(but->text().toLower().contains(text.toLower())){
            result<<but;
        }
    }

    return result;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
