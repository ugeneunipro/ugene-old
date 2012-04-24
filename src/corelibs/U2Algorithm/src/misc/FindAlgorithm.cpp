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

#include "FindAlgorithm.h"

#include <QRegExp>

#include <U2Core/Log.h>
#include <U2Core/TextUtils.h>
#include <U2Algorithm/DynTable.h>
#include <U2Algorithm/RollingArray.h>

#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>

#include <assert.h>

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

//TODO: in BothStrands&SingleShot mode it's impossible to find result on complement strand if there also a result on direct strand from the same pos!

static void findInAmino(
                     FindAlgorithmResultsListener* rl, 
                     DNATranslation* aminoTT, 
                     DNATranslation* complTT, 
                     FindAlgorithmStrand strand,
                     bool insDel,
                     const char* seq, 
                     const U2Region& range,  
                     const char* pattern, 
                     int patternLen, 
                     bool singleShot, 
                     int maxErr, 
                     int& stopFlag, 
                     int& percentsCompleted, 
                     int& currentPos)
{
    assert(aminoTT->getSrcAlphabet()->isNucleic() && aminoTT->getDstAlphabet()->isAmino());
    
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
    currentPos = 0;
    int conStart = isDirect(strand)? 0 : 1;
    int conEnd =  isComplement(strand) ? 2 : 1;
    QByteArray complMap = complTT == NULL ? QByteArray() : complTT->getOne2OneMapper();
    int patternLenInNucl =  3 * patternLen;

    for (int i=range.startPos, end = range.endPos(), translStrand = 0;
        i<end-2 && !stopFlag; 
        i++, leftTillPercent--, translStrand = translStrand == 2 ? 0 : translStrand + 1) 
    {
        bool sShot_retflag = false;
        currentPos = i+2;
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
                if (singleShot) {
                    if( insDel ) {
                        sShot_retflag = true;
                    } else {
                        return;
                    }
                }
            }
            if (err <= maxErr) {
                int newLen = dt.getLastLen();
                newLen *= 3;
                if (res.isEmpty() || res.err > err || (res.err == err && newLen < res.region.length)) {
                    assert(newLen + 3  * maxErr >= patternLenInNucl);
                    int newStart = i - newLen + 3;
                    if (insDel || (range.contains(newStart) && range.contains(newStart + newLen - 1))) {//boundary check for mismatch mode
                        assert(insDel || newLen == patternLenInNucl);
                        assert(newStart >= range.startPos);
                        assert(newStart+newLen <= range.endPos());

                        res.region.startPos = newStart;
                        res.region.length = newLen;
                        res.err = err;
                        res.strand = (ci == 1) ? U2Strand::Complementary : U2Strand::Direct;
                        res.translation = true;
                        if( !insDel && singleShot ) {
                            rl->onResult( res );
                            res.clear();
                            sShot_retflag = true;
                        }
                    }
                } 
            }
            dt.shiftColumn();
            if (leftTillPercent == 0) {
                percentsCompleted = qMin(percentsCompleted+1,100);
                leftTillPercent = onePercentLen;
            }
        }//ci
        if( sShot_retflag ) {
            return;
        }
    }
    for (int i=0; i < 6; i++) {
        if (!context[i].res.isEmpty()) { //todo: order by startpos?
            assert(insDel || context[i].res.region.length == patternLenInNucl);
            rl->onResult(context[i].res);
        }
    }
}
static void findInAmino_subst(
                              FindAlgorithmResultsListener* rl, 
                              DNATranslation* aminoTT, 
                              DNATranslation* complTT, 
                              FindAlgorithmStrand strand,
                              const char* seq, 
                              const U2Region& range,  
                              const char* pattern, 
                              int patternLen, 
                              bool singleShot, 
                              int maxErr,
                              int& stopFlag, 
                              int& percentsCompleted, 
                              int& currentPos) 
{
    assert(complTT && aminoTT && aminoTT->getSrcAlphabet()->isNucleic() && aminoTT->getDstAlphabet()->isAmino());
    int patternLenInNucl =  3 * patternLen;
    if( range.length < patternLenInNucl ) {
        currentPos = range.endPos();
        return;
    }

    QByteArray revPattern(pattern);
    //    TextUtils::reverse(revPattern.data(), patternLen);

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
    currentPos = 0;
    int conStart = isDirect(strand)? 0 : 1;
    int conEnd =  isComplement(strand) ? 2 : 1;
    QByteArray complMap = complTT ? complTT->getOne2OneMapper() : QByteArray();

    for( int i = range.startPos, end = range.endPos(), translStrand = 0;
        i < end - patternLenInNucl + 1 && !stopFlag; 
        i++, leftTillPercent--, translStrand = translStrand == 2 ? 0 : translStrand + 1) 
    {
        currentPos = i;
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
                currentPos += 1; 
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
        if( singleShot ) {
            for( int j = conStart; j < conEnd; ++j ) {
                int cur_ctx = j * 3 + translStrand;
                if( !context[cur_ctx].res.isEmpty() ) {
                    context[cur_ctx].res.clear();
                    return;
                }
            }
        }
    } //base pos
    currentPos = range.endPos();
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

    assert( a >= 0 && b >= 0 );

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

static void findInAmino_regExp(FindAlgorithmResultsListener* rl, 
							  DNATranslation* aminoTT, 
							  DNATranslation* complTT, 
							  FindAlgorithmStrand strand,
							  const char* seq, 
							  qint64 seqLen,
							  const U2Region& range,  
							  const char* pattern, 
							  int patternLen, 
							  bool singleShot,
							  int maxRegExpResult,
							  int& stopFlag, 
							  int& percentsCompleted, 
							  int& currentPos,
							  int currentLen)
{
	int onePercentLen = range.length/100;
	int leftTillPercent = onePercentLen;
	percentsCompleted = 0;
	currentPos = range.startPos;

	int conStart = isDirect(strand)? 0 : 1;
	int conEnd =  isComplement(strand) ? 2 : 1;
	bool hasResult = false;

	QRegExp regExp(pattern);

	QByteArray complMap = complTT == NULL ? QByteArray() : complTT->getOne2OneMapper();
	int maxAminoResult = maxRegExpResult * 3;

	QString substrDirect;
	QString substrComp;

	for(qint64 i = range.startPos;i<range.endPos()-2 && !stopFlag;++i,--leftTillPercent,++currentPos){
		substrDirect.clear();
		substrComp.clear();
		for(qint64 j = i+currentLen; j<range.endPos()-2 && j - i < maxAminoResult && !stopFlag ;j+=3,++currentLen){
			for(int ci = conStart;ci<conEnd;++ci){
				if(ci == 1){ // complementary					
					substrComp.push_front(aminoTT->translate3to1(complMap.at( (quint8) seq[j+2]), complMap.at((quint8) seq[j+1]), complMap.at( (quint8)seq[j]) )); //compl amino
					if(regExp.exactMatch(substrComp) ){
						FindAlgorithmResult res ; 
						res.region.startPos = i;
						res.region.length = j-i+3;					
						res.strand = U2Strand::Complementary ;

						rl->onResult(res);
						hasResult = true;
					}
				}
				else{ // direct
					substrDirect.push_back(aminoTT->translate3to1( seq[j], seq[j+1], seq[j+2]));  //direct amino
					if(regExp.exactMatch(substrDirect) ){
						FindAlgorithmResult res ; 
						res.region.startPos = i;
						res.region.length = j-i+3;					
						res.strand = U2Strand::Direct;

						rl->onResult(res);
						hasResult = true;
					}
				}
				
			}
			if(singleShot && hasResult){
				return;
			}
		}
		if (leftTillPercent == 0) {
			percentsCompleted = qMin(percentsCompleted+1,100);
			leftTillPercent = onePercentLen;
		}
		currentLen = 0;
	}
	currentPos = range.endPos();
}


static void findRegExp(FindAlgorithmResultsListener* rl, 
					   DNATranslation* aminoTT, 
					   DNATranslation* complTT, 
					   FindAlgorithmStrand strand,
					   const char* seq, 
					   qint64 seqLen,
					   const U2Region& range,  
					   const char* pattern, 
					   int patternLen, 
					   bool singleShot,
					   int maxRegExpResult,
					   int& stopFlag, 
					   int& percentsCompleted, 
					   int& currentPos,
					   int currentLen)
{

	if(aminoTT != NULL){
		findInAmino_regExp(rl, aminoTT, complTT, strand, seq, seqLen, range, pattern, patternLen,
			singleShot,maxRegExpResult, stopFlag, percentsCompleted, currentPos,currentLen);
		return;
	}
	
	int onePercentLen = range.length/100;
	int leftTillPercent = onePercentLen;
	percentsCompleted = 0;
	currentPos = range.startPos;

	int conStart = isDirect(strand)? 0 : 1;
	int conEnd =  isComplement(strand) ? 2 : 1;
	bool hasResult = false;

	QRegExp regExp(pattern);

	for(qint64 i = range.startPos;i<range.endPos() && !stopFlag;++i,--leftTillPercent,++currentPos){
		for(qint64 j = i+currentLen; j<range.endPos() && j - i < maxRegExpResult && !stopFlag ;++j,++currentLen){
			for(int ci = conStart; ci<conEnd; ++ci){
				QString substr ;
				QByteArray tmp(maxRegExpResult + 1,0);
				char* complSeq = NULL;

				if (ci == 1) { // complementary 					
					complSeq = tmp.data();
					TextUtils::translate(complTT->getOne2OneMapper(), seq+i, j-i+1, complSeq);
					TextUtils::reverse(complSeq, j-i+1);
					substr = QString(QByteArray(complSeq,j-i+1));
				}
				else{ // direct 
					substr = QString(QByteArray(seq+i,j-i+1));
				}
				if(regExp.exactMatch(substr)){
					FindAlgorithmResult res ; 
					res.region.startPos = i;
					res.region.length = j-i+1;					
					res.strand = (ci == 1) ? U2Strand::Complementary : U2Strand::Direct;

					rl->onResult(res);
					hasResult = true;
				}	
			}
			if(singleShot && hasResult){
				return;
			}
		}
		if (leftTillPercent == 0) {
			percentsCompleted = qMin(percentsCompleted+1,100);
			leftTillPercent = onePercentLen;
		}
		currentLen = 0;
	}	
	currentPos = range.endPos();
}


static void find_subst(
    FindAlgorithmResultsListener* rl, 
    DNATranslation* aminoTT, 
    DNATranslation* complTT, 
    FindAlgorithmStrand strand,
    const char* seq, 
    const U2Region& range,  
    const char* pattern, 
    int patternLen, 
    bool singleShot,
    bool useAmbiguousBases,
    int maxErr,
    int& stopFlag, 
    int& percentsCompleted, 
    int& currentPos) 
{
    assert(complTT == NULL || complTT->isOne2One());
    
    if (aminoTT != NULL) {
        findInAmino_subst(rl, aminoTT, complTT, strand, seq, range, 
            pattern, patternLen, singleShot, maxErr, stopFlag, percentsCompleted, currentPos);
        return;
    }
    if( range.length - patternLen < 0 ) {
        currentPos = range.endPos();
        return;
    }
    char* complPattern = NULL;
    QByteArray tmp;
    if (isComplement(strand)) {
        assert(complTT!=NULL);
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
    currentPos = range.startPos;

    int conStart = isDirect(strand)? 0 : 1;
    int conEnd =  isComplement(strand) ? 2 : 1;
    assert(conStart < conEnd);
    for (int i=range.startPos, end = range.endPos(); i < end - patternLen + 1 && !stopFlag; i++, leftTillPercent--) {
        currentPos = i;
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
                ++currentPos;
                res.region.startPos = i;
                res.region.length = patternLen;
                res.err = curErr;
                res.strand = (ci == 1) ? U2Strand::Complementary : U2Strand::Direct;

                rl->onResult(res);
//                res.clear();
            }

            if (leftTillPercent == 0) {
                percentsCompleted = qMin(percentsCompleted+1,100);
                leftTillPercent = onePercentLen;
            }
        }//strand
        if( singleShot ) {
            for( int j = conStart; j < conEnd; ++j ) {
                if( !context[j].res.isEmpty() ) {
                    return;
                }
            }
        }
    } //base pos
    currentPos = range.endPos();
}

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
                         bool singleShot, 
                         int maxErr,
						 int maxRegExpResult,
                         int& stopFlag, 
                         int& percentsCompleted, 
                         int& currentPos,
						 int currentLen) 
{
    Q_UNUSED(seqLen);
    assert(complTT == NULL || complTT->isOne2One());
    assert(patternLen > maxErr);    

	if(patternSettings == FindAlgorithmPatternSettings_RegExp){
		findRegExp(rl, aminoTT, complTT, strand, seq, seqLen, range, pattern, patternLen,
			singleShot,maxRegExpResult, stopFlag, percentsCompleted, currentPos,currentLen);
		return;
	}

    if( patternSettings == FindAlgorithmPatternSettings_Subst ) {
        find_subst( rl, aminoTT, complTT, strand, seq, range, pattern, patternLen,
			singleShot, useAmbiguousBases, maxErr, stopFlag, percentsCompleted, currentPos );
        return;
    }

	bool insDel = (patternSettings == FindAlgorithmPatternSettings_InsDel);

    if (aminoTT != NULL) {
        findInAmino(rl, aminoTT, complTT, strand, insDel, seq, range, 
            pattern, patternLen, singleShot, maxErr, stopFlag, percentsCompleted, currentPos);
        return;
    }
    char* complPattern = NULL;
    QByteArray tmp;
    if (isComplement(strand)) {
        assert(complTT!=NULL);
        tmp.resize(patternLen);
        complPattern = tmp.data();
        TextUtils::translate(complTT->getOne2OneMapper(), pattern, patternLen, complPattern);
        TextUtils::reverse(complPattern, patternLen);
    }

    int width =  patternLen + maxErr;
    int height = patternLen;

    StrandContext context[] = {
        StrandContext(width, height, insDel, pattern), 
        StrandContext(width, height, insDel, complPattern)
    };

    int onePercentLen = range.length/100;
    int leftTillPercent = onePercentLen;
    percentsCompleted = 0;
    currentPos = range.startPos;

    int conStart = isDirect(strand)? 0 : 1;
    int conEnd =  isComplement(strand) ? 2 : 1;
    assert(conStart < conEnd);
    for (int i=range.startPos, end = range.endPos(); i < end && !stopFlag; i++, leftTillPercent--) {
        currentPos = i;
        bool sShot_retflag = false;
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
                if (singleShot) {
                    if( insDel ) {
                        sShot_retflag = true;
                    } else {
                        return;
                    }
                }
            }

            if (err <= maxErr) {
                int newLen = dt.getLastLen();
                if (res.isEmpty() || res.err > err || (res.err == err && newLen < res.region.length)) {
                    //                    assert(newLen + maxErr >= patternLen);
                    int newStart = i-newLen+1;
                    if (insDel || (range.contains(newStart) && range.contains(newStart + newLen - 1))) {//boundary check for mismatch mode                      
                        assert(insDel || newLen == patternLen);
                        assert(newStart >= range.startPos);
                        assert(newStart+newLen <= range.endPos());

                        res.region.startPos = newStart;
                        res.region.length = newLen;
                        res.err = err;
                        res.strand = (ci == 1) ? U2Strand::Complementary : U2Strand::Direct;
                        res.translation = (aminoTT != NULL) ? true : false; 
                        if( !insDel && singleShot ) {
                            rl->onResult( res );
                            res.clear();
                            sShot_retflag = true;
                        }
                    }
                } 
            }

            dt.shiftColumn();
            if (leftTillPercent == 0) {
                percentsCompleted = qMin(percentsCompleted+1,100);
                leftTillPercent = onePercentLen;
            }
        }//strand
        if( sShot_retflag ) {
            return;
        }
    } //base pos

    for (int i=0; i<2; i++) {
        if (!context[i].res.isEmpty()) { //todo: order by startpos?
            assert(insDel || context[i].res.region.length == patternLen);
            rl->onResult(context[i].res);
        }
    }
}






}//namespace
