/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QTableView>

#include "api/GTLineEdit.h"
#include "api/GTSpinBox.h"
#include "api/GTWidget.h"

#include "GTUtilsPcr.h"

namespace U2 {

void GTUtilsPcr::setPrimer(U2OpStatus &os, U2Strand::Direction direction, const QByteArray &primer) {
    QString boxName = "forwardPrimerBox";
    if (U2Strand::Complementary == direction) {
        boxName = "reversePrimerBox";
    }
    QWidget *primerBox = GTWidget::findWidget(os, boxName);
    QLineEdit *primerEdit = dynamic_cast<QLineEdit*>(GTWidget::findWidget(os, "primerEdit", primerBox));
    GTLineEdit::setText(os, primerEdit, primer, true);
}

void GTUtilsPcr::setMismatches(U2OpStatus &os, U2Strand::Direction direction, int mismatches) {
    QString boxName = "forwardPrimerBox";
    if (U2Strand::Complementary == direction) {
        boxName = "reversePrimerBox";
    }
    QWidget *primerBox = GTWidget::findWidget(os, boxName);
    QSpinBox *mismatchesSpinBox = dynamic_cast<QSpinBox*>(GTWidget::findWidget(os, "mismatchesSpinBox", primerBox));
    GTSpinBox::setValue(os, mismatchesSpinBox, mismatches);
}

int GTUtilsPcr::productsCount(U2OpStatus &os) {
    return table(os)->model()->rowCount(QModelIndex());
}

QString GTUtilsPcr::getResultRegion(U2OpStatus &os, int number) {
    QModelIndex productIdx = table(os)->model()->index(number, 0);
    return table(os)->model()->data(productIdx, Qt::DisplayRole).toString();
}

QPoint GTUtilsPcr::getResultPoint(U2OpStatus &os, int number) {
    QModelIndex productIdx = table(os)->model()->index(number, 0);
    QRect productRect = table(os)->visualRect(productIdx);
    return content(os)->mapToGlobal(productRect.center());
}

QPoint GTUtilsPcr::getDetailsPoint(U2OpStatus &os) {
    QWidget *warning = GTWidget::findWidget(os, "warningLabel");
    QPoint result = warning->geometry().center();
    result.setY(result.y()+3);
    return warning->parentWidget()->mapToGlobal(result);
}

QTableView * GTUtilsPcr::table(U2OpStatus &os) {
    return dynamic_cast<QTableView*>(GTWidget::findWidget(os, "productsTable"));
}

QWidget * GTUtilsPcr::content(U2OpStatus &os) {
    return GTWidget::findWidget(os, "qt_scrollarea_viewport", table(os));
}

} // U2
