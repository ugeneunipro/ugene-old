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

#include "Primer3DialogFiller.h"
#include "api/GTLineEdit.h"
#include "api/GTSpinBox.h"
#include "api/GTTabWidget.h"
#include "api/GTWidget.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::KalignDialogFiller"
Primer3DialogFiller::Primer3DialogFiller(U2OpStatus &os, const Primer3Settings &settings)
    : Filler(os, "Primer3Dialog"),
      settings(settings)
{}

#define GT_METHOD_NAME "run"
void Primer3DialogFiller::commonScenario() {
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    if(settings.resultsCount != -1){
        QSpinBox* resultsCountSpinBox = dialog->findChild<QSpinBox*>("edit_PRIMER_NUM_RETURN");
        GTSpinBox::setValue(os, resultsCountSpinBox, settings.resultsCount);
    }

    QTabWidget *tabWidget = dialog->findChild<QTabWidget*>("tabWidget");
    if (!settings.primersGroupName.isEmpty() || !settings.primersName.isEmpty()) {
        GTTabWidget::setCurrentIndex(os, tabWidget, 6);

        if (!settings.primersGroupName.isEmpty()) {
            QLineEdit *groupEdit = dialog->findChild<QLineEdit*>("groupNameEdit");
            GT_CHECK(groupEdit != NULL, "groupNameEdit is NULL");
            GTLineEdit::setText(os, groupEdit, settings.primersGroupName);
        }

        if (!settings.primersName.isEmpty()) {
            QLineEdit *primerEdit = dialog->findChild<QLineEdit*>("annotationNameEdit");
            GT_CHECK(primerEdit != NULL, "annotationNameEdit is NULL");
            GTLineEdit::setText(os, primerEdit, settings.primersName);
        }
    }

    if (settings.start != -1) {
        QLineEdit* start = dialog->findChild<QLineEdit*>("start_edit_line");
        GT_CHECK(start != NULL, "start_edit_line is NULL");
        GTLineEdit::setText(os, start, QString::number(settings.start));
    }

    if (settings.end != -1) {
        QLineEdit* end = dialog->findChild<QLineEdit*>("end_edit_line");
        GT_CHECK(end != NULL, "start_edit_line is NULL");
        GTLineEdit::setText(os, end, QString::number(settings.end));
    }

    QPushButton* button = dialog->findChild<QPushButton*>("pickPrimersButton");
    GT_CHECK(button !=NULL, "PickPrimers button is NULL");
    GTWidget::click(os, button);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

} // namespace
