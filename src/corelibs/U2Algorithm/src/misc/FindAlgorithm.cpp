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

#include <QtCore/QRegExp>

#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/Log.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Algorithm/DynTable.h>
#include <U2Algorithm/RollingArray.h>

#include "FindAlgorithm.h"

namespace U2 {

class StrandContext {
public:
    StrandContext(int width, int height, bool insDel, const char* p) 
        : dynTable(width, height, insDel), pattern(p)
    {
    }

    StrandContext( const char * data, int arr_size, const char * p ) //using rolling array only in subst mode
        : rollArr( data, arr_size ), pattern(p)
    {
    }
    
    StrandContext() : pattern(NULL) {}

    static quint64 estimateRamUsageForOneContext(int width, int height)
    {
        return DynTable::estimateTableSizeInBytes(width, height);
    }

    DynTable dynTable;
    RollingArray<char> rollArr;
    const char* pattern;
    FindAlgorithmResult res;
};

static bool isDirect(FindAlgorithmStrand s) {
    return s == FindAlgorithmStrand_Both || s == FindAlgorithmStrand_Direct;
}

static bool isComplement(FindAlgorithmStrand s) {
    return s == FindAlgorithmStrand_Both || s == FindAlgorithmStrand_Complement;
}

FindAlgorithmSettings::FindAlgorithmSettings(const QByteArray &pattern, FindAlgorithmStrand strand,
    DNATranslation *complementTT, DNATranslation *proteinTT, const U2Region &searchRegion,
    int maxErr, FindAlgorithmPatternSettings _patternSettings, bool ambBases, int _maxRegExpResult,
    int _maxResult2Find )
    : pattern( pattern ), strand( strand ), complementTT( complementTT ), proteinTT( proteinTT ),
    searchRegion( searchRegion ), maxErr( maxErr ), patternSettings( _patternSettings ),
    useAmbiguousBases( ambBases ), maxRegExpResult( _maxRegExpResult ),
    maxResult2Find( _maxResult2Find )
{

}

//TODO: in BothStrands&SingleShot mode it's impossible to find result on complement strand if there also a result on direct strand from the same pos!

static void findInAmino(    FindAlgorithmResultsListener* rl,
                            DNATranslation* aminoTT,
                            DNATranslation* complTT,
                            FindAlgorithmStrand strand,
                            bool insDel,
                            const char* seq,
                            const U2Region& range,
                            const char* pattern,
                            int patternLen,
                            int maxErr,
                            int& stopFlag,
                            int& percentsCompleted )
{
    SAFE_POINT( aminoTT->getSrcAlphabet( )->isNucleic( ) && aminoTT->getDstAlphabet( )->isAmino( ),
        "Invalid alphabet detected!", );
    
    int width =  patternLen + maxErr;
    int height = patternLen;

    QByteArray revPattern(pattern);
    TextUtils::reverse(revPattern.data(), patternLen);

    StrandContext context[] = {
        StrandContext(width, height, insDel, pattern), 
        StrandContext(width, height, insDel, pattern), 
        StrandContext(width, height, insDel, pattern), 
        StrandContext(width, height, insDel, revPattern.data()),
        StrandContext(width, height, insDel, revPattern.data()),
        StrandContext(width, height, insDel, revPattern.data())
    };

    int onePercentLen = range.length / 100;
    int leftTillPercent = onePercentLen;

    percentsCompleted = 0;
    int conStart = isDirect(strand)? 0 : 1;
    int conEnd =  isComplement(strand) ? 2 : 1;
    QByteArray complMap = complTT == NULL ? QByteArray() : complTT->getOne2OneMapper();
    int patternLenInNucl =  3 * patternLen;

    for (int i=range.startPos, end = range.endPos(), translStrand = 0;
        i<end-2 && !stopFlag; 
        i++, leftTillPercent--, translStrand = translStrand == 2 ? 0 : translStrand + 1) 
    {
        for (int ci = conStart; ci < conEnd; ci++) {
            StrandContext& ctx = context[3 * ci + translStrand];
            DynTable& dt = ctx.dynTable;
            const char* p = ctx.pattern;
            FindAlgorithmResult& res = ctx.res; 

            for (int j=0; j < patternLen; j++) { //TODO: optimize -> specialize loops
                char amino = ci == 0 ? 
                    aminoTT->translate3to1( seq[i], seq[i+1], seq[i+2]) :  //direct amino
                    aminoTT->translate3to1(complMap.at( (quint8) seq[i+2]), complMap.at((quint8) seq[i+1]), complMap.at( (quint8)seq[i]) ); //compl amino

                bool matched = (amino == p[j]);
                dt.match(j, matched);
            }
            int err = dt.getLast();
            if (!res.isEmpty() && (err > maxErr || (i-res.region.startPos) >= patternLenInNucl)) {
                rl->onResult(res);
                res.clear();
            }
            if (err <= maxErr) {
                int newLen = dt.getLastLen();
                newLen *= 3;
                if (res.isEmpty() || res.err > err || (res.err == err && newLen < res.region.length)) {
                    SAFE_POINT( newLen + 3  * maxErr >= patternLenInNucl, "Internal algorithm error!", );
                    int newStart = i - newLen + 3;
                    if (insDel || (range.contains(newStart) && range.contains(newStart + newLen - 1))) {//boundary check for mismatch mode
                        SAFE_POINT( insDel || newLen == patternLenInNucl, "Internal algorithm error!", );
                        SAFE_POINT( newStart >= range.startPos, "Internal algorithm error!", );
                        SAFE_POINT( newStart+newLen <= range.endPos( ), "Internal algorithm error!", );

                        res.region.startPos = newStart;
                        res.region.length = newLen;
                        res.err = err;
                        res.strand = (ci == 1) ? U2Strand::Complementary : U2Strand::Direct;
                        res.translation = true;
                    }
                } 
            }
            dt.shiftColumn();
            if (leftTillPercent == 0) {
                percentsCompleted = qMin(percentsCompleted+1,100);
                leftTillPercent = onePercentLen;
            }
        }//ci
    }
    for (int i=0; i < 6; i++) {
        if (!context[i].res.isEmpty()) { //todo: order by startpos?
            SAFE_POINT( insDel || context[i].res.region.length == patternLenInNucl,
                "Internal algorithm error: found region has invalid length!", );
            rl->onResult(context[i].res);
        }
    }
}
static void findInAmino_subst(  FindAlgorithmResultsListener *rl,
                                DNATranslation *aminoTT,
                                DNATranslation *complTT,
                                FindAlgorithmStrand strand,
                                const char *seq,
                                const U2Region &range,
                                const char *pattern,
                                int patternLen,
                                int maxErr,
                                int &stopFlag,
                                int &percentsCompleted )
{
    SAFE_POINT( NULL != complTT && NULL != aminoTT && aminoTT->getSrcAlphabet( )->isNucleic( )
        && aminoTT->getDstAlphabet( )->isAmino( ), "Invalid alphabet supplied!", );
    int patternLenInNucl =  3 * patternLen;
    if( range.length < patternLenInNucl ) {
        return;
    }

    QByteArray revPattern(pattern);

    QByteArray translatedPiece1( patternLen, 0 );
    QByteArray translatedPiece2( patternLen, 0 );
    QByteArray translatedPiece3( patternLen, 0 );

    int tail = range.length - patternLenInNucl - 2;
    tail = tail > 0 ? 0 : tail;
    aminoTT->translate( seq + range.startPos, patternLenInNucl + tail + 2, translatedPiece1.data(), patternLen );
    aminoTT->translate( seq + range.startPos+1, patternLenInNucl + tail + 1, translatedPiece2.data(), patternLen );
    aminoTT->translate( seq + range.startPos+2, patternLenInNucl + tail, translatedPiece3.data(), patternLen );

    QByteArray translatedPiece1c( patternLen, 0 );
    QByteArray translatedPiece2c( patternLen, 0 );
    QByteArray translatedPiece3c( patternLen, 0 );
    QByteArray compl_seq( patternLenInNucl + tail + 2, 0 );
    complTT->translate( seq + range.startPos, patternLenInNucl + tail + 2, compl_seq.data(), patternLenInNucl + tail + 2);
    TextUtils::reverse( compl_seq.data(), compl_seq.size() );

    int compl_tail = qBound( 0, int(range.length - patternLenInNucl), 2);
    int if0slot = 0 == compl_tail;
    int if1slot = 1 == compl_tail;
    int if2slot = 2 == compl_tail;

    aminoTT->translate( compl_seq.data()+compl_tail, patternLenInNucl, translatedPiece1c.data(), patternLen );
    aminoTT->translate( compl_seq.data()+2*if0slot+if2slot, patternLenInNucl-2*if0slot, translatedPiece2c.data(), patternLen );
    aminoTT->translate( compl_seq.data()+2*if1slot+if0slot, patternLenInNucl-if0slot-if1slot, translatedPiece3c.data(), patternLen );

    StrandContext context[] = {
        StrandContext(translatedPiece1, patternLen, pattern), 
        StrandContext(translatedPiece2, patternLen, pattern), 
        StrandContext(translatedPiece3, patternLen, pattern), 
        StrandContext(translatedPiece1c, patternLen, revPattern.data()),
        StrandContext(translatedPiece2c, patternLen, revPattern.data()),
        StrandContext(translatedPiece3c, patternLen, revPattern.data()),
    };

    int onePercentLen = range.length / 100;
    int leftTillPercent = onePercentLen;

    percentsCompleted = 0;
    int conStart = isDirect(strand)? 0 : 1;
    int conEnd =  isComplement(strand) ? 2 : 1;
    QByteArray complMap = complTT ? complTT->getOne2OneMapper() : QByteArray();

    for( int i = range.startPos, end = range.endPos(), translStrand = 0;
        i < end - patternLenInNucl + 1 && !stopFlag; 
        i++, leftTillPercent--, translStrand = translStrand == 2 ? 0 : translStrand + 1) 
    {
        for (int ci = conStart; ci < conEnd; ci++) {
            StrandContext & ctx = context[ci * 3 + translStrand];
            const char * p = ctx.pattern;
            FindAlgorithmResult & res = ctx.res; 
            bool match = true;
            int curErr = 0;
            for ( int j = 0; j < patternLen; j++ ) {
                char rollchar = ctx.rollArr.get(j);
                if( rollchar != p[j] && ++curErr > maxErr ) {
                    match = false;
                    break;
                }
            }
            if( i + patternLenInNucl + 2 < end ) {
                int wheree = i + patternLenInNucl;
                char c1 = ci ? complMap.at( (quint8)seq[wheree+2] ) : seq[wheree];
                char c2 = ci ? complMap.at((quint8)seq[wheree+1] ) : seq[wheree+1];
                char c3 = ci ? complMap.at((quint8)seq[wheree] ) : seq[wheree+2];
                char newchar = aminoTT->translate3to1( c1, c2, c3 );
                ci ? ctx.rollArr.push_front_pop_back( newchar ) : ctx.rollArr.push_back_pop_front( newchar );
            }

            if( match ) {
                res.region.startPos = i;
                res.region.length = patternLenInNucl;
                res.err = curErr;
                res.strand = (ci == 1) ? U2Strand::Complementary : U2Strand::Direct;
                res.translation = true;

                rl->onResult(res);
            }
            if (leftTillPercent == 0) {
                percentsCompleted = qMin(percentsCompleted+1,100);
                leftTillPercent = onePercentLen;
            }
        }//strand
    } //base pos
}

static char* createAmbiguousBaseMap() {

    // Source: http://www.ncbi.nlm.nih.gov/blast/fasta.shtml
    // Unknown symbol is zero: no match

    const int SIZE = 128;
    static char map[SIZE];

    for (int i = 0; i < SIZE; i++) {
        map[i] = 0x00;
    }

    map['A'] = 0x01; // Bitmask: 00000001
    map['C'] = 0x02; // Bitmask: 00000010
    map['G'] = 0x04; // Bitmask: 00000100
    map['T'] = 0x08; // Bitmask: 00001000
    map['U'] = 0x08; // Bitmask: 00001000
    map['M'] = 0x03; // Bitmask: 00000011
    map['R'] = 0x05; // Bitmask: 00000101
    map['W'] = 0x09; // Bitmask: 00001001
    map['S'] = 0x06; // Bitmask: 00000110
    map['Y'] = 0x0A; // Bitmask: 00001010
    map['K'] = 0x0C; // Bitmask: 00001100
    map['V'] = 0x07; // Bitmask: 00000111
    map['H'] = 0x0B; // Bitmask: 00001011
    map['D'] = 0x0D; // Bitmask: 00001101
    map['B'] = 0x0E; // Bitmask: 00001110
    map['N'] = 0x0F; // Bitmask: 00001111
    map['X'] = 0x0F; // Bitmask: 00001111

    return &map[0];
}


inline bool cmpAmbiguous( char a, char b){
    static char* charMap = createAmbiguousBaseMap();

    SAFE_POINT( a >= 0 && b >= 0, "Invalid characters supplied!", false );

    char c1 = charMap[uchar(a)];
    char c2 = charMap[uchar(b)];

    return c1 & c2;
}

inline bool match_pattern(const char* seq, const char* p, int start, int patternLen, int maxErr, int& curErr ) {
    bool match = true;
    curErr = 0;
    for ( int j = 0; j < patternLen; j++ ) {
        if( seq[start+j] != p[j] && ++curErr > maxErr ) {
            match = false;
            break;
        }
    }
    return match;
}

inline bool match_pattern_ambiguous(const char* seq, const char* p, int start, int patternLen, int maxErr, int& curErr ) {
    bool match = true;
    curErr = 0;
    for ( int j = 0; j < patternLen; j++ ) {
        if( !cmpAmbiguous(seq[start+j],p[j]) && ++curErr > maxErr ) {
            match = false;
            break;
        }
    }
    return match;
}

// reverses found result if it's located on the reverse complement strand
inline void prepareResultPosition( int regionStart, int regionLength, int &foundStart,
    int foundLength, U2Strand resultStrand )
{
    foundStart = resultStrand.isCompementary( )
        ? regionStart + regionLength - foundStart - foundLength
        : regionStart + foundStart;
}

static void sendResultToListener( int resultStartPos, int resultLength, U2Strand resultStrand,
    FindAlgorithmResultsListener *rl )
{
    FindAlgorithmResult res;
    res.region.startPos = resultStartPos;
    res.region.length = resultLength;
    res.strand = resultStrand;

    rl->onResult(res);
}

static void regExpSearch(   const QString &refSequence,
                            const QRegExp &regExp,
                            const U2Strand &searchStrand,
                            const U2Region &sequenceRange,
                            int maxResultLen,
                            int currentStrand,
                            int totalStrandCount,
                            bool refSeqIsAminoTranslation,
                            int &percentsCompleted,
                            int &stopFlag,
                            FindAlgorithmResultsListener *rl )
{
    int foundStartPos = 0;
    while ( 0 == stopFlag
        && -1 != ( foundStartPos = regExp.indexIn( refSequence, foundStartPos ) ) )
    {
        // remember that there are a few iterations, so a single one yields
        // 1 / @conEnd of total progress
        percentsCompleted = ( 100 * foundStartPos * ( currentStrand + 1 ) )
            / ( sequenceRange.length * totalStrandCount );

        const int foundLength = regExp.matchedLength( );
        if ( maxResultLen >= foundLength ) {
            int resultStartPos = refSeqIsAminoTranslation ? foundStartPos * 3 : foundStartPos;
            const int resultLen = refSeqIsAminoTranslation ? foundLength * 3 : foundLength;
            prepareResultPosition( sequenceRange.startPos, sequenceRange.length, resultStartPos, resultLen,
                searchStrand );
            sendResultToListener( resultStartPos, resultLen, searchStrand, rl );
        }

        // try to find smaller substrings starting from the same position
        int substrLength = foundLength - 1;
        while ( 0 == stopFlag && 0 < substrLength
            && foundStartPos == ( regExp.indexIn( refSequence.left( foundStartPos + substrLength ), foundStartPos ) ) )
        {
            const int foundSubstrLength = regExp.matchedLength( );
            if ( maxResultLen >= foundSubstrLength ) {
                int resultStartPos = refSeqIsAminoTranslation ? foundStartPos * 3 : foundStartPos;
                const int resultLen = refSeqIsAminoTranslation ? foundSubstrLength * 3 : foundSubstrLength;
                prepareResultPosition( sequenceRange.startPos, sequenceRange.length, resultStartPos, resultLen,
                    searchStrand );
                sendResultToListener( resultStartPos, resultLen, searchStrand, rl );
            }
            substrLength = foundSubstrLength - 1;
        }

        ++foundStartPos;
    }
}

static void findInAmino_regExp( FindAlgorithmResultsListener *rl,
                                DNATranslation *aminoTT,
                                DNATranslation *complTT,
                                FindAlgorithmStrand strand,
                                const char *seq,
                                const U2Region &range,
                                const char *pattern,
                                int maxRegExpResult,
                                int &stopFlag,
                                int &percentsCompleted )
{
    percentsCompleted = 0;

    int conStart = isDirect( strand )? 0 : 1;
    int conEnd =  isComplement( strand ) ? 2 : 1;

    QRegExp regExp( pattern );
    SAFE_POINT( regExp.isValid( ), "Invalid regular expression supplied!", );

    int maxAminoResult = maxRegExpResult * 3;

    for ( int ci = conStart; ci < conEnd; ++ci ) {
        QString translation;
        QByteArray tmp( range.length + 1, 0 );
        char *rawTranslation = tmp.data( );
        U2Strand resultStrand;
        const int translationLen = range.length / 3;

        if ( ci == 1 ) { // complementary
            TextUtils::translate( complTT->getOne2OneMapper( ), seq + range.startPos, range.length,
                rawTranslation );
            TextUtils::reverse( rawTranslation, range.length );
            aminoTT->translate( rawTranslation, range.length );
            translation = QString( QByteArray( rawTranslation, translationLen ) );
            resultStrand = U2Strand::Complementary;
        } else { // direct
            aminoTT->translate( seq + range.startPos, range.length, rawTranslation, range.length );
            translation = QString( QByteArray( rawTranslation, translationLen ) );
            resultStrand = U2Strand::Direct;
        }

        regExpSearch( translation, regExp, resultStrand, range, maxAminoResult, ci, conEnd, true,
            percentsCompleted, stopFlag, rl );
    }
}

static void findRegExp( FindAlgorithmResultsListener *rl,
                        DNATranslation *aminoTT,
                        DNATranslation *complTT,
                        FindAlgorithmStrand strand,
                        const char *seq,
                        const U2Region &range,
                        const char *pattern,
                        int maxRegExpResult,
                        int &stopFlag,
                        int &percentsCompleted )
{
    if ( NULL != aminoTT ) {
        findInAmino_regExp( rl, aminoTT, complTT, strand, seq, range, pattern,
            maxRegExpResult, stopFlag, percentsCompleted );
        return;
    }

    percentsCompleted = 0;

    const int conStart = isDirect( strand ) ? 0 : 1;
    const int conEnd =  isComplement( strand ) ? 2 : 1;

    QRegExp regExp( pattern );
    SAFE_POINT( regExp.isValid( ), "Invalid regular expression supplied!", );

    for ( int ci = conStart; ci < conEnd; ++ci ) {
        QString substr;
        QByteArray tmp( range.length + 1, 0 );
        char *complSeq = NULL;
        U2Strand resultStrand;

        if ( ci == 1 ) { // complementary
            complSeq = tmp.data( );
            TextUtils::translate( complTT->getOne2OneMapper( ), seq + range.startPos, range.length,
                complSeq );
            TextUtils::reverse( complSeq, range.length );
            substr = QString( QByteArray( complSeq, range.length ) );
            resultStrand = U2Strand::Complementary;
        } else { // direct
            substr = QString( QByteArray( seq + range.startPos, range.length ) );
            resultStrand = U2Strand::Direct;
        }

        regExpSearch( substr, regExp, resultStrand, range, maxRegExpResult, ci, conEnd, false,
            percentsCompleted, stopFlag, rl );
    }
}

static void find_subst( FindAlgorithmResultsListener* rl,
                        DNATranslation* aminoTT,
                        DNATranslation* complTT,
                        FindAlgorithmStrand strand,
                        const char* seq,
                        const U2Region& range,
                        const char* pattern,
                        int patternLen,
                        bool useAmbiguousBases,
                        int maxErr,
                        int& stopFlag,
                        int& percentsCompleted )
{
    SAFE_POINT( NULL == complTT || complTT->isOne2One( ), "Invalid translation supplied!", );

    if (aminoTT != NULL) {
        findInAmino_subst( rl, aminoTT, complTT, strand, seq, range, pattern, patternLen, maxErr,
            stopFlag, percentsCompleted );
        return;
    }
    if( range.length - patternLen < 0 ) {
        return;
    }
    char* complPattern = NULL;
    QByteArray tmp;
    if (isComplement(strand)) {
        SAFE_POINT( NULL != complTT, "Invalid translation supplied!", );
        tmp.resize(patternLen);
        complPattern = tmp.data();
        TextUtils::translate(complTT->getOne2OneMapper(), pattern, patternLen, complPattern);
        TextUtils::reverse(complPattern, patternLen);
    }

    StrandContext context[] = {
        StrandContext(0, 0, false, pattern),
        StrandContext(0, 0, false, complPattern)
    };
    
    int onePercentLen = range.length/100;
    int leftTillPercent = onePercentLen;
    percentsCompleted = 0;

    int conStart = isDirect(strand)? 0 : 1;
    int conEnd =  isComplement(strand) ? 2 : 1;
    SAFE_POINT( conStart < conEnd, "Internal algorithm error: incorrect strand order!", );
    for (int i=range.startPos, end = range.endPos(); i < end - patternLen + 1 && !stopFlag; i++, leftTillPercent--) {
        for (int ci = conStart; ci < conEnd; ci++) {
            StrandContext& ctx = context[ci];
            const char* p = ctx.pattern;
            FindAlgorithmResult& res = ctx.res; 
            
            bool match = true;
            int curErr = 0;
            if (useAmbiguousBases) {
                match = match_pattern_ambiguous(seq, p, i, patternLen, maxErr, curErr);
            } else {
                match = match_pattern(seq, p, i, patternLen, maxErr, curErr);
            }
            if( match ) {
                res.region.startPos = i;
                res.region.length = patternLen;
                res.err = curErr;
                res.strand = (ci == 1) ? U2Strand::Complementary : U2Strand::Direct;

                rl->onResult(res);
            }

            if (leftTillPercent == 0) {
                percentsCompleted = qMin(percentsCompleted+1,100);
                leftTillPercent = onePercentLen;
            }
        }//strand
    } //base pos
}

// value 12 - standart "out of memory" error code in linux
// error in FindAlgorithmResult also means number of mismatches, therefore positive values can cause confusion
const int FindAlgorithmResult::NOT_ENOUGH_MEMORY_ERROR = -12;

void FindAlgorithm::find(
                         FindAlgorithmResultsListener* rl,
                         DNATranslation* aminoTT, // if aminoTT!=NULL -> pattern must contain amino data and sequence must contain DNA data
                         DNATranslation* complTT, // if complTT!=NULL -> sequence is complemented before comparison with pattern
                         FindAlgorithmStrand strand, // if not direct there complTT must not be NULL
                         FindAlgorithmPatternSettings patternSettings,
                         bool useAmbiguousBases,
                         const char* seq,
                         int seqLen,
                         const U2Region& range,
                         const char* pattern,
                         int patternLen,
                         int maxErr,
                         int maxRegExpResult,
                         int& stopFlag,
                         int& percentsCompleted)
{
    Q_UNUSED(seqLen);
    SAFE_POINT( NULL == complTT || complTT->isOne2One( ), "Invalid translation supplied!", );
    SAFE_POINT( patternLen > maxErr, "Invalid maximum error count supplied!", );

    if ( patternSettings == FindAlgorithmPatternSettings_RegExp ) {
        findRegExp( rl, aminoTT, complTT, strand, seq, range, pattern, maxRegExpResult, stopFlag,
            percentsCompleted );
        return;
    }

    if( patternSettings == FindAlgorithmPatternSettings_Subst ) {
        find_subst( rl, aminoTT, complTT, strand, seq, range, pattern, patternLen,
            useAmbiguousBases, maxErr, stopFlag, percentsCompleted );
        return;
    }

    bool insDel = (patternSettings == FindAlgorithmPatternSettings_InsDel);

    if (aminoTT != NULL) {
        findInAmino(rl, aminoTT, complTT, strand, insDel, seq, range, pattern, patternLen, maxErr,
            stopFlag, percentsCompleted );
        return;
    }
    char* complPattern = NULL;
    QByteArray tmp;
    if (isComplement(strand)) {
        SAFE_POINT( NULL != complTT, "Invalid translation supplied!", );
        tmp.resize(patternLen);
        complPattern = tmp.data();
        TextUtils::translate(complTT->getOne2OneMapper(), pattern, patternLen, complPattern);
        TextUtils::reverse(complPattern, patternLen);
    }

    int width =  patternLen + maxErr;
    int height = patternLen;

    if ( width > INT_MAX / height ) {
        const FindAlgorithmResult result(FindAlgorithmResult::NOT_ENOUGH_MEMORY_ERROR);
        rl->onResult( result );
        return;
    }

    try {
        StrandContext context[] = {
            StrandContext(width, height, insDel, pattern),
            StrandContext(width, height, insDel, complPattern)
        };

        int onePercentLen = range.length/100;
        int leftTillPercent = onePercentLen;
        percentsCompleted = 0;

        int conStart = isDirect(strand)? 0 : 1;
        int conEnd =  isComplement(strand) ? 2 : 1;
        SAFE_POINT( conStart < conEnd, "Internal algorithm error: incorrect strand order!", );
        for (int i=range.startPos, end = range.endPos(); i < end && !stopFlag; i++, leftTillPercent--) {
            for (int ci = conStart; ci < conEnd; ci++) {
                StrandContext& ctx = context[ci];
                DynTable& dt = ctx.dynTable;
                const char* p = ctx.pattern;
                FindAlgorithmResult& res = ctx.res;
                for (int j=0; j<patternLen; j++) {
                    bool matched = seq[i] == p[j];
                    dt.match(j, matched);
                }

                int err = dt.getLast();

                if (!res.isEmpty() && (err > maxErr || (i-res.region.startPos) >= patternLen)) {
                    rl->onResult(res);
                    res.clear();
                }

                if (err <= maxErr) {
                    int newLen = dt.getLastLen();
                    if (res.isEmpty() || res.err > err || (res.err == err && newLen < res.region.length)) {
                        int newStart = i-newLen+1;
                        if (insDel || (range.contains(newStart) && range.contains(newStart + newLen - 1))) {//boundary check for mismatch mode
                            SAFE_POINT( insDel || newLen == patternLen, "Internal algorithm error!", );
                            SAFE_POINT( newStart >= range.startPos, "Internal algorithm error!", );
                            SAFE_POINT( newStart + newLen <= range.endPos( ), "Internal algorithm error!", );

                            res.region.startPos = newStart;
                            res.region.length = newLen;
                            res.err = err;
                            res.strand = (ci == 1) ? U2Strand::Complementary : U2Strand::Direct;
                            res.translation = (aminoTT != NULL) ? true : false;
                        }
                    }
                }

                dt.shiftColumn();
                if (leftTillPercent == 0) {
                    percentsCompleted = qMin(percentsCompleted+1,100);
                    leftTillPercent = onePercentLen;
                }
            }//strand
        } //base pos

        for (int i=0; i<2; i++) {
            if (!context[i].res.isEmpty()) { //todo: order by startpos?
                SAFE_POINT( insDel || context[i].res.region.length == patternLen,
                    "Internal algorithm error: found region has invalid length!", );
                rl->onResult(context[i].res);
            }
        }

    }
    catch (std::bad_alloc&) {
        const FindAlgorithmResult result(FindAlgorithmResult::NOT_ENOUGH_MEMORY_ERROR);
        rl->onResult( result );
        return;
    }
}

int FindAlgorithm::estimateRamUsageInMbytes(const FindAlgorithmPatternSettings patternSettings,
    const bool searchInAminoTT, const int patternLength, const int maxError)
{
    const int bytesToMbytesFactor = 1048576;
    quint64 ramUsage = 0;

    if(FindAlgorithmPatternSettings_InsDel == patternSettings) {
        ramUsage = 2 * StrandContext::estimateRamUsageForOneContext(patternLength + maxError,
                                                                        patternLength);
        if(searchInAminoTT) {
            ramUsage *= 4;
        }
    } else if(FindAlgorithmPatternSettings_Subst == patternSettings && searchInAminoTT)
        ramUsage = 7 * patternLength * sizeof(char);

    return ramUsage / bytesToMbytesFactor;
}

}//namespace
