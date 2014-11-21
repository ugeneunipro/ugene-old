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

#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "EditPrimerDialog.h"
#include "PrimerLibraryTable.h"

#include "PrimerLibraryWidget.h"

#define CHECK_OP_UI(os, result) \
    if (os.hasError()) { \
        QMessageBox::warning(this, tr("Error"), os.getError()); \
    } \
    CHECK_OP(os, result);

namespace U2 {

PrimerLibraryWidget::PrimerLibraryWidget(QWidget *parent)
: QWidget(parent), editPrimerButton(NULL), removePrimersButton(NULL)
{
    setupUi(this);
    QPushButton *newPrimerButton = buttonBox->addButton(tr("New primer"), QDialogButtonBox::ActionRole);
    connect(newPrimerButton, SIGNAL(clicked()), SLOT(sl_newPrimer()));

    editPrimerButton = buttonBox->addButton(tr("Edit primer"), QDialogButtonBox::ActionRole);
    connect(editPrimerButton, SIGNAL(clicked()), SLOT(sl_editPrimer()));

    removePrimersButton = buttonBox->addButton(tr("Remove primer(s)"), QDialogButtonBox::ActionRole);
    connect(removePrimersButton, SIGNAL(clicked()), SLOT(sl_removePrimers()));

    connect(buttonBox, SIGNAL(rejected()), SIGNAL(si_close()));

    connect(primerTable, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(sl_editPrimer()));
    connect(primerTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), SLOT(sl_selectionChanged()));
    sl_selectionChanged();
}

void PrimerLibraryWidget::sl_newPrimer() {
    EditPrimerDialog dlg(this);
    const int result = dlg.exec();
    CHECK(QDialog::Accepted == result, );

    U2OpStatusImpl os;
    Primer primer = dlg.getPrimer();
    primerTable->addPrimer(primer, os);
    CHECK_OP_UI(os, );
}

void PrimerLibraryWidget::sl_editPrimer() {
    QList<Primer> selection = primerTable->getSelection();
    CHECK(1 == selection.size(), );
    Primer primerToEdit = selection.first();
    EditPrimerDialog dlg(this, primerToEdit);
    const int result = dlg.exec();
    CHECK(QDialog::Accepted == result, );

    U2OpStatusImpl os;
    Primer primer = dlg.getPrimer();
    primer.id = primerToEdit.id;
    primerTable->updatePrimer(primer, os);
    CHECK_OP_UI(os, );
}

void PrimerLibraryWidget::sl_removePrimers() {
    QItemSelectionModel *selectionModel = primerTable->selectionModel();
    SAFE_POINT(NULL != selectionModel, L10N::nullPointerError("Selection"), );

    QModelIndexList selection = selectionModel->selectedIndexes();
    while (!selection.isEmpty()) {
        U2OpStatusImpl os;
        primerTable->removePrimer(selection.first(), os);
        CHECK_OP_UI(os, );
        selection = selectionModel->selectedIndexes();
    }
}

void PrimerLibraryWidget::sl_selectionChanged() {
    QList<Primer> selection = primerTable->getSelection();
    editPrimerButton->setEnabled(1 == selection.size());
    removePrimersButton->setDisabled(selection.isEmpty());
}

} // U2
