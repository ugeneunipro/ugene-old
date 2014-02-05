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

#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2OpStatusUtils.h>

#include "MAlignmentUtilTasks.h"


namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// TranslateMSA2AminoTask

TranslateMSA2AminoTask::TranslateMSA2AminoTask( MAlignmentObject* obj )
: Task("TranslateMSA2AminoTask", TaskFlags_FOSCOE), maObj(obj)
{
    assert(maObj != NULL);
    assert(maObj->getAlphabet()->isNucleic());

    QList<DNATranslation*> translations = 
        AppContext::getDNATranslationRegistry()->lookupTranslation(maObj->getAlphabet(), DNATranslationType_NUCL_2_AMINO);
    CHECK_EXT(translations.isEmpty(), setError(tr("Unable to find suitable translation for %1").arg(maObj->getGObjectName())), );
    
    translation = AppContext::getDNATranslationRegistry()->getStandardGeneticCodeTranslation(maObj->getAlphabet());

}

TranslateMSA2AminoTask::TranslateMSA2AminoTask( MAlignmentObject* obj, const QString& translationId )
: Task("TranslateMSA2AminoTask", TaskFlags_FOSCOE), maObj(obj)
{
    assert(maObj != NULL);
    assert(maObj->getAlphabet()->isNucleic());

    translation = AppContext::getDNATranslationRegistry()->lookupTranslation(translationId);

}


void TranslateMSA2AminoTask::run() {
    assert(translation != NULL);
    
    QList<DNASequence> lst = MSAUtils::ma2seq(maObj->getMAlignment(), true);
    resultMA = MAlignment(maObj->getMAlignment().getName(),translation->getDstAlphabet()) ;

    foreach (const DNASequence& dna, lst) {    
        int buflen = dna.length() / 3;
        QByteArray buf(buflen,'\0');
        translation->translate(dna.seq.constData(), dna.length(), buf.data(), buflen);
        buf.replace("*","X");
        resultMA.addRow(dna.getName(), buf, stateInfo);
    }
}


Task::ReportResult TranslateMSA2AminoTask::report() {
    if (!resultMA.isEmpty()) {
        maObj->setMAlignment(resultMA);
    }

    return ReportResult_Finished;
}


//////////////////////////////////////////////////////////////////////////
/// AlignInAminoFormTask

AlignInAminoFormTask::AlignInAminoFormTask( MAlignmentObject* obj, AlignGObjectTask* t, const QString& trId )
: Task ("Align in amino form", TaskFlags_FOSCOE), alignTask(t), maObj(obj), clonedObj(NULL), traslId(trId)
{
    setMaxParallelSubtasks(1);
}

AlignInAminoFormTask::~AlignInAminoFormTask() {
    delete clonedObj;
}

void AlignInAminoFormTask::prepare() {
    CHECK_EXT(maObj->getAlphabet()->isNucleic(), setError("AlignInAminoFormTask: Input alphabet it not nucleic!"), );

    SAFE_POINT(NULL != maObj, "NULL maObj in AlignInAminoFormTask::prepare!",);
    const MAlignment& msa = maObj->getMAlignment();
    const U2DbiRef& dbiRef = maObj->getEntityRef().dbiRef;

    U2EntityRef msaRef = MAlignmentImporter::createAlignment(dbiRef, msa, stateInfo);
    CHECK_OP(stateInfo, );
    
    clonedObj = new MAlignmentObject(msa.getName(), msaRef, maObj->getGHintsMap());
    alignTask->setMAObject(clonedObj);
    bufMA = clonedObj->getMAlignment();
    addSubTask(new TranslateMSA2AminoTask(clonedObj,traslId));
    addSubTask(alignTask);
}


void AlignInAminoFormTask::run() {
    CHECK_OP(stateInfo, );
    
    if (bufMA.isEmpty()) {
        return;
    }
    
    // applying a gap map
    assert(clonedObj != NULL);
    const MAlignment& newMA = clonedObj->getMAlignment();
    const QList<MAlignmentRow>& rows = newMA.getRows();
    
    // If the rows where rearranged we need to find index of each one.
    // However, what if the rows have equal names?
    // Do we have to keep some MAP<old seq, new seq> in memory? 

    U2OpStatus2Log os;
    foreach (const MAlignmentRow& row, rows) {
        int rowIdx = MSAUtils::getRowIndexByName(bufMA, row.getName());
        if (rowIdx == -1) {
            setError(tr("Can not find row %1 in original alignment.").arg(row.getName()));
            return;
        }
        for (int pos =0; pos < row.getCoreEnd(); ++pos) {
            char c = newMA.charAt(rowIdx, pos);
            if (c == MAlignment_GapChar) {
                bufMA.insertGaps(rowIdx, pos, 3, os);
            }
        }
    }
    bufMA.trim();
    
}

Task::ReportResult AlignInAminoFormTask::report() {
    CHECK_OP(stateInfo, Task::ReportResult_Finished);

    if (!bufMA.isEmpty()) {
        maObj->setMAlignment(bufMA);
    }

    return ReportResult_Finished;
}


} // U2

