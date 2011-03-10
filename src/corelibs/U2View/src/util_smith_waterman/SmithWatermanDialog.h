#ifndef _U2_SMITH_WATERMAN_DIALOG_H_
#define _U2_SMITH_WATERMAN_DIALOG_H_

#include <U2Algorithm/SmithWatermanSettings.h>

namespace U2 {

class ADVSequenceObjectContext;
class CreateAnnotationWidgetController;

#define DEF_PERCENT_OF_SCORE    90
#define DEF_GAP_OPEN_SCORE      (-10.0f)
#define DEF_GAP_EXTD_SCORE      (-1.0f)

enum SmithWatermanRangeType {
    SmithWatermanRangeType_noRange,
    SmithWatermanRangeType_wholeSequence,
    SmithWatermanRangeType_selectedRange,
    SmithWatermanRangeType_customRange
};

enum SmithWatermanSearchType {
    SmithWatermanSearchType_noSearch,
    SmithWatermanSearchType_inSequence,
    SmithWatermanSearchType_inTranslation
};

struct SWDialogConfig {
    SWDialogConfig() {
        ptrn = QByteArray();
        algVersion = QString("");
        scoringMatrix = QString("");
        
        gm.scoreGapOpen = DEF_GAP_OPEN_SCORE;
        gm.scoreGapExtd = DEF_GAP_EXTD_SCORE;

        resultFilter = QString("");

        minScoreInPercent = DEF_PERCENT_OF_SCORE;

        searchType  = SmithWatermanSearchType_noSearch;
        strand      = StrandOption_DirectOnly;
        rangeType   = SmithWatermanRangeType_noRange;
    }
    QByteArray ptrn;

    QString algVersion;
    QString scoringMatrix;
    GapModel gm;
    QString resultFilter;

    int minScoreInPercent;      // c {0, 1, ..., 100}

    SmithWatermanSearchType searchType;                
    StrandOption strand;
    SmithWatermanRangeType  rangeType;
};

class U2VIEW_EXPORT SmithWatermanDialogController {
public:
    static void run(QWidget* p, 
                        ADVSequenceObjectContext* ctx,
                        SWDialogConfig* dialogConfig);
};

} // namespace

#endif

