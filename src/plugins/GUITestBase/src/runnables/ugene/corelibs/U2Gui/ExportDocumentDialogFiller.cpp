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

#include "ExportDocumentDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTCheckBox.h"

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

QString ExportDocumentDialogFiller::defaultExportString = "";

#define GT_CLASS_NAME "ExportDocumentDialogFiller"
ExportDocumentDialogFiller::ExportDocumentDialogFiller(U2OpStatus &_os, const QString &_path, const QString &_name, 
                         ExportDocumentDialogFiller::FormatToUse _format, bool compressFile,
                         bool addToProject, GTGlobals::UseMethod method):
Filler(_os, "ExportDocumentDialog"),
path(_path), name(_name), useMethod(method), format(_format), compressFile(compressFile), addToProject(addToProject) {
    QString __path = QDir::cleanPath(QDir::currentPath() + "/" + _path);
    if (__path.at(__path.count() - 1) != '/') {
        __path += '/';
    }

    if (!path.isEmpty()) {
        path = __path;
    }

    comboBoxItems[Genbank] = "Genbank";
    comboBoxItems[GFF] = "GFF";
    comboBoxItems[CLUSTALW] = "CLUSTALW";
    comboBoxItems[BAM] = "BAM File";
    comboBoxItems[VCF] = "VCFv4";
    comboBoxItems[TEXT] = "Plain text";
    comboBoxItems[NWK] = "Newick Standard";
    comboBoxItems[FASTA] = "FASTA";
}

#define GT_METHOD_NAME "run"
void ExportDocumentDialogFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    if (!path.isEmpty()) {
        QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("fileNameEdit");
        GT_CHECK(lineEdit != NULL, "line edit not found");
        defaultExportString = GTLineEdit::copyText(os, lineEdit);
        GTLineEdit::setText(os, lineEdit, path + name);

        QComboBox *comboBox = dialog->findChild<QComboBox*>("formatCombo");
        GT_CHECK(comboBox != NULL, "ComboBox not found");
        int index = comboBox->findText(comboBoxItems[format]);

        GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
        if (comboBox->currentIndex() != index){
            GTComboBox::setCurrentIndex(os, comboBox, index);
        }

        QCheckBox *compressCheckBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("compressCheck"));
        GT_CHECK(compressCheckBox != NULL, "Check box not found");
        GTCheckBox::setChecked(os, compressCheckBox, compressFile);

        QCheckBox *addCheckBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("addToProjCheck"));
        GT_CHECK(addCheckBox != NULL, "Check box not found");
        GTCheckBox::setChecked(os, addCheckBox, addToProject);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "cancel button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
