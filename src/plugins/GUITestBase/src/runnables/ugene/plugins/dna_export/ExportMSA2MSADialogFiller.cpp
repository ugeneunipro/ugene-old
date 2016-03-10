/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QPushButton>

#include <primitives/GTComboBox.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTRadioButton.h>
#include <primitives/GTWidget.h>

#include "ExportMSA2MSADialogFiller.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ExportToSequenceFormatFiller"

ExportMSA2MSADialogFiller::ExportMSA2MSADialogFiller(GUITestOpStatus &os, const QString &formatName, const QString &path)
    : Filler(os, "U2__ExportMSA2MSADialog"),
      formatName(formatName),
      path(path) {

}

#define GT_METHOD_NAME "commonScenario"
void ExportMSA2MSADialogFiller::commonScenario() {
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    if (!path.isEmpty()) {
        GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "fileNameEdit", dialog), path);
    }

    if (!formatName.isEmpty()) {
        GTComboBox::setIndexWithText(os, GTWidget::findExactWidget<QComboBox *>(os, "formatCombo", dialog), formatName, true, GTGlobals::UseMouse);
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
