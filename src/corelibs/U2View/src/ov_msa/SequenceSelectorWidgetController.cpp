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

#include "SequenceSelectorWidgetController.h"

namespace U2{

SequenceSelectorWidgetController::SequenceSelectorWidgetController(MSAEditor* _msa):msa(_msa), defaultSeqName(""){
    setupUi(this);
    filler = new MSACompletionFiller();
    seqLineEdit->setText(msa->getRefSeqName());
    seqLineEdit->setCursorPosition(0);
    completer = new BaseCompleter(filler, seqLineEdit);
    sl_updateCompleter();

    connect(seqLineEdit, SIGNAL(editingFinished()), SLOT(sl_seqLineEditEditingFinished()));
    connect(addSeq, SIGNAL(clicked()), SLOT(sl_addSeqClicked()));
    connect(deleteSeq, SIGNAL(clicked()), SLOT(sl_deleteSeqClicked()));
    
    connect(msa->getMSAObject(), SIGNAL(si_alignmentChanged(const MAlignment& , const MAlignmentModInfo&)), 
        SLOT(sl_seqLineEditEditingFinished(const MAlignment& , const MAlignmentModInfo&)));
    
}

void SequenceSelectorWidgetController::setText(QString str) {
    if (seqLineEdit->text() != str) {
        seqLineEdit->setText(str);
        seqLineEdit->setCursorPosition(0);
    }
}

void SequenceSelectorWidgetController::sl_updateCompleter(){
    MAlignmentObject* maObj = msa->getMSAObject();
    const MAlignment& ma = maObj->getMAlignment();
    QStringList newNamesList = ma.getRowNames();
    filler->updateSeqList(newNamesList);
    if(!newNamesList.contains(defaultSeqName) && defaultSeqName != ""){
        defaultSeqName = "";
    }
    if(!newNamesList.contains(seqLineEdit->text())){
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

void SequenceSelectorWidgetController::sl_seqLineEditEditingFinished(){
    MAlignmentObject* maObj = msa->getMSAObject();
    const MAlignment& ma = maObj->getMAlignment();
    if(!ma.getRowNames().contains(seqLineEdit->text())){
        seqLineEdit->setText(defaultSeqName);
    }else{
        if (defaultSeqName != seqLineEdit->text()) {
            defaultSeqName = seqLineEdit->text();
            seqLineEdit->setCursorPosition(0);
        }
    }
    emit si_textControllerChanged();
} 

void SequenceSelectorWidgetController::sl_addSeqClicked(){
    seqLineEdit->setText(msa->getMSAObject()->getRow(msa->getCurrentSelection().y()).getName());
    seqLineEdit->setCursorPosition(0);
    emit si_textControllerChanged();
}

void SequenceSelectorWidgetController::sl_deleteSeqClicked(){
    seqLineEdit->setText("");
    emit si_textControllerChanged();
}

}
