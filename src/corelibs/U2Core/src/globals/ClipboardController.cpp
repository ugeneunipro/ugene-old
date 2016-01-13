/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "ClipboardController.h"

#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequenceObject.h>

namespace U2 {

///////////////////
///PasteFactory
PasteFactory::PasteFactory(QObject *parent)
    :QObject(parent){
}

///////////////////
///PasteTask
PasteTask::PasteTask()
    :Task(tr("Paste data"), TaskFlag_None){
}

///////////////////
///PasteUtils
QList<DNASequence> PasteUtils::getSequences(const QList<Document*>& docs, U2OpStatus& os){
    QList<DNASequence> res;

    foreach (Document* doc, docs){
        foreach(GObject *seqObj, doc->findGObjectByType(GObjectTypes::SEQUENCE)) {
            U2SequenceObject *casted = qobject_cast<U2SequenceObject*>(seqObj);
            if (casted == NULL){
                continue;
            }
            DNASequence seq = casted->getWholeSequence(os);
            if (os.hasError()){
                continue;
            }
            seq.alphabet = casted->getAlphabet();
            res.append(seq);

        }

        foreach(GObject *msaObj, doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT)) {
            MAlignmentObject* casted = qobject_cast<MAlignmentObject*>(msaObj);
            if (casted == NULL){
                continue;
            }
            foreach(const MAlignmentRow& row, casted->getMAlignment().getRows()) {
                DNASequence seq =  row.getSequence();
                seq.alphabet = casted->getAlphabet();
                res.append(seq);
            }

        }
    }

    return res;
}

} // U2
