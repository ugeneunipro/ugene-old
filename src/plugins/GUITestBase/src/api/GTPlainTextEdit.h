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

#ifndef _U2_GT_PLAINTEXTEDIT_H_
#define _U2_GT_PLAINTEXTEDIT_H_

#include "api/GTGlobals.h"
#include <QtGui/QPlainTextEdit>

namespace U2 {

class GTPlainTextEdit {
public:
    enum PasteMethod {Shortcut, Mouse};

    // fails if lineEdit is NULL, GTLineEdit::clear fails
    // or a set text differs from a given string
    static void setPlainText(U2OpStatus& os, QPlainTextEdit* plainTextEdit, const QString &text);

    // fails if lineEdit is NULL, or lineEdit's text wasn't cleared
    static void clear(U2OpStatus& os, QPlainTextEdit* plainTextEdit);

    // fails if GTLineEdit::clear fails
   // static void pasteClipboard(U2OpStatus& os, QPlainTextEdit* plainTextEdit, PasteMethod pasteMethod = Shortcut);

    // fails if lineEdit is NULL or lineEdit text is not in lineEdit's rect
    // considering lineEdit's fontMetrics and textMargins
    //static void checkTextSize(U2OpStatus& os, QPlainTextEdit* plainTextEdit);
};

}

#endif // _U2_GT_PLAINTEXTEDIT_H_
