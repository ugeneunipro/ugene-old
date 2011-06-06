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

#include "ORFFinder.h"

#include <U2Core/Log.h>
#include <U2Core/TextUtils.h>
#include <U2Algorithm/DynTable.h>
#include <U2Algorithm/RollingArray.h>

#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslationImpl.h>

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
                            const char* sequence, 
                            int seqLen, 
                            int& stopFlag, 
                            int& percentsCompleted)
{
    Q_UNUSED(seqLen);
    assert(cfg.proteinTT && cfg.proteinTT->isThree2One());
    DNATranslation3to1Impl* aTT = (DNATranslation3to1Impl*)cfg.proteinTT;
    bool mustFit = cfg.mustFit;
    bool mustInit = cfg.mustInit;
    bool allowAltStart = cfg.allowAltStart;
    bool allowOverlap = cfg.allowOverlap;
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
        int end = cfg.searchRegion.endPos();
        for(int i = cfg.searchRegion.startPos; i < end && !stopFlag; i++, leftTillPercent--) {
            int frame = i%3;
            QList<int>* initiators = start + frame;
            if (!initiators->isEmpty() && aTT->isStopCodon(sequence + i)) {
                foreach(int initiator, *initiators) {
                    int len = i - initiator;
                    if (len>=minLen) rl->onResult(ORFFindResult(U2Region(initiator, len), frame));
                }
                initiators->clear();
                if (!mustInit) {
                    initiators->append(i+3);
                }
            } else if (initiators->isEmpty() || allowOverlap) {
                if (aTT->isStartCodon(sequence + i)
                    || (allowAltStart && aTT->isCodon(DNATranslationRole_Start_Alternative, sequence + i))
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
        if (!mustFit && !stopFlag) {
            //check if non-terminated ORFs remained
            for (int i=0; i<3;i++) {
                foreach(int initiator, start[i]) {
                    int len = end - initiator - i;
                    len -= len%3;
                    if (len>=minLen) rl->onResult(ORFFindResult(U2Region(initiator, len), i + 1));
                }
            }
        }
    }

    if (isComplement(cfg.strand)) {
        assert(cfg.complementTT && cfg.complementTT->isOne2One());
        QByteArray revComplDna(cfg.searchRegion.length, 0);
        cfg.complementTT->translate(sequence + cfg.searchRegion.startPos, cfg.searchRegion.length, 
            revComplDna.data(), cfg.searchRegion.length);
        TextUtils::reverse(revComplDna.data(), revComplDna.size());
        const char* rcSeq = revComplDna.data();

        QList<int> start[3];
        if (!mustInit) {
            for (int i=0; i<3;i++) {
                int frame = (cfg.searchRegion.endPos() - i)%3;
                start[frame].append(cfg.searchRegion.endPos() - i);
            }
        }
        int end = cfg.searchRegion.startPos;
        for(int i = cfg.searchRegion.endPos(); i >= end && !stopFlag; rcSeq++, i--, leftTillPercent--) {
            int frame = i%3;
            QList<int>* initiators = start + frame;
            if (!initiators->isEmpty() && aTT->isStopCodon(rcSeq)) {
                foreach(int initiator, *initiators) {
                    int len = initiator - i;
                    if (len>=minLen) rl->onResult(ORFFindResult(U2Region(i, len), frame - 3));
                }
                initiators->clear();
                if (!mustInit) {
                    initiators->append(i-3);
                }
            } else if (initiators->isEmpty() || allowOverlap) {
                if (aTT->isStartCodon(rcSeq)
                    || (allowAltStart && aTT->isCodon(DNATranslationRole_Start_Alternative, rcSeq))
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
        if (!mustFit && !stopFlag) {
            //check if non-terminated ORFs remained
            for (int i=0; i<3;i++) {
                foreach(int initiator, start[i]) {
                    int ind = end + i%3;
                    int len = initiator - ind;
                    len -= len%3;
                    if (len>=minLen) rl->onResult(ORFFindResult(U2Region(ind, len), i - 3));
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

