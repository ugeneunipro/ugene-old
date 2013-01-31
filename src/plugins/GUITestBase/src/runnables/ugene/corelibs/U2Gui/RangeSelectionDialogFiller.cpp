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

#include "RangeSelectionDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTRadioButton.h"

#include <QtGui/QApplication>
#include <QtGui/QToolButton>
#include <QtGui/QPushButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::exportSequenceOfSelectedAnnotationsFiller"
selectSequenceRegionDialogFiller::selectSequenceRegionDialogFiller(U2OpStatus &_os, int *_len) : Filler(_os, "RangeSelectionDialog")
{
    rangeType = Single;
    selectAll = true;
    fromBegin = false;
    minVal = 0;
    maxVal = 0;
    length = 0;
    len = _len;
    multipleRange = QString();
}

selectSequenceRegionDialogFiller::selectSequenceRegionDialogFiller(U2OpStatus &_os) : Filler(_os, "RangeSelectionDialog")
{
    rangeType = Single;
    selectAll = true;
    fromBegin = false;
    minVal = 0;
    maxVal = 0;
    length = 0;
    len = NULL;
    multipleRange = QString();
}

selectSequenceRegionDialogFiller::selectSequenceRegionDialogFiller(U2OpStatus &_os, int _minVal, int _maxVal) : Filler(_os, "RangeSelectionDialog")
{
    rangeType = Single;
    selectAll = false;
    fromBegin = false;
    minVal = _minVal;
    maxVal = _maxVal;
    length = 0;
    len = NULL;
    multipleRange = QString();
}

selectSequenceRegionDialogFiller::selectSequenceRegionDialogFiller(U2OpStatus &_os, const QString &range) : Filler(_os, "RangeSelectionDialog")
{
    rangeType = Multiple;
    selectAll = false;
    fromBegin = false;
    minVal = 0;
    maxVal = 0;
    length = 0;
    len = NULL;
    multipleRange = range;
}

selectSequenceRegionDialogFiller::selectSequenceRegionDialogFiller(U2OpStatus &_os, int _length, bool selectFromBegin) : Filler(_os, "RangeSelectionDialog")
{
    rangeType = Single;
    selectAll = false;
    fromBegin = selectFromBegin;
    minVal = 0;
    maxVal = 0;
    length = _length;
    len = NULL;
    multipleRange = QString();
}

#define GT_METHOD_NAME "run"
void selectSequenceRegionDialogFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    if (selectAll) {
        QToolButton *min = dialog->findChild<QToolButton*>("minButton");
        QToolButton *max = dialog->findChild<QToolButton*>("maxButton");
        GT_CHECK(min != NULL, "Min button not found");
        GT_CHECK(max != NULL, "Max button not found");

        GTWidget::click(os, min);
        GTGlobals::sleep(500);
        GTWidget::click(os, max);
        GTGlobals::sleep(500);

        if (len != NULL) {
            QLineEdit *endEdit = dialog->findChild<QLineEdit*>("endEdit");
            GT_CHECK(endEdit != NULL, "QLineEdit \"endEdit\" not found");
            *len = endEdit->text().toInt();
        }
    } else if (rangeType == Single) {
        GT_CHECK(minVal <= maxVal, "Value \"min\" greater then \"max\"");

        QLineEdit *startEdit = dialog->findChild<QLineEdit*>("startEdit");
        QLineEdit *endEdit = dialog->findChild<QLineEdit*>("endEdit");
        GT_CHECK(startEdit != NULL, "QLineEdit \"startEdit\" not found");
        GT_CHECK(endEdit != NULL, "QLineEdit \"endEdit\" not found");

        if (length == 0) {
            GTLineEdit::setText(os, startEdit, QString::number(minVal));
            GTLineEdit::setText(os, endEdit, QString::number(maxVal));
        } else {
            int min = startEdit->text().toInt();
            int max = endEdit->text().toInt();
            GT_CHECK(max - min >= length, "Invalid argument \"length\"");

            if (fromBegin) {
                GTLineEdit::setText(os, startEdit, QString::number(1));
                GTLineEdit::setText(os, endEdit, QString::number(length));
            } else {
                GTLineEdit::setText(os, startEdit, QString::number(max - length + 1));
                GTLineEdit::setText(os, endEdit, QString::number(max));
            }
        }
    } else {
        GT_CHECK(! multipleRange.isEmpty(), "Range is empty");

        QRadioButton *multipleButton = dialog->findChild<QRadioButton*>("miltipleButton");
        GT_CHECK(multipleButton != NULL, "RadioButton \"miltipleButton\" not found");
        GTRadioButton::click(os, multipleButton);

        QLineEdit *regionEdit = dialog->findChild<QLineEdit*>("multipleRegionEdit");
        GT_CHECK(regionEdit != NULL, "QLineEdit \"multipleRegionEdit\" not foud");
        GTLineEdit::setText(os, regionEdit, multipleRange);
    }

    QPushButton *okButton = dialog->findChild<QPushButton*>("okButton");
    GT_CHECK(okButton != NULL, "OK button not found");
    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
