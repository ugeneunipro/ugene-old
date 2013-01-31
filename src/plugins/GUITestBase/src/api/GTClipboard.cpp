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

#include "GTClipboard.h"

#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtGui/QApplication>

namespace U2 {

#define GT_CLASS_NAME "GTClipboard"

#define GT_METHOD_NAME "text"
QString GTClipboard::text(U2OpStatus &os) {

// check that clipboard contains text
    QClipboard *clipboard = QApplication::clipboard();
    GT_CHECK_RESULT(clipboard != NULL, "Clipboard is NULL", "");
    const QMimeData *mimeData = clipboard->mimeData();
    GT_CHECK_RESULT(mimeData != NULL, "Clipboard MimeData is NULL", "");

    GT_CHECK_RESULT(mimeData->hasText() == true, "Clipboard doesn't contain text data", "");
    QString clipboardText = mimeData->text();
// need to clear clipboard, UGENE will crash on close otherwise because of Qt assert
    clipboard->clear();

    return clipboardText;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} //namespace
