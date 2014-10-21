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

#include <QMessageBox>
#include <QPushButton>

#include <U2Core/AppContext.h>
#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "AddPrimerDialog.h"
#include "PrimerLibrary.h"

#include "PrimerLibraryWidget.h"

namespace U2 {

/************************************************************************/
/* PrimerLibraryWidget */
/************************************************************************/
PrimerLibraryWidget::PrimerLibraryWidget(QWidget *parent)
: QWidget(parent), model(NULL), removePrimersButton(NULL)
{
    setupUi(this);
    QPushButton *newPrimerButton = buttonBox->addButton(tr("New primer"), QDialogButtonBox::ActionRole);
    connect(newPrimerButton, SIGNAL(clicked()), SLOT(sl_newPrimer()));

    removePrimersButton = buttonBox->addButton(tr("Remove primer(s)"), QDialogButtonBox::ActionRole);
    connect(removePrimersButton, SIGNAL(clicked()), SLOT(sl_removePrimers()));

    connect(buttonBox, SIGNAL(rejected()), SIGNAL(si_close()));

    model = new PrimerLibraryModel(this);
    primerTable->setModel(model);

    connect(primerTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), SLOT(sl_selectionChanged()));
    sl_selectionChanged();
}

void PrimerLibraryWidget::sl_newPrimer() {
    AddPrimerDialog dlg(this);
    const int result = dlg.exec();
    CHECK(QDialog::Accepted == result, );

    U2OpStatusImpl os;
    model->addPrimer(dlg.getPrimer(), os);
    checkOp(os);
}

void PrimerLibraryWidget::sl_removePrimers() {
    QItemSelectionModel *selectionModel = primerTable->selectionModel();
    SAFE_POINT(NULL != selectionModel, L10N::nullPointerError("Selection"), );

    QModelIndexList selection = selectionModel->selectedIndexes();
    while (!selection.isEmpty()) {
        U2OpStatusImpl os;
        model->removePrimer(selection.first(), os);
        checkOp(os);
        CHECK_OP(os, );
        selection = selectionModel->selectedIndexes();
    }
}

void PrimerLibraryWidget::sl_selectionChanged() {
    QItemSelectionModel *selectionModel = primerTable->selectionModel();
    SAFE_POINT(NULL != selectionModel, L10N::nullPointerError("Selection"), );

    removePrimersButton->setDisabled(selectionModel->selectedIndexes().isEmpty());
}

void PrimerLibraryWidget::checkOp(const U2OpStatus &os) {
    if (os.hasError()) {
        QMessageBox::warning(this, tr("Error"), os.getError());
    }
}

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

int PrimerLibraryModel::columnCount(const QModelIndex &parent) const {
    return 3;
}

QVariant PrimerLibraryModel::data(const QModelIndex &index, int role) const {
    CHECK(index.isValid(), QVariant());

    if (Qt::DisplayRole == role) {
        return displayData(index);
    }

    return QVariant();
}

QVariant PrimerLibraryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    CHECK(Qt::DisplayRole == role, QVariant());

    switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Tm");
        case 2:
            return tr("Sequence");
        default:
            return QVariant();
    }
}

QModelIndex PrimerLibraryModel::index(int row, int column, const QModelIndex &parent) const {
    CHECK(row < primers.size(), QModelIndex());
    return createIndex(row, column);
}

QModelIndex PrimerLibraryModel::parent(const QModelIndex &index) const {
    return QModelIndex();
}

int PrimerLibraryModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return primers.size();
}

void PrimerLibraryModel::addPrimer(Primer &primer, U2OpStatus &os) {
    PrimerLibrary *primerLibrary = PrimerLibrary::getInstance(os);
    CHECK_OP(os, );

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
            return primer.tm;
        case 2:
            return primer.sequence;
        default:
            return QVariant();
    }
}

} // U2
