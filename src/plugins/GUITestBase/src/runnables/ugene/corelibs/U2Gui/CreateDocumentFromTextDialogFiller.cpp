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

#include "CreateDocumentFromTextDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTPlainTextEdit.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTCheckBox.h"

#include <QtCore/QDir>
#include <QtGui/QPushButton>
#include <QtGui/QApplication>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::createDocumentFiller"
CreateDocumentFiller::CreateDocumentFiller(U2OpStatus &_os, const QString &_pasteDataHere, const QString &_documentLocation,
    documentFormat _format, const QString &_sequenceName, bool saveFile, GTGlobals::UseMethod method):
Filler(_os, "CreateDocumentFromTextDialog"), format(_format), useMethod(method)
{
    sequenceName = _sequenceName;
    pasteDataHere = _pasteDataHere;
    QString __documentLocation = QDir::cleanPath(QDir::currentPath() + "/" + _documentLocation);
    // + "/" + _sequenceName
    documentLocation = __documentLocation;
    comboBoxItems[FASTA] = "FASTA";
    comboBoxItems[Genbank] = "Genbank";
}

#define GT_METHOD_NAME "run"
void CreateDocumentFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QPlainTextEdit *plainText = dialog->findChild<QPlainTextEdit*>("sequenceEdit");
    GT_CHECK(plainText != NULL, "plain text not found");
    GTPlainTextEdit::setPlainText(os, plainText, pasteDataHere);

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("filepathEdit");
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEdit, documentLocation);

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");

    int index = comboBox->findText(comboBoxItems[format]);
    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    if (comboBox->currentIndex() != index){
        GTComboBox::setCurrentIndex(os, comboBox, index);
    }

    QLineEdit *lineEditName = dialog->findChild<QLineEdit*>("nameEdit");
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEditName, sequenceName);

    QCheckBox* saveFileCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "saveImmediatelyBox", dialog));
    GTCheckBox::setChecked(os, saveFileCheckBox);

    QPushButton *createButton = dialog->findChild<QPushButton*>(QString::fromUtf8("OKButton"));
    GT_CHECK(createButton != NULL, "Create button not found");

    GTWidget::click(os, createButton);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
