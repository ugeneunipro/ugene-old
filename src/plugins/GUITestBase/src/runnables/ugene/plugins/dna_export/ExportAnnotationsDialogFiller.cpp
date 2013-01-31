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

#include "ExportAnnotationsDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTComboBox.h"
#include "api/GTCheckBox.h"

#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ExportAnnotationsFiller"
ExportAnnotationsFiller::ExportAnnotationsFiller(U2OpStatus &_os, const QString &_exportToFile, fileFormat _format, bool _saveSequencesUnderAnnotations,
    bool _saveSequenceNames, GTGlobals::UseMethod method):
Filler(_os, "U2__ExportAnnotationsDialog"), format(_format), saveSequencesUnderAnnotations(_saveSequencesUnderAnnotations), saveSequenceNames(_saveSequenceNames), useMethod(method)
{
    QString __exportToFile = QDir::cleanPath(QDir::currentPath() + "/" + _exportToFile);
    exportToFile = __exportToFile;

    comboBoxItems[genbank] = "genbank";
    comboBoxItems[gff] = "gff";
    comboBoxItems[csv] = "csv";
}

#define GT_METHOD_NAME "run"
void ExportAnnotationsFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("fileNameEdit");
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEdit, exportToFile);

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");

    int index = comboBox->findText(comboBoxItems[format]);
    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    if (comboBox->currentIndex() != index){
        GTComboBox::setCurrentIndex(os, comboBox, index);
    }

    QCheckBox *checkButton = dialog->findChild<QCheckBox*>(QString::fromUtf8("exportSequenceCheck"));
    GT_CHECK(checkButton != NULL, "Check box not found");
    GTCheckBox::setChecked(os, checkButton, saveSequencesUnderAnnotations);

    checkButton = dialog->findChild<QCheckBox*>(QString::fromUtf8("exportSequenceNameCheck"));
    GT_CHECK(checkButton != NULL, "Check box not found");
    GTCheckBox::setChecked(os, checkButton, saveSequenceNames);

    QDialogButtonBox* buttonBox = dialog->findChild<QDialogButtonBox*>("buttonBox");

    GT_CHECK(buttonBox != NULL, "buttonBox is NULL");
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    GT_CHECK(okButton != NULL, "okButton is NULL");
    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
