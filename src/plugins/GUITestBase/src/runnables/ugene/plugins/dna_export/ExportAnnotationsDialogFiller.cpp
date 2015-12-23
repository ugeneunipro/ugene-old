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

#include "ExportAnnotationsDialogFiller.h"
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
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ExportAnnotationsFiller"
ExportAnnotationsFiller::ExportAnnotationsFiller(const QString &exportToFile, fileFormat format, HI::GUITestOpStatus &os)
: Filler(os, "U2__ExportAnnotationsDialog"), softMode(true), format(format), saveSequencesUnderAnnotations(false), saveSequenceNames(false), useMethod(GTGlobals::UseMouse)
{
    init(exportToFile);
}


ExportAnnotationsFiller::ExportAnnotationsFiller(HI::GUITestOpStatus &_os, const QString &_exportToFile, fileFormat _format, bool _saveSequencesUnderAnnotations,
    bool _saveSequenceNames, GTGlobals::UseMethod method):
Filler(_os, "U2__ExportAnnotationsDialog"), softMode(false), format(_format), saveSequencesUnderAnnotations(_saveSequencesUnderAnnotations), saveSequenceNames(_saveSequenceNames), useMethod(method)
{
    init(_exportToFile);
}

void ExportAnnotationsFiller::init(const QString &exportToFile) {
    this->exportToFile = QDir::cleanPath(QDir::currentPath() + "/" + exportToFile);

    comboBoxItems[bed] = "bed";
    comboBoxItems[genbank] = "genbank";
    comboBoxItems[gff] = "gff";
    comboBoxItems[gtf] = "gtf";
    comboBoxItems[csv] = "csv";
}

#define GT_METHOD_NAME "commonScenario"
void ExportAnnotationsFiller::commonScenario()
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
        GTComboBox::setCurrentIndex(os, comboBox, index, true, useMethod);
    }

    if (!softMode) {
        QCheckBox *checkButton = dialog->findChild<QCheckBox*>(QString::fromUtf8("exportSequenceCheck"));
        GT_CHECK(checkButton != NULL, "Check box not found");
        GTCheckBox::setChecked(os, checkButton, saveSequencesUnderAnnotations);

        checkButton = dialog->findChild<QCheckBox*>(QString::fromUtf8("exportSequenceNameCheck"));
        GT_CHECK(checkButton != NULL, "Check box not found");
        GTCheckBox::setChecked(os, checkButton, saveSequenceNames);
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
