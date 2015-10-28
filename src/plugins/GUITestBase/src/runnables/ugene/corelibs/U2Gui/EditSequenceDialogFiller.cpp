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

#include "EditSequenceDialogFiller.h"
#include "drivers/GTMouseDriver.h"
#include "drivers/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTComboBox.h"
#include "api/GTCheckBox.h"
#include "api/GTPlainTextEdit.h"
#include "api/GTSpinBox.h"

#include "runnables/qt/MessageBoxFiller.h"

#include <QtCore/QDir>
#include <QApplication>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>

namespace U2 {
using namespace HI;

#define GT_CLASS_NAME "GTUtilsDialog::insertSequenceFiller"
InsertSequenceFiller::InsertSequenceFiller(U2OpStatus &_os, const QString &_pasteDataHere, RegionResolvingMode _regionResolvingMode, int _insertPosition,
    const QString &_documentLocation, documentFormat _format, bool _saveToNewFile, bool _mergeAnnotations, GTGlobals::UseMethod method,
    bool _wrongInput, bool recalculateQuals)
    : Filler(_os, "EditSequenceDialog"), pasteDataHere(_pasteDataHere), regionResolvingMode(_regionResolvingMode), insertPosition(_insertPosition),
    documentLocation(_documentLocation), format(_format), saveToNewFile(_saveToNewFile), mergeAnnotations(_mergeAnnotations),
    useMethod(method), wrongInput(_wrongInput), recalculateQuals(recalculateQuals)
{
    if (!documentLocation.isEmpty()) {
        documentLocation = QDir::cleanPath(QDir::currentPath() + "/" + documentLocation);
    }
    comboBoxItems[FASTA] = "FASTA";
    comboBoxItems[Genbank] = "Genbank";
    mergeAnnotations = _mergeAnnotations;
}

#define GT_METHOD_NAME "run"
void InsertSequenceFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QPlainTextEdit *plainText = dialog->findChild<QPlainTextEdit*>("sequenceEdit");
    GT_CHECK(plainText != NULL, "plain text not found");
    GTPlainTextEdit::setPlainText(os, plainText, pasteDataHere);

    QString radioButtonName;
    switch (regionResolvingMode) {
        case Resize:
            radioButtonName = "resizeRB";
            break;
        case Remove:
            radioButtonName = "removeRB";
            break;
        case SplitJoin:
            radioButtonName = "splitRB";
            break;
        case SplitSeparate:
            radioButtonName = "split_separateRB";
            break;
    }

    GTCheckBox::setChecked(os, GTWidget::findExactWidget<QCheckBox *>(os, "recalculateQualsCheckBox"), recalculateQuals);

    QRadioButton *regionResolvingMode = dialog->findChild<QRadioButton*>(radioButtonName);//"regionResolvingMode");
    GT_CHECK(regionResolvingMode != NULL, "regionResolvingMode not found");
    GTRadioButton::click(os, regionResolvingMode);

    QSpinBox *insertPositionSpin = dialog->findChild<QSpinBox*>("insertPositionSpin");
    GT_CHECK(insertPositionSpin != NULL, "insertPositionSpin not found");
    GTSpinBox::setValue(os, insertPositionSpin, insertPosition, GTGlobals::UseKey);

    QGroupBox *checkButton = dialog->findChild<QGroupBox*>(QString::fromUtf8("saveToAnotherBox"));
    GT_CHECK(checkButton != NULL, "Check box not found");

    if ((saveToNewFile && !checkButton->isChecked()) || (!saveToNewFile && checkButton->isChecked())) {
        QPoint checkPos;
        switch(useMethod) {
        case GTGlobals::UseMouse:
            checkPos = QPoint(checkButton->rect().left() + 12, checkButton->rect().top() + 12);
            GTMouseDriver::moveTo(os, checkButton->mapToGlobal(checkPos));
            GTMouseDriver::click(os);
            break;
        case GTGlobals::UseKey:
            GTWidget::setFocus(os, checkButton);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
            break;
        default:
            break;
        }
    }

    GTGlobals::sleep(1000);


    if (saveToNewFile) {
        QCheckBox *checkButton1 = dialog->findChild<QCheckBox*>(QString::fromUtf8("mergeAnnotationsBox"));
        GT_CHECK(checkButton1 != NULL, "Check box not found");
        GTCheckBox::setChecked(os, checkButton1, mergeAnnotations);

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
    }

    if(wrongInput){
        GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    }
    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);

}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
