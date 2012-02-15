/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SMITH_WATERMAN_SETTINGS_H_
#define _U2_SMITH_WATERMAN_SETTINGS_H_

#include <U2Core/U2Region.h>
#include <U2Core/DNATranslation.h>

#include <U2Core/SMatrix.h>
#include <U2Algorithm/SmithWatermanReportCallback.h>
#include <U2Algorithm/SmithWatermanResultFilters.h>
#include <U2Algorithm/SmithWatermanResult.h>
#include <U2Algorithm/SmithWatermanResult.h>
#include <U2Core/SequenceWalkerTask.h>

namespace U2 {

static inline bool isComplement(StrandOption strand) {
    return StrandOption_Both== strand || StrandOption_ComplementOnly == strand;
}
static inline bool isDirect(StrandOption strand) {
        return StrandOption_Both== strand || StrandOption_DirectOnly == strand;
}

struct GapModel {
    float scoreGapOpen;
    float scoreGapExtd;
};

struct SmithWatermanSettings {
    SmithWatermanSettings() 
        : percentOfScore(0), complTT( NULL ), aminoTT( NULL ),
        resultListener( NULL ), resultFilter( NULL ),
        resultCallback( NULL ) {
    }

    QByteArray          ptrn;
    QByteArray          sqnc;

    U2Region             globalRegion;
    StrandOption        strand;

    float               percentOfScore;
    GapModel            gapModel;
    SMatrix             pSm;

    DNATranslation*     complTT;
    DNATranslation*     aminoTT;

    SmithWatermanResultListener* resultListener;
    SmithWatermanResultFilter*   resultFilter;
    SmithWatermanReportCallback* resultCallback;

    bool operator==(const SmithWatermanSettings& op) const {
        return  ptrn == op.ptrn && 
                sqnc == op.sqnc && 
                globalRegion == op.globalRegion && 
                strand == op.strand &&
                gapModel.scoreGapExtd == op.gapModel.scoreGapExtd &&
                gapModel.scoreGapOpen == op.gapModel.scoreGapOpen;    
    }
};

} // namespace

#endif
