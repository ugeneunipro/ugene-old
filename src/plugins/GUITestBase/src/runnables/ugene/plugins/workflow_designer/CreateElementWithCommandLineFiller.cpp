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

#include "CreateElementWithCommandLineToolFiller.h"

#include <api/GTTextEdit.h>

namespace U2 {

#define GT_CLASS_NAME "CreateElementWithCommandLineFiller"

CreateElementWithCommandLineToolFiller::CreateElementWithCommandLineToolFiller(U2OpStatus& os,
                                                                               const ElementWithCommandLineSettings& settings)
    : Filler(os, "CreateExternalProcessWorkerDialog"),
      settings(settings)
{}

#define GT_METHOD_NAME "run"
void CreateElementWithCommandLineToolFiller::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    // page 1
    QLineEdit* nameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameLineEdit", dialog));
    GT_CHECK(nameEdit, "nameLineEdit not found");
    GTLineEdit::setText(os, nameEdit, settings.elementName);
    GTGlobals::sleep();

    QWidget* nextButton = GTWidget::findWidget(os, "__qt__passive_wizardbutton1", dialog);
    GTWidget::click(os, nextButton);

    // page 2
    QWidget* addInputButton = GTWidget::findWidget(os, "addInputButton", dialog);
    GT_CHECK(addInputButton != NULL, "addInputButton not found");
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"inputTableView"));
    GT_CHECK(table != NULL, "inputTable not found");
    fillTheTable(table, addInputButton, settings.input);

    QWidget* addOutputButton = GTWidget::findWidget(os, "addOutputButton", dialog);
    GT_CHECK(addOutputButton != NULL, "addOutputButton not found");
    table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"outputTableView"));
    GT_CHECK(table != NULL, "outputTable not found");
    fillTheTable(table, addOutputButton, settings.output);

    nextButton = GTWidget::findWidget(os, "__qt__passive_wizardbutton1", dialog);
    GTWidget::click(os, nextButton);

    // page 3
    QWidget* addAttributeButton = GTWidget::findWidget(os, "addAttributeButton", dialog);
    GT_CHECK(addAttributeButton != NULL, "addAttributeButton not found");
    table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"attributesTableView"));
    GT_CHECK(table != NULL, "attributesTable not found");
    fillTheTable(table, addAttributeButton, settings.parameters);

    nextButton = GTWidget::findWidget(os, "__qt__passive_wizardbutton1", dialog);
    GTWidget::click(os, nextButton);

    // page 4
    QLineEdit* execLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "templateLineEdit", dialog));
    GT_CHECK(nameEdit, "templateLineEdit not found");
    GTLineEdit::setText(os, execLineEdit, settings.executionString);

    QTextEdit* prompterTextEdit = qobject_cast<QTextEdit*>(GTWidget::findWidget(os, "prompterTextEdit", dialog));
    GT_CHECK(prompterTextEdit, "templateLineEdit not found");
    GTTextEdit::setText(os, prompterTextEdit, settings.parameterizedDescription);

    QWidget* finishButton = GTWidget::findButtonByText(os, "Finish");
    GTWidget::click(os, finishButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

QString CreateElementWithCommandLineToolFiller::dataTypeToString(const InOutType &type) const {
    switch (type) {
    case MultipleAlignment:
        return "Multiple Alignment";
    case Sequence:
        return "Sequence";
    case SequenceWithAnnotations:
        return "Sequence with annotations";
    case SetOfAnnotations:
        return "Set of annotations";
    case DataString:
        return "String";
    default:
        return QString();
    }
}

QString CreateElementWithCommandLineToolFiller::dataTypeToString(const ParameterType &type) const {
    switch (type) {
    case Boolean:
        return "Boolean";
    case Number:
        return "Number";
    case ParameterString:
        return "String";
    case URL:
        return "URL";
    default:
        return QString();
    }
}

void CreateElementWithCommandLineToolFiller::processDataType(QTableView *table, int row, const InOutDataType &type) {
    setType(table, row, type.first);
    {
        GTMouseDriver::moveTo(os, GTTableView::getCellPosition(os, table, 2, row));
        GTMouseDriver::doubleClick(os);

        QComboBox* box = qobject_cast<QComboBox*>(QApplication::focusWidget());
        GTComboBox::setIndexWithText(os, box, type.second);
#ifdef Q_OS_WIN
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
#endif
    }
}

void CreateElementWithCommandLineToolFiller::processDataType(QTableView *table, int row, const ParameterType &type) {
    setType(table, row, type);
}

}
