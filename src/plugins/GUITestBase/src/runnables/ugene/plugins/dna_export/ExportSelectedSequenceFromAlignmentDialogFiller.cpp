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

#include "ExportSelectedSequenceFromAlignmentDialogFiller.h"
#include <primitives/GTWidget.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTRadioButton.h>
#include <primitives/GTComboBox.h>
#include <primitives/GTCheckBox.h>

#include <QtCore/QDir>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#endif


namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::exportSelectedSequenceFromAlignment"

ExportSelectedSequenceFromAlignment::ExportSelectedSequenceFromAlignment(HI::GUITestOpStatus &_os, const QString &_path,
                                                                         documentFormat _format, bool _keepGaps, bool _addToProj):
    Filler(_os, "U2__SaveSelectedSequenceFromMSADialog"),
    path(_path),
    format(_format),
    keepGaps(_keepGaps),
    addToProj(_addToProj)
{
    comboBoxItems[EMBL] = "EMBL";
    comboBoxItems[FASTA] = "FASTA";
    comboBoxItems[FASTQ] = "FASTQ";
    comboBoxItems[GFF] = "GFF";
    comboBoxItems[Genbank] = "Genbank";
    comboBoxItems[Swiss_Prot] = "Swiss_Prot";

}

ExportSelectedSequenceFromAlignment::ExportSelectedSequenceFromAlignment(HI::GUITestOpStatus &os, CustomScenario *scenario)
    : Filler(os, "U2__SaveSelectedSequenceFromMSADialog", scenario),
      format(EMBL),
      keepGaps(false),
      addToProj(false)
{

}

#define GT_METHOD_NAME "run"
void ExportSelectedSequenceFromAlignment::commonScenario()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>();
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEdit, path);

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");

    int index = comboBox->findText(comboBoxItems[format]);
    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    GTComboBox::setCurrentIndex(os, comboBox, index);

    QCheckBox *addToProjectBox = dialog->findChild<QCheckBox*>("addToProjectBox");
    GTCheckBox::setChecked(os, addToProjectBox, addToProj);

    if(keepGaps){
        QRadioButton* keepGapsRB = dialog->findChild<QRadioButton*>("keepGapsRB");
        GTRadioButton::click(os,keepGapsRB);
    }
    else{
        QRadioButton* trimGapsRB = dialog->findChild<QRadioButton*>("trimGapsRB");
        GTRadioButton::click(os,trimGapsRB);
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME


}
