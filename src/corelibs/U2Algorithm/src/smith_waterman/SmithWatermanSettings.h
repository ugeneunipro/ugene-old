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
