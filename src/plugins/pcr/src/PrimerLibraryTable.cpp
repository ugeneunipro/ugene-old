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

#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "PrimerGroupBox.h"
#include "PrimerLibrary.h"
#include "PrimerStatistics.h"

#include "PrimerLibraryTable.h"

namespace U2 {

/************************************************************************/
/* PrimerLibraryModel */
/************************************************************************/
PrimerLibraryModel::PrimerLibraryModel(QObject *parent)
: QAbstractItemModel(parent)
{
    U2OpStatus2Log os;
    PrimerLibrary *primerLibrary = PrimerLibrary::getInstance(os);
    SAFE_POINT_OP(os, );
    primers = primerLibrary->getPrimers(os);
}

int PrimerLibraryModel::columnCount(const QModelIndex & /*parent*/) const {
    return 5;
}

QVariant PrimerLibraryModel::data(const QModelIndex &index, int role) const {
    CHECK(index.isValid(), QVariant());
    CHECK(index.row() < rowCount(index.parent()), QVariant());

    if (Qt::DisplayRole == role) {
        return displayData(index);
    }

    return QVariant();
}

QVariant PrimerLibraryModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const {
    CHECK(Qt::DisplayRole == role, QVariant());

    switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("GC-content (%)");
        case 2:
            return PrimersPairStatistics::TmString;
        case 3:
            return tr("Length (bp)");
        case 4:
            return tr("Sequence");
        default:
            return QVariant();
    }
}

QModelIndex PrimerLibraryModel::index(int row, int column, const QModelIndex & /*parent*/) const {
    CHECK(row < primers.size(), QModelIndex());
    return createIndex(row, column);
}

QModelIndex PrimerLibraryModel::parent(const QModelIndex & /*index*/) const {
    return QModelIndex();
}

int PrimerLibraryModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return primers.size();
}

Primer PrimerLibraryModel::getPrimer(const QModelIndex &index, U2OpStatus &os) const {
    CHECK_EXT(index.row() >= 0 && index.row() < primers.size(), os.setError(L10N::internalError("Incorrect primer number")), Primer());
    return primers.at(index.row());
}

void PrimerLibraryModel::addPrimer(Primer &primer, U2OpStatus &os) {
    PrimerLibrary *primerLibrary = PrimerLibrary::getInstance(os);
    CHECK_OP(os, );

    // Append statistics
    PrimerStatisticsCalculator calc(primer.sequence.toLocal8Bit());
    primer.gc = calc.getGC();
    primer.tm = calc.getTm();

    primerLibrary->addPrimer(primer, os);
    CHECK_OP(os, );

    beginInsertRows(QModelIndex(), primers.size(), primers.size());
    primers << primer;
    endInsertRows();
}

void PrimerLibraryModel::removePrimer(const QModelIndex &index, U2OpStatus &os) {
    PrimerLibrary *primerLibrary = PrimerLibrary::getInstance(os);
    CHECK_OP(os, );

    SAFE_POINT_EXT(index.row() >= 0 && index.row() < primers.size(), os.setError(tr("Incorrect primer number")), );
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    Primer primer = primers.takeAt(index.row());
    primerLibrary->removePrimer(primer, os);
    endRemoveRows();
}

QVariant PrimerLibraryModel::displayData(const QModelIndex &index) const {
    Primer primer = primers[index.row()];
    switch (index.column()) {
        case 0:
            return primer.name;
        case 1:
            return PrimerGroupBox::getDoubleStringValue(primer.gc);
        case 2:
            return PrimerGroupBox::getDoubleStringValue(primer.tm);
        case 3:
            return primer.sequence.length();
        case 4:
            return primer.sequence;
        default:
            return QVariant();
    }
}

/************************************************************************/
/* PrimerLibraryTable */
/************************************************************************/
PrimerLibraryTable::PrimerLibraryTable(QWidget *parent)
: QTableView(parent)
{
    model = new PrimerLibraryModel(this);
    setModel(model);
}

QList<Primer> PrimerLibraryTable::getSelection() const {
    QList<Primer> result;

    QModelIndexList selection = selectionModel()->selectedIndexes();
    foreach (const QModelIndex &index, selection) {
        if (index.column() > 0) {
            continue;
        }
        U2OpStatusImpl os;
        Primer primer = model->getPrimer(index, os);
        SAFE_POINT_OP(os, result);
        result << primer;
    }
    return result;
}

void PrimerLibraryTable::addPrimer(Primer &primer, U2OpStatus &os) {
    model->addPrimer(primer, os);
}

void PrimerLibraryTable::removePrimer(const QModelIndex &index, U2OpStatus &os) {
    model->removePrimer(index, os);
}

} // U2
