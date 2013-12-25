/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "GTLineEdit.h"
#include "GTWidget.h"

#include "GTMouseDriver.h"
#include "GTKeyboardDriver.h"
#include "GTKeyboardUtils.h"

namespace U2 {

#define GT_CLASS_NAME "GTLineEdit"

#define GT_METHOD_NAME ""
void GTLineEdit::setText(U2OpStatus& os, QLineEdit* lineEdit, const QString &str, bool noCheck /*= false*/) {

    GT_CHECK(lineEdit != NULL, "lineEdit is NULL");
    if(lineEdit->text() == str){
        return;
    }

    GTWidget::setFocus(os, lineEdit);
    if (lineEdit->text() == str) {
        return;
    }
    if (!lineEdit->text().isEmpty()) {
        clear(os, lineEdit);
    }

    GTKeyboardDriver::keySequence(os, str);
    GTGlobals::sleep(500);

    if(noCheck){
        return;
    }

    QString s = lineEdit->text();
    GT_CHECK(s == str, "Can't set text, set text differs from a given string");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clear"
void GTLineEdit::clear(U2OpStatus& os, QLineEdit* lineEdit) {

    GT_CHECK(lineEdit != NULL, "lineEdit is NULL");

    GTWidget::setFocus(os, lineEdit);

    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(1000);

    QString s = lineEdit->text();
    GT_CHECK(s.isEmpty() == true, "Can't clear text, lineEdit is not empty");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "pasteClipboard"
void GTLineEdit::pasteClipboard(U2OpStatus& os, QLineEdit* lineEdit, PasteMethod pasteMethod) {

    GT_CHECK(lineEdit != NULL, "lineEdit is NULL");

    clear(os, lineEdit);
    switch(pasteMethod) {
        case Mouse:
            os.setError("GTLineEdit::pasteClipboard: Not implemented: Paste by mouse");
            break;

        default:
        case Shortcut:
            GTKeyboardUtils::paste(os);
            break;
    }

    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkTextSize"
void GTLineEdit::checkTextSize(U2OpStatus& os, QLineEdit* lineEdit) {

    GT_CHECK(lineEdit != NULL, "lineEdit is NULL");

    QMargins lineEditMargins = lineEdit->textMargins();
    QFontMetrics fontMetrics = lineEdit->fontMetrics();
    int textWidth = lineEditMargins.left() + lineEditMargins.right() + fontMetrics.width(lineEdit->text());
    int rectWidth = lineEdit->rect().width();

    GT_CHECK(textWidth <= rectWidth, "GTLineEdit::checkTextSize: Text is not inside LineEdit's rect");
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
