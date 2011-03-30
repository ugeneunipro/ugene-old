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

const QString ORFSettingsKeys::STRAND("orf_finder/strand");
const QString ORFSettingsKeys::ALLOW_ALT_START("orf_finder/allow_alt_start");
const QString ORFSettingsKeys::AMINO_TRANSL("orf_finder/amino_transl");
const QString ORFSettingsKeys::MIN_LEN("orf_finder/min_length");
const QString ORFSettingsKeys::MUST_FIT("orf_finder/must_fit");
const QString ORFSettingsKeys::MUST_INIT("orf_finder/must_init");
const QString ORFSettingsKeys::SEARCH_REGION("orf_finder/region");

const QString ORFAlgorithmSettings::ANNOTATION_GROUP_NAME("orf");

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
    int minLen = qMax(cfg.minLen, 3);

    int onePercentLen = cfg.searchRegion.length/100;
    if (cfg.strand == ORFAlgorithmStrand_Both) onePercentLen /=2;
    int leftTillPercent = onePercentLen;
    percentsCompleted = 0;

    if (isDirect(cfg.strand)) {
        int start[3] = {-1,-1,-1};
        if (!mustInit) {
            for (int i=0; i<3;i++) {
                int frame = (cfg.searchRegion.startPos + i)%3;
                start[frame] = cfg.searchRegion.startPos + i;
            }
        }
        int end = cfg.searchRegion.endPos();
        for(int i = cfg.searchRegion.startPos; i < end && !stopFlag; i++, leftTillPercent--) {
            int frame = i%3;
            if (start[frame] >=0) {
                if (aTT->isStopCodon(sequence + i)) {
                    int len = i - start[frame];
                    if (len>=minLen) rl->onResult(ORFFindResult(U2Region(start[frame], len), frame));
                    start[frame] = mustInit? -1 : i + 3;
                }
            } else if (mustInit) { 
                if (aTT->isStartCodon(sequence + i)) {
                    start[frame] = i;
                } else if (allowAltStart 
                    && aTT->isCodon(DNATranslationRole_Start_Alternative, sequence + i)) {
                    start[frame] = i;
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
                if (start[i] >=0) {
                    int len = end - start[i] - i;
                    len -= len%3;
                    if (len>=minLen) rl->onResult(ORFFindResult(U2Region(start[i], len), i + 1));
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

        int start[3] = {-1,-1,-1};
        if (!mustInit) {
            for (int i=0; i<3;i++) {
                int frame = (cfg.searchRegion.endPos() - i)%3;
                start[frame] = cfg.searchRegion.endPos() - i;
            }
        }
        int end = cfg.searchRegion.startPos;
        for(int i = cfg.searchRegion.endPos(); i >= end && !stopFlag; rcSeq++, i--, leftTillPercent--) {
            int frame = i%3;
            if (start[frame] >=0) {
                if (aTT->isStopCodon(rcSeq)) {
                    int len = start[frame] - i;
                    if (len>=minLen) rl->onResult(ORFFindResult(U2Region(i, len), frame - 3));
                    start[frame] = mustInit? -1 : i - 3;
                }
            } else if (mustInit) {
                if (aTT->isStartCodon(rcSeq)) {
                    start[frame] = i;
                } else if (allowAltStart 
                    && aTT->isCodon(DNATranslationRole_Start_Alternative, rcSeq)) {
                    start[frame] = i;
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
                if (start[i] >=0) {
                    int ind = end + i%3;
                    int len = start[i] - ind;
                    len -= len%3;
                    if (len>=minLen) rl->onResult(ORFFindResult(U2Region(ind, len), i - 3));
                }
            }
        }
    }

}

}//namespace

