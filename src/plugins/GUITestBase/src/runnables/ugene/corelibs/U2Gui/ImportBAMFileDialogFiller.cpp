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

#include "ImportBAMFileDialogFiller.h"
#include "primitives/GTWidget.h"
#include "primitives/GTSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "api/GTFileDialog.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ImportBAMFileFiller"
#define GT_METHOD_NAME "run"
ImportBAMFileFiller::ImportBAMFileFiller(U2OpStatus &os, const QString destinationUrl,
                                         const QString referenceFolderPath, const QString referenceFileName,
                                         bool importUnmappedReads,
                                         int timeoutMs) :
    Filler(os, "Import BAM File"),
    referenceFolderPath(referenceFolderPath),
    referenceFileName(referenceFileName),
    destinationUrl(destinationUrl),
    importUnmappedReads(importUnmappedReads) {
    settings.timeout = timeoutMs;
}

ImportBAMFileFiller::ImportBAMFileFiller(U2OpStatus &os, CustomScenario* _c):Filler(os, "Import BAM File", _c),
    referenceFolderPath(""),
    referenceFileName(""),
    destinationUrl(""),
    importUnmappedReads(false) {
    settings.timeout = 120000;
}

void ImportBAMFileFiller::commonScenario() {

    GTGlobals::sleep(500);
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (!referenceFolderPath.isEmpty()) {
        GTFileDialogUtils *ob = new GTFileDialogUtils(os, referenceFolderPath, referenceFileName);
        GTUtilsDialog::waitForDialog(os, ob);
        GTWidget::click(os, GTWidget::findWidget(os,"refUrlButton",dialog));
        }

    if (!destinationUrl.isEmpty()){
        QLineEdit* destinationUrlEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "destinationUrlEdit", dialog));
        GT_CHECK(destinationUrlEdit, "destinationUrlEdit not found");
        GTLineEdit::setText(os, destinationUrlEdit, destinationUrl);
    }

    QCheckBox* importUnmapped = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "importUnmappedBox", dialog));
    GT_CHECK(importUnmapped, "ImportUnmappedReads checkbox is NULL");
    if (importUnmapped->isChecked() != importUnmappedReads) {
        GTCheckBox::setChecked(os, importUnmapped, importUnmapped);
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
