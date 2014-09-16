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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QCheckBox>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMessageBox>
#endif

#include "MSASelectSubalignmentDialog.h"
#include "MSAEditor.h"
#include <U2Gui/HelpButton.h>

namespace U2 {

SelectSubalignmentDialog::SelectSubalignmentDialog(MSAEditorUI *ui, QWidget *p)
    : QDialog(p),
      ui(ui)
{
    SAFE_POINT(ui != NULL, tr("MSA Editor UI is NULL"), );
    SAFE_POINT(ui->getEditor() != NULL, tr("MSA Editor is NULL"), );

    setupUi(this);
    new HelpButton(this, buttonBox, "4227488");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Select"));

    connect(allButton, SIGNAL(clicked()), SLOT(sl_allButtonClicked()));
    connect(noneButton, SIGNAL(clicked()), SLOT(sl_noneButtonClicked()));
    connect(invertButton, SIGNAL(clicked()), SLOT(sl_invertButtonClicked()));

    MAlignmentObject *mobj = ui->getEditor()->getMSAObject();
    SAFE_POINT(mobj != NULL, tr("MSA Object is NULL"), );

    int rowNumber = mobj->getNumRows();
    int alignLength = mobj->getLength();

    sequencesTableWidget->clearContents();
    sequencesTableWidget->setRowCount(rowNumber);
    sequencesTableWidget->setColumnCount(1);
    sequencesTableWidget->verticalHeader()->setHidden( true );
    sequencesTableWidget->horizontalHeader()->setHidden( true );
    sequencesTableWidget->setShowGrid(false);
#if (QT_VERSION < 0x050000) //Qt 5
    sequencesTableWidget->horizontalHeader()->setResizeMode( 0, QHeaderView::Stretch );
#else
    sequencesTableWidget->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
#endif


    int startSeq = -1;
    int endSeq = -1;
    int startPos = -1;
    int endPos = -1;
    QRect selection = ui->getEditor()->getCurrentSelection();
    if (selection.isNull()) {
        startPos = 1;
        endPos = alignLength;
        startSeq = 0;
        endSeq = rowNumber -1;
     } else {
        startSeq = selection.y();
        endSeq = selection.y() + selection.height() - 1;
        startPos = selection.x() + 1;
        endPos = selection.x() + selection.width();
    }

    startPosBox->setMaximum(alignLength);
    endPosBox->setMaximum(alignLength);

    startPosBox->setValue(startPos);
    endPosBox->setValue(endPos);

    for (int i = 0; i < rowNumber; i++) {
        QCheckBox *cb = new QCheckBox(mobj->getMAlignment().getRow(i).getName(), this);
        cb->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
        if ( (i >= startSeq) && (i <= endSeq)) {
            cb->setChecked(true);
        }
        sequencesTableWidget->setCellWidget(i, 0, cb);
        sequencesTableWidget->setRowHeight(i, 15);
    }

}

void SelectSubalignmentDialog::accept() {
    int start = startPosBox->value() - 1;
    int end = endPosBox->value() - 1;
    int seqLen = ui->getEditor()->getAlignmentLen();

    CHECK_EXT( start <= end,
               QMessageBox::critical(this, windowTitle(), tr("Start position must be less than end position!")), );

    U2Region region(start, end - start + 1);
    U2Region sequence(0, seqLen);
    CHECK_EXT( sequence.contains(region),
               QMessageBox::critical(this, windowTitle(), tr("Entered region not contained in current sequence")), );

    selectedNames.clear();
    selectedIndexes.clear();
    for (qint64 i = 0; i < sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        if(cb->isChecked()){
            selectedNames.append(cb->text());
            selectedIndexes.append(i);
        }
    }
    CHECK_EXT( !selectedIndexes.isEmpty(),
               QMessageBox::critical(this, windowTitle(), tr("No sequences selected")), );

    window = region;
    QDialog::accept();
}

void SelectSubalignmentDialog::sl_allButtonClicked(){
    for (int i = 0; i < sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        cb->setChecked(true);
    }
}

void SelectSubalignmentDialog::sl_invertButtonClicked(){
    for (int i=0; i<sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        cb->setChecked(!cb->isChecked());
    }
}

void SelectSubalignmentDialog::sl_noneButtonClicked(){
    for (int i=0; i<sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        cb->setChecked(false);
    }
}


} // namespace
