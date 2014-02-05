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

#include "ORFFinder.h"

#include <U2Algorithm/DynTable.h>
#include <U2Algorithm/RollingArray.h>

#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslationImpl.h>
#include <U2Core/Log.h>
#include <U2Core/Task.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>

#include <assert.h>

namespace U2 {

const QString ORFAlgorithmSettings::ANNOTATION_GROUP_NAME("orf");
const QString ORFAlgorithmSettings::STRAND_BOTH("both strands");
const QString ORFAlgorithmSettings::STRAND_DIRECT("direct");
const QString ORFAlgorithmSettings::STRAND_COMPL("complement");

static bool isDirect(ORFAlgorithmStrand s) {
    return s == ORFAlgorithmStrand_Both || s == ORFAlgorithmStrand_Direct;
}

static bool isComplement(ORFAlgorithmStrand s) {
    return s == ORFAlgorithmStrand_Both || s == ORFAlgorithmStrand_Complement;
}

void ORFFindAlgorithm::find(
                            ORFFindResultsListener* rl,
                            const ORFAlgorithmSettings& cfg,
                            U2EntityRef& entityRef,
                            int& stopFlag, 
                            int& percentsCompleted)
{
    assert(cfg.maxResult2Search >= 0);
    assert(cfg.proteinTT && cfg.proteinTT->isThree2One());

    TaskStateInfo os;
    U2SequenceObject dnaSeq("sequence",entityRef);

    const int BLOCK_READ_FROM_DB = 128000;
    int seqPointer = 0;
    QByteArray sequence("");

    DNATranslation3to1Impl* aTT = (DNATranslation3to1Impl*)cfg.proteinTT;
    bool mustFit = cfg.mustFit;
    bool mustInit = cfg.mustInit;
    bool allowAltStart = cfg.allowAltStart;
    bool allowOverlap = cfg.allowOverlap;
    bool circularSearch = cfg.circularSearch;
    int minLen = qMax(cfg.minLen, 3);

    int onePercentLen = cfg.searchRegion.length/100;
    if (cfg.strand == ORFAlgorithmStrand_Both) onePercentLen /=2;
    int leftTillPercent = onePercentLen;
    percentsCompleted = 0;

    if (isDirect(cfg.strand)) {
        QList<int> start[3];
        if (!mustInit) {
            for (int i=0; i<3;i++) {
                int frame = (cfg.searchRegion.startPos + i)%3;
                start[frame].append(cfg.searchRegion.startPos + i);
            }
        }
        qint64 end = cfg.searchRegion.endPos();
        for(qint64 i = cfg.searchRegion.startPos; i < end && !stopFlag && !os.isCoR(); i++, leftTillPercent--,++seqPointer) {
            if((seqPointer % BLOCK_READ_FROM_DB) == 0){ // query to db
                sequence.clear();
                qint64 regLen = qMin(end - i, (qint64)BLOCK_READ_FROM_DB + 3);
                sequence.append(dnaSeq.getSequenceData(U2Region(i, regLen), os));
                SAFE_POINT_OP(os, );
                seqPointer = 0;
            }
            int frame = i%3;
            QList<int>* initiators = start + frame;
            if (!initiators->isEmpty() && aTT->isStopCodon(sequence.data() + seqPointer)) {
                foreach(int initiator, *initiators) {
                    qint64 len = i - initiator;
                    if (cfg.includeStopCodon) {
                        len+=3;
                    }
                    if (len>=minLen && !os.isCoR()) rl->onResult(ORFFindResult(U2Region(initiator, len), frame),os);
                }
                initiators->clear();
                if (!mustInit) {
                    initiators->append(i+3);
                }
            } else if (initiators->isEmpty() || allowOverlap) {
                if (aTT->isStartCodon(sequence.data() + seqPointer)
                    || (allowAltStart && aTT->isCodon(DNATranslationRole_Start_Alternative, sequence.data() + seqPointer))
                   ) {
                    if (initiators->isEmpty() || initiators->last() != i) {
                        initiators->append(i);
                    }
                }
            }
            if (leftTillPercent == 0) {
                percentsCompleted = qMin(percentsCompleted+1,100);
                leftTillPercent = onePercentLen;
            }
        }

        if(circularSearch && !stopFlag && !os.isCoR()){
            //circular
            qint64 regLen = end - cfg.searchRegion.startPos;
            qint64 minInitiator = end;
            bool initiatorsRemain = false;
            for (int i=0; i<3;i++) {
                foreach(int initiator, start[i]) {
                    if(initiator < minInitiator){
                        minInitiator = initiator;
                        initiatorsRemain = true;
                    }

                }
            }
            seqPointer = 0;
            for(qint64 i = cfg.searchRegion.startPos; i < minInitiator && !stopFlag && initiatorsRemain && !os.isCoR(); i++,++seqPointer) {
                if( (seqPointer %BLOCK_READ_FROM_DB) == 0){ // query to db
                    sequence.clear();
                    qint64 regLen = qMin((qint64)minInitiator - i, (qint64)BLOCK_READ_FROM_DB + 3);
                    sequence.append(dnaSeq.getSequenceData(U2Region(i, regLen), os));
                    SAFE_POINT_OP(os, );
                    seqPointer = 0;
                }
                int frame =(regLen+i)%3;
                QList<int>* initiators = start + frame;
                if (!initiators->isEmpty() && aTT->isStopCodon(sequence.data() + seqPointer)) {
                    foreach(int initiator, *initiators) {
                        int len = regLen + i - initiator;
                        if (len>=minLen && !os.isCoR()){                            
                            rl->onResult(ORFFindResult(U2Region(initiator, end-initiator), U2Region(cfg.searchRegion.startPos, i), frame),os);
                        }
                    }
                    initiators->clear();
                }
            }
        }

        if (!mustFit && !stopFlag ) {
            //check if non-terminated ORFs remained
            for (int i=0; i<3;i++) {
                foreach(int initiator, start[i]) {
                    int len = end - initiator - i;
                    len -= len%3;
                    if (len>=minLen && !os.isCoR()) rl->onResult(ORFFindResult(U2Region(initiator, len), i + 1),os);
                }
            }
         }
    }

    if (isComplement(cfg.strand)) {
        assert(cfg.complementTT && cfg.complementTT->isOne2One());

        QList<int> start[3];
        if (!mustInit) {
            for (int i=0; i<3;i++) {
                int frame = (cfg.searchRegion.endPos()-1 - i)%3;
                start[frame].append(cfg.searchRegion.endPos()-1 - i);
            }
        }
        qint64 end = cfg.searchRegion.startPos;
        seqPointer = 0;
        for(int i = cfg.searchRegion.endPos()-1; i >= end && !stopFlag && !os.isCoR();i--, leftTillPercent--,seqPointer++) {
            if((seqPointer % BLOCK_READ_FROM_DB) == 0){// query to db
                sequence.clear();
                QByteArray tmp;
                qint64 regStart = qMax(end, (qint64)i - (BLOCK_READ_FROM_DB + 3));
                qint64 regLen = qMin(i - end + 1, (qint64)BLOCK_READ_FROM_DB + 3 + 1);
                tmp.append(dnaSeq.getSequenceData(U2Region(regStart, regLen), os));
                SAFE_POINT_OP(os, );
                sequence.append(tmp,tmp.size());
                cfg.complementTT->translate(tmp,tmp.size(),sequence.data(),sequence.size());
                TextUtils::reverse(sequence.data(),sequence.size());
                seqPointer = 0;
            }
            int frame = i%3;
            QList<int>* initiators = start + frame;
            if (!initiators->isEmpty() && aTT->isStopCodon(sequence.data()+seqPointer)) {
                foreach(int initiator, *initiators) {
                    int len = initiator - i;
                    int ind = i;
                    if (cfg.includeStopCodon) {
                        ind -= 3;
                        len += 3;
                    }
                    if (len>=minLen && !os.isCoR()) rl->onResult(ORFFindResult(U2Region(ind+1, len), frame - 3),os);
                }
                initiators->clear();
                if (!mustInit) {
                    initiators->append(i-3);
                }
            } else if (initiators->isEmpty() || allowOverlap) {
                if (aTT->isStartCodon(sequence.data()+seqPointer)
                    || (allowAltStart && aTT->isCodon(DNATranslationRole_Start_Alternative, sequence.data()+seqPointer))
                   ) {
                    if (initiators->isEmpty() || initiators->last() != i) {
                        initiators->append(i);
                    }
                }
            }
            if (leftTillPercent == 0) {
                percentsCompleted = qMin(percentsCompleted+1,100);
                leftTillPercent = onePercentLen;
            }
        }
        if(circularSearch && !stopFlag && !os.isCoR()){
            int regLen = cfg.searchRegion.endPos() - cfg.searchRegion.startPos;
            int maxInitiator = -1;
            bool initiatorsRemain = false;
            for (int i=0; i<3;i++) {
                foreach(int initiator, start[i]) {
                    if(initiator > maxInitiator){
                        maxInitiator = initiator;
                        initiatorsRemain = true;
                    }

                }
            }
            seqPointer = 0;
            for(qint64 i = cfg.searchRegion.endPos()-1; i >= maxInitiator && !stopFlag && initiatorsRemain && !os.isCoR(); i--,seqPointer++) {
                if((seqPointer % BLOCK_READ_FROM_DB) == 0){// query to db
                    sequence.clear();
                    QByteArray tmp;
                    qint64 regStart = qMax((qint64)maxInitiator, (qint64)i - (BLOCK_READ_FROM_DB + 3));
                    qint64 regLen = qMin(i - maxInitiator + 1, (qint64)BLOCK_READ_FROM_DB + 3 + 1);
                    tmp.append(dnaSeq.getSequenceData(U2Region(regStart, regLen), os));
                    SAFE_POINT_OP(os, );
                    sequence.append(tmp,tmp.size());
                    cfg.complementTT->translate(tmp,tmp.size(),sequence.data(),sequence.size());
                    TextUtils::reverse(sequence.data(),sequence.size());
                    seqPointer = 0;
                }
                int frame =(regLen+i)%3;
                QList<int>* initiators = start + frame;
                if (!initiators->isEmpty() && aTT->isStopCodon(sequence.data()+seqPointer)) {
                    foreach(int initiator, *initiators) {
                        int len = regLen + initiator - i ;
                        if (len>=minLen && !os.isCoR()){                            
                            rl->onResult(ORFFindResult(U2Region(i+1, cfg.searchRegion.endPos()-(i+1)), U2Region(end, initiator+1), frame - 3),os);
                        }
                    }
                    initiators->clear();
                }
            }
        }
        if (!mustFit && !stopFlag ) {
            //check if non-terminated ORFs remained
            for (int i=0; i<3;i++) {
                foreach(int initiator, start[i]) {
                    int ind = end + i%3;
                    int len = initiator - ind;
                    len -= len%3;
                    if (len>=minLen && !os.isCoR()) rl->onResult(ORFFindResult(U2Region(ind, len), i - 3),os);
                }
            }
        }
    }

}


QString ORFAlgorithmSettings::getStrandStringId(ORFAlgorithmStrand strand)
{
    if (strand == ORFAlgorithmStrand_Direct) {
        return STRAND_DIRECT;
    } else if (strand == ORFAlgorithmStrand_Complement) {
        return STRAND_COMPL;
    } else {
        assert (strand == ORFAlgorithmStrand_Both );
        return STRAND_BOTH;
    }
}

ORFAlgorithmStrand ORFAlgorithmSettings::getStrandByStringId( const QString& id )
{
    if (id == STRAND_DIRECT) {
        return ORFAlgorithmStrand_Direct;
    } else if (id == STRAND_COMPL) {
        return ORFAlgorithmStrand_Complement;
    } else {
        return ORFAlgorithmStrand_Both;
    }
}

}//namespace

