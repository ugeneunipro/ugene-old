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

#include "InSilicoPcrTask.h"
#include "PrimerGroupBox.h"

#include "InSilicoPcrProductsTable.h"

namespace U2 {

InSilicoPcrProductsTable::InSilicoPcrProductsTable(QWidget *parent)
: QTableWidget(parent)
{

}

void InSilicoPcrProductsTable::showProducts(InSilicoPcrTask *task, ADVSequenceObjectContext *sequenceContext) {
    if (NULL != this->sequenceContext) {
        this->sequenceContext->disconnect(this);
    }
    this->sequenceContext = sequenceContext;
    setRowCount(task->getResults().size());
    int row = 0;
    foreach (const InSilicoPcrProduct &result, task->getResults()) {
        qint64 startPos = result.region.startPos + 1;
        qint64 endPos = result.region.endPos() % task->getSettings().sequence.length();

        setItem(row, 0, new QTableWidgetItem(QString("%1 - %2").arg(startPos).arg(endPos)));
        setItem(row, 1, new QTableWidgetItem(QString::number(result.region.length)));
        setItem(row, 2, new QTableWidgetItem(PrimerGroupBox::getDoubleStringValue(result.ta)));

        row++;
    }
}

} // U2
