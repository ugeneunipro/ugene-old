/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/DNATranslation.h>


#include "MAlignmentUtilTasks.h"


namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// TranslateMSA2AminoTask

TranslateMSA2AminoTask::TranslateMSA2AminoTask( MAlignmentObject* obj )
: Task("TranslateMSA2AminoTask", TaskFlags_FOSCOE), maObj(obj)
{
    assert(maObj != NULL);
    assert(maObj->getAlphabet()->isNucleic());

    translations = AppContext::getDNATranslationRegistry()->lookupTranslation(maObj->getAlphabet(), DNATranslationType_NUCL_2_AMINO);
}

void TranslateMSA2AminoTask::run()
{
    assert(!translations.isEmpty());

    if (translations.isEmpty()) {
        setError(tr("Unable to find suitable translation for %1").arg(maObj->getGObjectName()));
        return;
    }

    DNATranslation* transl = translations.first();

    QList<DNASequence> lst = MSAUtils::ma2seq(maObj->getMAlignment(), true);
    resultMA = MAlignment(maObj->getMAlignment().getName(),transl->getDstAlphabet()) ;

    foreach (const DNASequence& dna, lst) {    
        int buflen = dna.length() / 3;
        QByteArray buf(buflen,'\0');
        transl->translate(dna.seq.constData(), dna.length(), buf.data(), buflen);
        buf.replace("*","X");
        MAlignmentRow row(dna.getName(), buf);  
        resultMA.addRow(row);
    }

    

}

Task::ReportResult TranslateMSA2AminoTask::report()
{
    if (!resultMA.isEmpty()) {
        maObj->setMAlignment(resultMA);
    }

    return ReportResult_Finished;
}


//////////////////////////////////////////////////////////////////////////
/// MSAAlignMultiTask

MSAAlignMultiTask::MSAAlignMultiTask( MAlignmentObject* obj, MAlignmentGObjectTask* t, bool toAmino )
: Task ("MSAAlignMultiTask",TaskFlags_FOSCOE), alignTask(t), maObj(obj), clonedObj(NULL), convertToAmino(toAmino)
{
    setMaxParallelSubtasks(1);
}

void MSAAlignMultiTask::prepare()
{
    if (convertToAmino == true && maObj->getAlphabet()->isNucleic()) {
        clonedObj = qobject_cast<MAlignmentObject*> ( maObj->clone() );
        alignTask->setMAObject(clonedObj);
        bufMA = clonedObj->getMAlignment();
        addSubTask(new TranslateMSA2AminoTask(clonedObj));
    }

    addSubTask(alignTask);

}

Task::ReportResult MSAAlignMultiTask::report()
{

    if (!bufMA.isEmpty() && convertToAmino) {
        maObj->setMAlignment(bufMA);
    }

    return ReportResult_Finished;
}

void MSAAlignMultiTask::run()
{
    if ( hasErrors() || isCanceled() ) {
        return;
    }
    
    if (bufMA.isEmpty() || !convertToAmino) {
        return;
    }
    
    // applying a gap map
    assert(clonedObj != NULL);
    const MAlignment& newMA = clonedObj->getMAlignment();
    const QList<MAlignmentRow>& rows = newMA.getRows();
    
    // If the rows where rearranged we need to find index of each one.
    // However, what if the rows have equal names?
    // Do we have to keep some MAP<old seq, new seq> in memory? 

    foreach (const MAlignmentRow& row, rows) {
        int rowIdx = MSAUtils::getRowIndexByName(bufMA, row.getName());
        if (rowIdx == -1) {
            setError(tr("Can not find row %1 in original alignment.").arg(row.getName()));
        }
        for (int pos =0; pos < row.getCoreEnd(); ++pos) {
            char c = newMA.charAt(rowIdx, pos);
            if (c == MAlignment_GapChar) {
                bufMA.insertChars(rowIdx,pos,MAlignment_GapChar,3);
            }
        }
    }
    bufMA.trim();
    
}



} // U2

