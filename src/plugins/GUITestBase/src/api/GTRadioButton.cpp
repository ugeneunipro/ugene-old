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

#include "GTRadioButton.h"
#include "GTWidget.h"
#include <U2Gui/MainWindow.h>
#include <U2Core/AppContext.h>
#include <QMainWindow>

#include "GTMouseDriver.h"
#include "GTKeyboardDriver.h"

namespace U2 {

#define GT_CLASS_NAME "GTRadioButton"

#define GT_METHOD_NAME "click"
void GTRadioButton::click(U2OpStatus& os, QRadioButton *radioButton) {
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
QRadioButton* GTRadioButton::getRadioButtonByText(U2OpStatus &os, QString text, QWidget *parent){
    QRadioButton* result=NULL;
    if(parent==NULL){
        parent=AppContext::getMainWindow()->getQMainWindow();
    }
    QList<QRadioButton*> radioList = parent->findChildren<QRadioButton*>();
    foreach(QRadioButton* but, radioList){
        if(but->text().toLower().contains(text.toLower())){
            GT_CHECK_RESULT(result==NULL, "Several radioButtons contain this text.",result)
            result=but;
        }
    }

    GT_CHECK_RESULT(result,"No radioButtons with this text found",NULL);
    return result;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
