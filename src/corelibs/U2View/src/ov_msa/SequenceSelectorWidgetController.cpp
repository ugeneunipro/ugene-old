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

#include <U2Core/U2OpStatusUtils.h>

#include "SequenceSelectorWidgetController.h"

const int CURSOR_START_POSITION = 0;

namespace U2 {

SequenceSelectorWidgetController::SequenceSelectorWidgetController(MSAEditor* _msa)
    : msa(_msa), defaultSeqName(""), seqId(MAlignmentRow::invalidRowId())
{
    setupUi(this);
    filler = new MSACompletionFiller();

    seqLineEdit->setText(msa->getReferenceRowName());
    seqLineEdit->setCursorPosition(CURSOR_START_POSITION);
    seqLineEdit->setObjectName("sequenceLineEdit");
    completer = new BaseCompleter(filler, seqLineEdit);
    updateCompleter();

    connect(addSeq, SIGNAL(clicked()), SLOT(sl_addSeqClicked()));
    connect(deleteSeq, SIGNAL(clicked()), SLOT(sl_deleteSeqClicked()));
    
    connect(msa->getMSAObject(), SIGNAL(si_alignmentChanged(const MAlignment& , const MAlignmentModInfo&)), 
        SLOT(sl_seqLineEditEditingFinished(const MAlignment& , const MAlignmentModInfo&)));

    connect(completer, SIGNAL(si_editingFinished()), SLOT(sl_seqLineEditEditingFinished()));
}

SequenceSelectorWidgetController::~SequenceSelectorWidgetController() {
    delete completer;
}

QString SequenceSelectorWidgetController::text() const {
    return seqLineEdit->text();
}

void SequenceSelectorWidgetController::setSequenceId(qint64 newId) {
    const MAlignment ma = msa->getMSAObject()->getMAlignment();
    U2OpStatusImpl os;
    const MAlignmentRow selectedRow = ma.getRowByRowId(newId, os);
    seqId = newId;
    const QString selectedName = selectedRow.getName();
    if (seqLineEdit->text() != selectedName) {
        seqLineEdit->setText(selectedName);
        seqLineEdit->setCursorPosition(CURSOR_START_POSITION);
    }
}

qint64 SequenceSelectorWidgetController::sequenceId( ) const {
    return seqId;
}

void SequenceSelectorWidgetController::updateCompleter() {
    MAlignmentObject* maObj = msa->getMSAObject();
    const MAlignment& ma = maObj->getMAlignment();
    QStringList newNamesList = ma.getRowNames();
    filler->updateSeqList(newNamesList);
    if (!newNamesList.contains(defaultSeqName) && defaultSeqName != "") {
        defaultSeqName = "";
    }
    if (!newNamesList.contains(seqLineEdit->text())) {
        sl_seqLineEditEditingFinished();
    }
}

void SequenceSelectorWidgetController::sl_seqLineEditEditingFinished(const MAlignment& , const MAlignmentModInfo&){
    MAlignmentObject* maObj = msa->getMSAObject();
    const MAlignment& ma = maObj->getMAlignment();
    filler->updateSeqList(ma.getRowNames());
    defaultSeqName = "";
    sl_seqLineEditEditingFinished();
}

void SequenceSelectorWidgetController::sl_seqLineEditEditingFinished() {
    MAlignmentObject* maObj = msa->getMSAObject();
    const MAlignment& ma = maObj->getMAlignment();
    if (!ma.getRowNames().contains(seqLineEdit->text())) {
        seqLineEdit->setText(defaultSeqName);
    } else {
        const QString selectedSeqName = seqLineEdit->text();
        if (defaultSeqName != selectedSeqName) {
            defaultSeqName = seqLineEdit->text();
            seqLineEdit->setCursorPosition(CURSOR_START_POSITION);
        }
        // index in popup list
        const int sequenceIndex = completer->getLastChosenItemIndex();
        if ( completer == QObject::sender( ) && -1 != sequenceIndex ) {
            const QStringList rowNames = ma.getRowNames( );
            SAFE_POINT( rowNames.contains( selectedSeqName ), "Unexpected sequence name is selected", );
            if ( 1 < rowNames.count( selectedSeqName ) ) { // case when there are sequences with identical names
                int selectedRowIndex = -1;
                // search for chosen row in the msa
                for ( int sameNameCounter = 0; sameNameCounter <= sequenceIndex; ++sameNameCounter ) {
                    selectedRowIndex = rowNames.indexOf( selectedSeqName, selectedRowIndex + 1 );
                }
                seqId = ma.getRow( selectedRowIndex ).getRowId( );
            } else { // case when chosen name is unique in the msa
                seqId = ma.getRow( selectedSeqName ).getRowId( );
            }
        }
    }
    emit si_selectionChanged();
} 

void SequenceSelectorWidgetController::sl_addSeqClicked() {
    const MAlignmentRow selectedRow = msa->getMSAObject()->getRow(msa->getCurrentSelection().y());
    setSequenceId(selectedRow.getRowId());
    emit si_selectionChanged();
}

void SequenceSelectorWidgetController::sl_deleteSeqClicked() {
    seqLineEdit->setText("");
    setSequenceId(MAlignmentRow::invalidRowId());
    emit si_selectionChanged();
}

}
