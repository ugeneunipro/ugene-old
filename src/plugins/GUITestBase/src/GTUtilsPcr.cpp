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

#include <QTableView>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>

#include "api/GTFile.h"
#include "api/GTLineEdit.h"
#include "api/GTSpinBox.h"
#include "api/GTTableView.h"
#include "api/GTWidget.h"

#include "GTUtilsPcr.h"

namespace U2 {

void GTUtilsPcr::setPrimer(U2OpStatus &os, U2Strand::Direction direction, const QByteArray &primer) {
    QLineEdit *primerEdit = dynamic_cast<QLineEdit*>(GTWidget::findWidget(os, "primerEdit", primerBox(os, direction)));
    GTLineEdit::setText(os, primerEdit, primer, true);
}

void GTUtilsPcr::setMismatches(U2OpStatus &os, U2Strand::Direction direction, int mismatches) {
    QSpinBox *mismatchesSpinBox = dynamic_cast<QSpinBox*>(GTWidget::findWidget(os, "mismatchesSpinBox", primerBox(os, direction)));
    GTSpinBox::setValue(os, mismatchesSpinBox, mismatches);
}

void GTUtilsPcr::setPerfectMatch(U2OpStatus &os, int number) {
    QSpinBox *spinBox = dynamic_cast<QSpinBox*>(GTWidget::findWidget(os, "perfectSpinBox"));
    GTSpinBox::setValue(os, spinBox, number, GTGlobals::UseKeyBoard);
}

void GTUtilsPcr::setMaxProductSize(U2OpStatus &os, int number) {
    QSpinBox *spinBox = dynamic_cast<QSpinBox*>(GTWidget::findWidget(os, "productSizeSpinBox"));
    GTSpinBox::setValue(os, spinBox, number, GTGlobals::UseKeyBoard);
}

QWidget * GTUtilsPcr::browseButton(U2OpStatus &os, U2Strand::Direction direction) {
    return GTWidget::findWidget(os, "browseButton", primerBox(os, direction));
}

int GTUtilsPcr::productsCount(U2OpStatus &os) {
    return GTTableView::rowCount(os, table(os));
}

QString GTUtilsPcr::getResultRegion(U2OpStatus &os, int number) {
    return GTTableView::data(os, table(os), number, 0);
}

QPoint GTUtilsPcr::getResultPoint(U2OpStatus &os, int number) {
    return GTTableView::getCellPoint(os, table(os), number, 0);
}

QPoint GTUtilsPcr::getDetailsPoint(U2OpStatus &os) {
    QWidget *warning = GTWidget::findWidget(os, "detailsLinkLabel");
    QPoint result = warning->geometry().center();
    result.setX(result.x()/2);
    return warning->parentWidget()->mapToGlobal(result);
}

QWidget * GTUtilsPcr::primerBox(U2OpStatus &os, U2Strand::Direction direction) {
    QString boxName = "forwardPrimerBox";
    if (U2Strand::Complementary == direction) {
        boxName = "reversePrimerBox";
    }
    return GTWidget::findWidget(os, boxName);
}

QTableView * GTUtilsPcr::table(U2OpStatus &os) {
    return dynamic_cast<QTableView*>(GTWidget::findWidget(os, "productsTable"));
}

void GTUtilsPcr::clearPcrDir(U2OpStatus &os){
    Q_UNUSED(os);
    QString path = AppContext::getAppSettings()->getUserAppsSettings()->getDefaultDataDirPath() + "/pcr";
    GTFile::removeDir(path);
}

} // U2
