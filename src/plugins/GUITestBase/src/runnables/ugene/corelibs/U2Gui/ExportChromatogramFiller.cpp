/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "ExportChromatogramFiller.h"
#include "api/GTWidget.h"
#include "api/GTCheckBox.h"
#include "api/GTComboBox.h"
#include "api/GTLineEdit.h"

#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QPushButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ExportChromatogramFiller"
ExportChromatogramFiller::ExportChromatogramFiller(U2OpStatus &_os, const QString &_path, const QString &_name,
                                ExportChromatogramFiller::FormatToUse _format,  bool reversed, bool complement, 
                                bool addDocumentToProject,GTGlobals::UseMethod method):
Filler(_os, "ExportChromatogramDialog"), name(_name), useMethod(method), format(_format), reversed(reversed), 
       complement(complement), addDocumentToProject(addDocumentToProject) {
    path = _path;
    comboBoxItems[SCF] = "SCF";
}

#define GT_METHOD_NAME "run"
void ExportChromatogramFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>();
    GT_CHECK(lineEdit != NULL, "line edit not found");

    GTLineEdit::setText(os, lineEdit, path + name);

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");

    int index = comboBox->findText(comboBoxItems[format]);
    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    GTComboBox::setCurrentIndex(os, comboBox, index);

    if (addDocumentToProject)
       {
        QCheckBox *checkBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("addToProjectBox"));
        GTCheckBox::setChecked(os, checkBox, addDocumentToProject);
       }

    if (reversed)
      {
        QCheckBox *checkBoxReversed = dialog->findChild<QCheckBox*>(QString::fromUtf8("reverseBox"));
        GTCheckBox::setChecked(os, checkBoxReversed, reversed);
      }

    if (complement)
      {
        QCheckBox *checkBoxComplement = dialog->findChild<QCheckBox*>(QString::fromUtf8("complementBox"));
        GTCheckBox::setChecked(os, checkBoxComplement, complement);
      }

    QPushButton *exportButton = dialog->findChild<QPushButton*>(QString::fromUtf8("okButton"));
    GT_CHECK(exportButton != NULL, "Export button not found");

    GTWidget::click(os, exportButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
