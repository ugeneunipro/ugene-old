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

#include "ExportSequencesDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTCheckBox.h"
#include "api/GTRadioButton.h"
#include "api/GTSpinBox.h"

#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QPushButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ExportSelectedRegionFiller"
ExportSelectedRegionFiller::ExportSelectedRegionFiller(U2OpStatus &_os, const QString &_path, const QString &_name, GTGlobals::UseMethod method):
Filler(_os, "U2__ExportSequencesDialog"), name(_name), useMethod(method) {
    QString __path = QDir::cleanPath(QDir::currentPath() + "/" + _path);
    if (__path.at(__path.count() - 1) != '/') {
        __path += '/';
    }

    path = __path;
}

#define GT_METHOD_NAME "run"
void ExportSelectedRegionFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "fileNameEdit", dialog));
    GT_CHECK(lineEdit != NULL, "File name line edit not found");
    GTLineEdit::setText(os, lineEdit, path + name);

    QPushButton *okButton = dialog->findChild<QPushButton*>(QString::fromUtf8("exportButton"));
    GT_CHECK(okButton != NULL, "Export button not found");
    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::exportSequenceOfSelectedAnnotationsFiller"
ExportSequenceOfSelectedAnnotationsFiller::ExportSequenceOfSelectedAnnotationsFiller(U2OpStatus &_os, const QString &_path, FormatToUse _format, MergeOptions _options, int _gapLength,
    bool _addDocToProject, bool _exportWithAnnotations, GTGlobals::UseMethod method):
    Filler(_os, "U2__ExportSequencesDialog"), gapLength(_gapLength), format(_format), addToProject(_addDocToProject), exportWithAnnotations(false),
    options(_options), useMethod(method)
{
    exportWithAnnotations = _exportWithAnnotations;
    QString __path = QDir::cleanPath(QDir::currentPath() + "/" + _path);
    // no needs to add '/' so _path includes file name
    /*if (__path.at(__path.count() - 1) != '/') {
    __path += '/';
    }*/
    path = __path;

    comboBoxItems[Fasta] = "FASTA";
    comboBoxItems[Fastaq] = "FASTAQ";
    comboBoxItems[Gff] = "GFF";
    comboBoxItems[Genbank] = "Genbank";

    mergeRadioButtons[SaveAsSeparate] = "separateButton";
    mergeRadioButtons[Merge] = "mergeButton";
}

#define GT_METHOD_NAME "run"
void ExportSequenceOfSelectedAnnotationsFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *fileNameEdit = dialog->findChild<QLineEdit*>("fileNameEdit");
    GT_CHECK(fileNameEdit != NULL, "fileNameEdit not found");
    GTLineEdit::setText(os, fileNameEdit, path);

    GTGlobals::sleep(200);

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");
    int index = comboBox->findText(comboBoxItems[format]);

    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    if (comboBox->currentIndex() != index){
        GTComboBox::setCurrentIndex(os, comboBox, index);
    }

    GTGlobals::sleep(200);

    QCheckBox *projectCheckBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("addToProjectBox"));
    GT_CHECK(projectCheckBox != NULL, "addToProjectBox not found");
    GTCheckBox::setChecked(os, projectCheckBox, addToProject);

    GTGlobals::sleep(200);

    QCheckBox *annotationsCheckBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("withAnnotationsBox"));
    GT_CHECK(annotationsCheckBox != NULL, "Check box not found");
    if(annotationsCheckBox->isEnabled()){
        GTCheckBox::setChecked(os, annotationsCheckBox, exportWithAnnotations);
    }

    GTGlobals::sleep(200);

    QRadioButton *mergeButton = dialog->findChild<QRadioButton*>(mergeRadioButtons[options]);

    GT_CHECK(mergeButton != NULL, "Radio button " + mergeRadioButtons[options] + " not found");
    if (mergeButton->isEnabled()){
        GTRadioButton::click(os, mergeButton);
    }

    GTGlobals::sleep(200);

    if (gapLength){
        QSpinBox *mergeSpinBox = dialog->findChild<QSpinBox*>("mergeSpinBox");
        GT_CHECK(mergeSpinBox != NULL, "SpinBox not found");
        GTSpinBox::setValue(os, mergeSpinBox, gapLength);
    }

    GTGlobals::sleep(200);

    QPushButton *exportButton = dialog->findChild<QPushButton*>(QString::fromUtf8("exportButton"));
    GT_CHECK(exportButton != NULL, "Export button not found");
    GTWidget::click(os, exportButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
