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

#include <QtCore/QByteArray>

#include <U2Core/L10n.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>

#include "uHMMFormatReader.h"

#include <math.h>

using namespace U2;

const int BUF_SZ        = 1024;
const int BAD_READ      = -1;
const int EMPTY_READ    = 0;
const char TERM_SYM     = '\0';

enum HMMERHeaderTags {
    BAD_TAG = -1,
    NAME,
    ACC,
    DESC,
    LENG,
    ALPH,
    RF,
    CS,
    MAP,
    DATE,
    COM,
    NSEQ,
    EFFN,
    CKSUM,
    STATS,
    GA,
    TC,
    NC,
    NULE, // in HMMER2 only
    HMM,
    BM,
    SM
}; // HMMERHeaderTags

static QMap< QByteArray, HMMERHeaderTags > getHeaderTagsMap() {
    static QMap< QByteArray, HMMERHeaderTags > ret;
    if (ret.isEmpty()) {
        ret["NAME"]     = NAME;
        ret["ACC"]      = ACC;
        ret["DESC"]     = DESC;
        ret["LENG"]     = LENG;
        ret["ALPH"]     = ALPH;
        ret["RF"]       = RF;
        ret["CS"]       = CS;
        ret["MAP"]      = MAP;
        ret["DATE"]     = DATE;
        ret["COM"]      = COM;
        ret["NSEQ"]     = NSEQ;
        ret["EFFN"]     = EFFN;
        ret["CKSUM"]    = CKSUM;
        ret["STATS"]    = STATS;
        ret["GA"]       = GA;
        ret["TC"]       = TC;
        ret["NC"]       = NC;
        ret["NULE"]     = NULE;
        ret["HMM"]      = HMM;
        ret["BM"]       = BM;
        ret["SM"]       = SM;
    }
    return ret;
}

static float h2ascii2prob( const QByteArray & curToken , float null ) {
    float ret = .0;
    if( '*' == curToken.at( 0 ) ) {
        ret = 0.0;
    } else {
        bool ok = false;
        int num = curToken.toInt( &ok );
        if( !ok ) {
            throw UHMMFormatReader::UHMMFormatReaderException(
                UHMMFormatReader::tr( "Number expected in NULE line. %1 found" ).arg( QString( curToken ) ) );
        }
        ret = ( null ) * exp( num * 0.00069314718 );
    }
    return ret;
}

static void checkReadThrowException( int readBytes ) {
    if( BAD_READ == readBytes ) {
        throw UHMMFormatReader::UHMMFormatReaderException( 
            UHMMFormatReader::HMM_FORMAT_READER_ERROR_PREFIX + UHMMFormatReader::READ_FAILED );
    }
}

static void getTagValue( const QByteArray& ln, QByteArray& tag, QByteArray& val ) {
    QString line( ln.trimmed() );
    QStringList words = line.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
    
    if( 1 >= words.size() ) {
        if( words.size() == 1 && getHeaderTagsMap().value(words.at(0).toAscii(), BAD_TAG) == NAME ) {
            tag = words.at(0).toAscii();
            val = "hmm_profile";
        } else {
            throw UHMMFormatReader::UHMMFormatReaderException(UHMMFormatReader::tr("bad_line_in_header_section:%1").arg(QString(ln)));
        }
    } else {
        tag = words.first().toAscii();
        val = ln.mid( tag.size() ).trimmed();
        assert( !tag.isEmpty() );
        if( val.isEmpty() ) {
            throw UHMMFormatReader::UHMMFormatReaderException(UHMMFormatReader::tr("Empty value in header line:%1").arg(QString(ln)));
        }
    }
}

static void setInteger( int& num, const QByteArray& numStr ) {
    bool ok = false;
    int ret = numStr.toInt( &ok );
    if( !ok ) {
        throw UHMMFormatReader::UHMMFormatReaderException( 
            UHMMFormatReader::tr( "cannot_parse_integer_number_from_string:%1" ).arg( QString(numStr) ) );
    } else {
        num = ret;
    }
}

static void setFloat( float& num, const QByteArray& numStr ) {
    bool ok = false;
    float ret = numStr.toFloat( &ok );
    if( !ok ) {
        throw UHMMFormatReader::UHMMFormatReaderException( 
            UHMMFormatReader::tr( "cannot_parse_float_number_from_string:%1" ).arg( QString(numStr) ) );
    } else {
        num = ret;
    }
}

static void set2Floats( float& f1, float& f2, const QByteArray& str ) {
    QString line(str.trimmed());
    if (line.endsWith(";"))  { //PFAM compatibilty fix
        line = line.mid(0, line.length()-1);
    }

    QStringList words = line.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );

    if( 2 != words.size() ) {
        throw UHMMFormatReader::UHMMFormatReaderException( 
            UHMMFormatReader::tr( "cannot_parse_2_float_numbers_in_str:%1" ).arg( QString(str) ) );
    }
    setFloat( f1, words.at( 0 ).toAscii() );
    setFloat( f2, words.at( 1 ).toAscii() );
}

static void setUInteger( uint32_t& num, const QByteArray& numStr ) {
    bool ok = false;
    uint32_t ret = numStr.toUInt( &ok );
    if( !ok ) {
        throw UHMMFormatReader::UHMMFormatReaderException( 
            UHMMFormatReader::tr( "cannot_parse_uinteger_number_from_string:%1" ).arg( QString(numStr) ) );
    } else {
        num = ret;
    }
}

static void setYesNoValue( int& flags, int val, const QByteArray& s ) {
    QByteArray str = s.toLower();
    if( "yes" == str ) {
        flags |= val;
    } else if( "no" != str ) {
        throw UHMMFormatReader::UHMMFormatReaderException( 
            UHMMFormatReader::tr( "cannot_parse_yes/no_value_from_string:%1" ).arg( QString(str) ) );
    }
}

static void allocAndCopyStr( const QByteArray& from, char** to ) {
    assert( !from.isEmpty() && NULL != to );
    int sz = from.size();
    *to = (char*)calloc( sizeof( char ), sz + 1 );
    if( NULL == *to ) {
        throw UHMMFormatReader::UHMMFormatReaderException( UHMMFormatReader::tr( "no_memory:cannot_allocate_hmm_date" ) );
    }
    qCopy( from.data(), from.data() + sz, *to );
    (*to)[sz] = TERM_SYM;
}

const int ALPHA_VERSION_STATS_FIELDS_NUM = 3;
const int BETA_VERSION_STATS_FIELDS_NUM  = 4;

static void setHmmStats( float* params, const QByteArray& s, uint32_t& statstracker ) {
    assert( NULL != params );
    QString str( s );
    QStringList words = str.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
    int wordsSz = words.size();
    if( ALPHA_VERSION_STATS_FIELDS_NUM != wordsSz && BETA_VERSION_STATS_FIELDS_NUM != wordsSz ) {
        throw UHMMFormatReader::UHMMFormatReaderException( UHMMFormatReader::tr( "bad_stats_line:%1" ).arg( QString(str) ) );
    }
    if( "LOCAL" != words.at( 0 ).toUpper() ) {
        throw UHMMFormatReader::UHMMFormatReaderException( 
            UHMMFormatReader::tr( "bad_stats_line:%1.\"LOCAL\" word is not found" ).arg( QString(str) ) );
    }

    if( ALPHA_VERSION_STATS_FIELDS_NUM == wordsSz ) {

    } else if( BETA_VERSION_STATS_FIELDS_NUM == wordsSz ) {

    } else {
        assert( false );
    }
    switch( wordsSz ) {
    case ALPHA_VERSION_STATS_FIELDS_NUM: // this one is for backward compatibility with hmmer3 alpha version
        {
            QByteArray  numStr = words.at( 2 ).toAscii();
            QString     tagStr = words.at( 1 ).toUpper();
            if( "VLAMBDA" == tagStr ) {
                setFloat( params[p7_MLAMBDA], numStr );
                setFloat( params[p7_VLAMBDA], numStr );
                setFloat( params[p7_FLAMBDA], numStr );
                statstracker |= 0x1;
            } else if( "VMU" == tagStr ) {
                setFloat( params[p7_MMU], numStr );
                setFloat( params[p7_VMU], numStr );
                statstracker |= 0x2;
            } else if( "FTAU" == tagStr ) {
                setFloat( params[p7_FTAU], numStr );
                statstracker |= 0x4;
            } else {
                throw UHMMFormatReader::UHMMFormatReaderException( UHMMFormatReader::tr( "bad_stats_line:%1.%2 is not recognized" ).
                    arg( str ).arg( tagStr ) );
            }
        }
        break;
    case BETA_VERSION_STATS_FIELDS_NUM:
        {
            QString tagStr = words.at( 1 ).toUpper();
            QByteArray num1Str = words.at( 2 ).toAscii();
            QByteArray num2Str = words.at( 3 ).toAscii();
            if( "MSV" == tagStr ) {
                setFloat( params[p7_MMU], num1Str );
                setFloat( params[p7_MLAMBDA], num2Str );
                statstracker |= 0x1;
            } else if( "VITERBI" == tagStr ) {
                setFloat( params[p7_VMU], num1Str );
                setFloat( params[p7_VLAMBDA], num2Str );
                statstracker |= 0x2;
            } else if( "FORWARD" == tagStr ) {
                setFloat( params[p7_FTAU], num1Str );
                setFloat( params[p7_FLAMBDA], num2Str );
                statstracker |= 0x4;
            } else {
                throw UHMMFormatReader::UHMMFormatReaderException( UHMMFormatReader::tr( "bad_stats_line:%1. %2 not recognized" ).
                    arg( str ).arg( tagStr ) );
            }
        }
        break;
    default:
        assert( false );
    }

}

static QByteArray getNextToken( QStringList& tokens ) {
    if( tokens.isEmpty() ) {
        throw UHMMFormatReader::UHMMFormatReaderException( UHMMFormatReader::tr( "unexpected_end_of_line" ) );
    }
    return tokens.takeFirst().toAscii();
}

static void setMainModelFloatVal( float& num, const QByteArray& str ) {
    if( "*" == str ) {
        num = 0.0f;
    } else {
        float tmp = 0;
        setFloat( tmp, str );
        num = expf( -1.0 * tmp );
    }
}

static void skipBlankLines( IOAdapter* io ) {
    assert( NULL != io && io->isOpen() );
    char c = 0;
    bool skip = true;
    while( skip ) {
        int ret = io->readBlock( &c, 1 );
        checkReadThrowException( ret );
        if( 0 == ret ) { return; }
        skip = TextUtils::LINE_BREAKS[(uchar)c] || TextUtils::WHITES[(uchar)c];
    }
    io->skip( -1 );
}

static void readLine( IOAdapter* io, QByteArray& to, QStringList* tokens = NULL ) {
    assert( NULL != io );
    to.clear();
    QByteArray buf( BUF_SZ, TERM_SYM );
    bool there = false;
    while( !there ) {
        int bytes = io->readUntil( buf.data(), BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &there );
        checkReadThrowException( bytes );
        if( EMPTY_READ == bytes ) {
            break;
        }
        to.append( QByteArray( buf.data(), bytes ) );
    }
    to = to.trimmed();

    if( NULL != tokens ) {
        *tokens = QString( to ).split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
    }
    skipBlankLines( io );
}

namespace U2 {

const QString UHMMFormatReader::HMM_FORMAT_READER_ERROR_PREFIX  = UHMMFormatReader::tr( "HMM reader error occurred: " );
const QString UHMMFormatReader::READ_FAILED                     = UHMMFormatReader::tr( "reading file failed" );
const QString UHMMFormatReader::HMMER2_VERSION_HEADER           = "HMMER2";
const QString UHMMFormatReader::HMMER3_VERSION_HEADER           = "HMMER3";

UHMMFormatReader::UHMMFormatReader( IOAdapter * i, TaskStateInfo & tsi ) : io( i ), ti( tsi ) {
    if( ti.hasError() || ti.cancelFlag ) {
        return;
    }
    
    if( NULL == io || !io->isOpen() ) {
        ti.setError( HMM_FORMAT_READER_ERROR_PREFIX + L10N::badArgument( "io adapter" ) );
        return;
    }
}

UHMMFormatReader::HMMER_VERSIONS UHMMFormatReader::getVersion( const QByteArray & str ) const {
    QString header( str );
    if( header.startsWith( HMMER2_VERSION_HEADER ) ) {
        return HMMER2_VERSION;
    } else if( header.startsWith( HMMER3_VERSION_HEADER ) ) {
        return HMMER3_VERSION;
    }
    
    return UNKNOWN_VERSION;
}

P7_HMM * UHMMFormatReader::getNextHmm() {
    if( ti.hasError() || ti.cancelFlag ) {
        return NULL;
    }
    
    QByteArray header;
    readLine( io, header );
    HMMER_VERSIONS version = getVersion( header );
    
    P7_HMM * ret = NULL;
    switch( version ) {
    case HMMER2_VERSION:
        ret = readHMMER2ASCII();
        break;
    case HMMER3_VERSION:
        ret = readHMMER3ASCII();
        break;
    case UNKNOWN_VERSION:
        ti.setError( tr( "Input file made by unknown version of HMMER or is not HMM profile file" ) );
        break;
    default:
        assert( false );
    }

    return ret;
}

const int MAX_HMM_HEADER_TAGS = 300;

P7_HMM * UHMMFormatReader::readHMMER3ASCII() {
    assert( NULL != io && io->isOpen() );
    
    ESL_ALPHABET *abc  = NULL;
    P7_HMM       *hmm  = NULL;
    uint32_t      statstracker = 0;
    int           x = 0;
    bool          ok = false;
    
    try {
        if ((hmm = p7_hmm_CreateShell()) == NULL) {
            throw UHMMFormatReader::UHMMFormatReaderException( UHMMFormatReader::tr( "no_memory:cannot_create_hmm_shell" ) );
        }
        
        /* Header section */
        QMap< QByteArray, HMMERHeaderTags > headerTagsMap = getHeaderTagsMap();
        bool isHeaderSection = true;
        int tagsNum = 0;
        while( isHeaderSection && ++tagsNum < MAX_HMM_HEADER_TAGS ) {
            QByteArray line;
            QByteArray tagStr;
            QByteArray valueStr;
            HMMERHeaderTags tag;
            
            readLine( io, line );
            getTagValue( line, tagStr, valueStr );
            tag = headerTagsMap.value( tagStr, BAD_TAG );
            switch( tag ) {
            case NAME:
                p7_hmm_SetName( hmm, valueStr.data() );
                break;
            case ACC:
                p7_hmm_SetAccession( hmm, valueStr.data() );
                break;
            case DESC:
                p7_hmm_SetDescription( hmm, valueStr.data() );
                break;
            case LENG:
                setInteger( hmm->M, valueStr );
                if( 0 >= hmm->M ) {
                    throw UHMMFormatReader::UHMMFormatReaderException(
                        UHMMFormatReader::tr( "length_of_a_model_should_be_positive.we_have:%1" ).arg( hmm->M ) );
                }
                break;
            case ALPH:
                {
                    int abcType = esl_abc_EncodeType( valueStr.data() );
                    if( eslUNKNOWN == abcType ) {
                        throw UHMMFormatReader::UHMMFormatReaderException( 
                            UHMMFormatReader::tr( "unrecognized_alphabet_type:%1" ).arg( QString(valueStr) ) );
                    }
                    abc = esl_alphabet_Create( abcType );
                }
                break;
            case RF:
                setYesNoValue( hmm->flags, p7H_RF, valueStr );
                break;
            case CS:
                setYesNoValue( hmm->flags, p7H_CS, valueStr );
                break;
            case MAP:
                setYesNoValue( hmm->flags, p7H_MAP, valueStr );
                break;
            case DATE:
                allocAndCopyStr( valueStr, &(hmm->ctime) );
                break;
            case COM: // COM is command line that was used to create this hmm. we don't need it
                break;
            case NSEQ:
                setInteger( hmm->nseq, valueStr );
                if( 0 >= hmm->nseq ) {
                    throw UHMMFormatReader::UHMMFormatReaderException(
                        UHMMFormatReader::tr( "nseq_should_be_positive.we_have:%1" ).arg( hmm->nseq ) );
                }
                break;
            case EFFN:
                setFloat( hmm->eff_nseq, valueStr );
                if( 0 >= hmm->eff_nseq ) {
                    throw UHMMFormatReader::UHMMFormatReaderException(
                        UHMMFormatReader::tr( "effn_should_be_positive.we_have:%1" ).arg( hmm->eff_nseq ) );
                }
                break;
            case CKSUM:
                setUInteger( hmm->checksum, valueStr );
                hmm->flags |= p7H_CHKSUM;
                break;
            case STATS:
                setHmmStats( hmm->evparam, valueStr, statstracker );
                break;
            case GA:
                set2Floats( hmm->cutoff[p7_GA1], hmm->cutoff[p7_GA2], valueStr );
                hmm->flags |= p7H_GA;
                break;
            case TC:
                set2Floats( hmm->cutoff[p7_TC1], hmm->cutoff[p7_TC2], valueStr );
                hmm->flags |= p7H_TC;
                break;
            case NC:
                set2Floats( hmm->cutoff[p7_NC1], hmm->cutoff[p7_NC2], valueStr );
                hmm->flags |= p7H_NC;
                break;
            case HMM:
                isHeaderSection = false;
                continue;
            case BM:
            case SM:
                //TODO: update HMMER and start use these fields too
                break;
            case BAD_TAG:
                throw UHMMFormatReader::UHMMFormatReaderException(
                    UHMMFormatReader::tr( "unrecognized_tag_in_header_section:%1" ).arg( QString(tagStr) ) );
            default:
                assert( 0 );
            }
        } /* end, loop over possible header tags */
        
        /* If we saw one STATS line, we needed all 3 */
        if( statstracker == 0x7 ) {
            hmm->flags |= p7H_STATS;
        } else if( statstracker != 0x0 ) {
            throw UHMMFormatReader::UHMMFormatReaderException(
                UHMMFormatReader::tr( "missing_one_or_more_STATS_parameter_lines" ) );
        }
        
        if( NULL == abc ) {
            throw UHMMFormatReader::UHMMFormatReaderException(
                UHMMFormatReader::tr( "failed_to_create_alphabet" ) );
        }
        
        if( p7_hmm_CreateBody( hmm, hmm->M, abc->type ) != eslOK ) {
            throw UHMMFormatReader::UHMMFormatReaderException(
                UHMMFormatReader::tr( "failed_to_allocate_body_of_hmm" ) );
        }
        
        QByteArray line;
        QStringList tokens;
        readLine( io, line );
        readLine( io, line, &tokens );
        QByteArray curToken = getNextToken( tokens );
        
        /* Optional model composition (filter null model) may immediately follow headers */
        if( "COMPO" == curToken.toUpper() ) {
            for( x = 0; x < abc->K; x++ ) {
                curToken = getNextToken( tokens );
                setMainModelFloatVal( hmm->compo[x], curToken );
            }
            hmm->flags |= p7H_COMPO;
            readLine( io, line, &tokens );
            curToken = getNextToken( tokens );
        }
        
        /* First two lines are node 0: insert emissions, then transitions from node 0 (begin) */
        setMainModelFloatVal( hmm->ins[0][0], curToken );
        for( x = 1; x < abc->K; x++ ) {
            curToken = getNextToken( tokens );
            setMainModelFloatVal( hmm->ins[0][x], curToken );
        }
        readLine( io, line, &tokens );
        for( x = 0; x < p7H_NTRANSITIONS; x++ ) {
            curToken = getNextToken( tokens );
            setMainModelFloatVal( hmm->t[0][x], curToken );
        }
        
        /* The main model section. */
        int k = 0;
        for( k = 1; k <= hmm->M; k++ ) {
            int n = 0;
            readLine( io, line, &tokens );
            curToken = getNextToken( tokens );
            setInteger( n, curToken );
            if( k != n ) {
                throw UHMMFormatReader::UHMMFormatReaderException(
                    UHMMFormatReader::tr( "expected_line_to_start_with:%1.it_starts_with:%2" ).arg( k ).arg( QString(curToken) ) );
            }
            for( x = 0; x < abc->K; x++ ) {
                curToken = getNextToken( tokens );
                setMainModelFloatVal( hmm->mat[k][x], curToken );
            }
            curToken = getNextToken( tokens );
            if( hmm->flags & p7H_MAP ) {
                int num = 0;
                setInteger( num, curToken );
                hmm->map[k] = num;
            }
            curToken = getNextToken( tokens );
            if (hmm->flags & p7H_RF) {
                assert( !curToken.isEmpty() );
                hmm->rf[k] = curToken.at( 0 );
            }
            
            curToken = getNextToken( tokens );
            if( hmm->flags & p7H_CS ) {
                assert( !curToken.isEmpty() );
                hmm->cs[k] = curToken.at( 0 );
            }
            
            readLine( io, line, &tokens );
            for( x = 0; x < abc->K; x++ ) {
                curToken = getNextToken( tokens );
                setMainModelFloatVal( hmm->ins[k][x], curToken );
            }

            readLine( io, line, &tokens );
            for( x = 0; x < p7H_NTRANSITIONS; x++ ) {
                curToken = getNextToken( tokens );
                setMainModelFloatVal( hmm->t[k][x], curToken );
            }
        }
        
        /* The closing // */
        readLine( io, line, &tokens );
        curToken = getNextToken( tokens );
        if( "//" != curToken ) {
            throw UHMMFormatReader::UHMMFormatReaderException(
                UHMMFormatReader::tr( "expected_closing_//.found_%1_instead" ).arg( QString(curToken) ) );
        }
        skipBlankLines( io );
        
        /* Finish up. */
        if( hmm->flags & p7H_RF ) {
            hmm->rf[0]  = ' ';
            hmm->rf[hmm->M+1] = '\0';
        }
        if( hmm->flags & p7H_CS ) {
            hmm->cs[0]  = ' '; 
            hmm->cs[hmm->M+1] = '\0';
        }
        if( hmm->flags & p7H_MAP ) {
            hmm->map[0] = 0;
        }
        if( hmm->name == NULL ) {
            throw UHMMFormatReader::UHMMFormatReaderException( UHMMFormatReader::tr( "no_name_found_in_hmm" ) );
        }
        assert( 0 <= hmm->M );
        assert( NULL != hmm->abc );
    } catch( const UHMMFormatReaderException & ex ) {
        ti.setError( ex.what );
    } catch(...) {
        ti.setError( HMMER3_UNKNOWN_ERROR );
    }
    esl_alphabet_Destroy( abc );
    
    if( ti.hasError() ) {
        p7_hmm_Destroy( hmm );
        return NULL;
    }
    assert( NULL != hmm );
    
    return hmm;
}

P7_HMM * UHMMFormatReader::readHMMER2ASCII() {
    assert( NULL != io && io->isOpen() );
    
    ESL_ALPHABET *abc  = NULL;
    P7_HMM       *hmm  = NULL;
    P7_BG        *bg   = NULL;
    float         null[p7_MAXABET];
    int           k,x;
    
    try {
        if( ( hmm = p7_hmm_CreateShell()) == NULL ) {
            throw UHMMFormatReaderException( tr( "allocation failure, HMM shell" ) );
        }
        
        /* Header */
        /* H2 save files have no EFFN; 
        * COM lines don't have number tags like [1];
        * they have CKSUM but we ignore it because it uses different algorithm;
        * have EVD line, we ignore it, H3 stats are different;
        * XT, NULT lines are ignored; algorithm-dependent config is all internal in H3
        */
        QMap< QByteArray, HMMERHeaderTags > headerTagsMap = getHeaderTagsMap();
        bool isHeaderSection = true;
        bool triedToSetAlph = false;
        int tagsNum = 0;
        while( isHeaderSection && ++tagsNum < MAX_HMM_HEADER_TAGS  ) {
            QByteArray line;
            QStringList tokens;
            QByteArray tagStr;
            QByteArray valueStr;
            HMMERHeaderTags tag;
            
            readLine( io, line, &tokens );
            getTagValue( line, tagStr, valueStr );
            tag = headerTagsMap.value( tagStr, BAD_TAG );
            switch( tag ) {
            case NAME:
                p7_hmm_SetName( hmm, valueStr.data() );
                break;
            case ACC:
                p7_hmm_SetAccession( hmm, valueStr.data() );
                break;
            case DESC:
                p7_hmm_SetDescription( hmm, valueStr.data() );
                break;
            case LENG:
                setInteger( hmm->M, valueStr );
                if( 0 >= hmm->M ) {
                    throw UHMMFormatReader::UHMMFormatReaderException(
                        UHMMFormatReader::tr( "length_of_a_model_should_be_positive.we_have:%1" ).arg( hmm->M ) );
                }
                break;
            case ALPH:
                {
                    triedToSetAlph = true;
                    if(valueStr.toLower() != "nucleic") { // try to encode abc in hmm section
                        int abcType = esl_abc_EncodeType( valueStr.data() );
                        if( eslUNKNOWN == abcType ) {
                            throw UHMMFormatReader::UHMMFormatReaderException(
                                tr( "unrecognized_alphabet_type:%1\n" ).arg( QString(valueStr) ) );
                        }
                        abc = esl_alphabet_Create( abcType );
                    }
                }
                break;
            case RF:
                setYesNoValue( hmm->flags, p7H_RF, valueStr );
                break;
            case CS:
                setYesNoValue( hmm->flags, p7H_CS, valueStr );
                break;
            case MAP:
                setYesNoValue( hmm->flags, p7H_MAP, valueStr );
                break;
            case DATE:
                allocAndCopyStr( valueStr, &(hmm->ctime) );
                break;
            case COM: // we don't need it
                break;
            case NSEQ:
                setInteger( hmm->nseq, valueStr );
                if( 0 >= hmm->nseq ) {
                    throw UHMMFormatReader::UHMMFormatReaderException(
                        UHMMFormatReader::tr( "nseq_should_be_positive.we_have:%1" ).arg( hmm->nseq ) );
                }
                break;
            case GA:
                set2Floats( hmm->cutoff[p7_GA1], hmm->cutoff[p7_GA2], valueStr );
                hmm->flags |= p7H_GA;
                break;
            case TC:
                set2Floats( hmm->cutoff[p7_TC1], hmm->cutoff[p7_TC2], valueStr );
                hmm->flags |= p7H_TC;
                break;
            case NC:
                set2Floats( hmm->cutoff[p7_NC1], hmm->cutoff[p7_NC2], valueStr );
                hmm->flags |= p7H_NC;
                break;
            case NULE:
                {
                    int K = 0;
                    if( !triedToSetAlph) {
                        throw UHMMFormatReaderException( tr( "ALPH must precede NULE in HMMER2 save files" ) );
                    } else if(abc != NULL){
                        K = abc->K;
                    } else {
                        K = 4; // because it is dna or rna
                    }
                    
                    getNextToken( tokens ); // skip NULE word
                    for (x = 0; x < K; x++) {
                        QByteArray curToken = getNextToken( tokens );
                        assert( 1 <= curToken.size() );
                        null[x] = h2ascii2prob( curToken, 1.0/(float)K );
                    }
                }
                break;
            case HMM:
                {
                    if(abc == NULL) {
                        if(!triedToSetAlph) {
                            throw UHMMFormatReaderException(tr("ALPH section must precede HMM"));
                        } else {
                            QString val(valueStr);
                            DNAAlphabet* al = AppContext::getDNAAlphabetRegistry()->findAlphabet(val.remove(QRegExp("\\s+")).toAscii());
                            if(al == NULL || !al->isNucleic()) {
                                throw UHMMFormatReaderException(tr("Unknown alphabet"));
                            } else {
                                int abcT = al->getId() == BaseDNAAlphabetIds::NUCL_DNA_DEFAULT() ||
                                    al->getId() == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED() ? eslDNA : eslRNA;;
                                abc = esl_alphabet_Create(abcT);
                            }
                        }
                    }
                    isHeaderSection = false;
                }
                continue;
            default:
                continue;
            }
        } // over header section
        
        /* Skip main model header lines; allocate body of HMM now that K,M are known */
        QByteArray line;
        QStringList tokens;
        readLine( io, line );
        readLine( io, line, &tokens );
        
        if( p7_hmm_CreateBody(hmm, hmm->M, abc->type ) != eslOK ) {
            throw UHMMFormatReaderException( tr( "Failed to allocate body of the new HMM" ) );
        }
        if( (bg = p7_bg_Create( abc )) == NULL ) {
            throw UHMMFormatReaderException( tr( "failed to create background model" ) );
        }
        
        /* H2's tbd1 line ==> translated to H3's node 0 */
        QByteArray tok1;
        QByteArray tok2;
        QByteArray tok3;
        tok1 = getNextToken( tokens );
        tok2 = getNextToken( tokens );
        tok3 = getNextToken( tokens );
        
        hmm->t[0][p7H_MM] = h2ascii2prob(tok1, 1.0);	/* B->M1 */
        hmm->t[0][p7H_MI] = 0.0;	                /* B->I0 */
        hmm->t[0][p7H_MD] = h2ascii2prob(tok3, 1.0);    /* B->D1 */
        hmm->t[0][p7H_IM] = 1.0;
        hmm->t[0][p7H_II] = 0.0;
        hmm->t[0][p7H_DM] = 1.0;
        hmm->t[0][p7H_DD] = 0.0;
        for (x = 0; x < abc->K; x++) {
            hmm->ins[0][x] = bg->f[x];
        }
        
        /* The main model section. */
        for (k = 1; k <= hmm->M; k++) {
            int n = 0;
            readLine( io, line, &tokens );
            tok1 = getNextToken( tokens );
            setInteger( n, tok1 );
            
            if( n != k ) {
                throw UHMMFormatReaderException( tr( "Expected match line to start with %1. saw %2" ).arg( k ).arg( QString( tok1 ) ) );
            }
            
            /* Line 1: match emissions; optional map info */
            for (x = 0; x < abc->K; x++) {
                tok1 = getNextToken( tokens );
                hmm->mat[k][x] = h2ascii2prob(tok1, null[x]);
            }
            if (hmm->flags & p7H_MAP) {
                int n = 0;
                tok1 = getNextToken( tokens );
                setInteger( n, tok1 );
                hmm->map[k] = n;
            }
            
            /* Line 2: optional RF; then we ignore insert emissions */
            readLine( io, line, &tokens );
            tok1 = getNextToken( tokens );
            if (hmm->flags & p7H_RF) {
                assert( 1 <= tok1.size() );
                hmm->rf[k]     = tok1.at( 0 );
            }
            for (x = 0; x < abc->K; x++) {
                hmm->ins[k][x] = bg->f[x];
            }
            
            /* Line 3: optional CS, then transitions (ignoring last 2, which are entry/exit */
            readLine( io, line, &tokens );
            tok1 = getNextToken( tokens );
            if (hmm->flags & p7H_CS) {
                assert( 1 <= tok1.size() );
                hmm->cs[k]   = tok1.at( 0 );
            }
            if (k < hmm->M) {		/* ignore last insert transition line; H3/H2 not compatible there */
                for (x = 0; x < p7H_NTRANSITIONS; x++) {
                    tok1 = getNextToken( tokens );
                    hmm->t[k][x] = h2ascii2prob(tok1, 1.0);
                }
            }
        }
        
        /* node M transitions: H2 doesn't have an I_M state */
        hmm->t[hmm->M][p7H_MM] = 1.0;
        hmm->t[hmm->M][p7H_MI] = 0.0;
        hmm->t[hmm->M][p7H_MD] = 0.0;
        hmm->t[hmm->M][p7H_IM] = 1.0;
        hmm->t[hmm->M][p7H_II] = 0.0;
        hmm->t[hmm->M][p7H_DM] = 1.0;
        hmm->t[hmm->M][p7H_DD] = 0.0;
        
        /* The closing // */
        readLine( io, line, &tokens );
        tok1 = getNextToken( tokens );
        if( "//" != tok1 ) {
            throw UHMMFormatReaderException( tr( "Expected closing //. found %1 instead" ).arg( QString( tok1 ) ) );
        }
        
        /* Tidy up. */
        if (hmm->flags & p7H_RF) {
            hmm->rf[0] = ' ';
            hmm->rf[hmm->M+1] = '\0';
        }
        if (hmm->flags & p7H_CS) {
            hmm->cs[0] = ' ';
            hmm->cs[hmm->M+1] = '\0';
        }
        if (hmm->name == NULL) {
            throw UHMMFormatReaderException( tr( "No NAME found for HMM" ) );
        }
        if (hmm->M <= 0) {
            throw UHMMFormatReaderException( tr( "No LENG found for HMM" ) );
        }
        if (abc == NULL) {
            throw UHMMFormatReaderException( tr( "No ALPH found for HMM" ) );
        }
        
        /* Calibrate the model:         cfg   rng   bg    gm    om */
        TaskStateInfo tmpTsi;
        if( p7_Calibrate(hmm, NULL, NULL, &bg, NULL, NULL, 0, tmpTsi ) != eslOK ) {
            throw UHMMFormatReaderException( tr( "Failed to calibrate HMMER2 model after input conversion" ) );
        }
    } catch( const UHMMFormatReaderException & ex ) {
        ti.setError( ex.what );
    } catch(...) {
        ti.setError( HMMER3_UNKNOWN_ERROR );
    }
    
    p7_bg_Destroy(bg);
    esl_alphabet_Destroy( abc );
    
    if( ti.hasError() ) {
        p7_hmm_Destroy( hmm );
        return NULL;
    }
    assert( NULL != hmm );
    
    return hmm;
}

} // U2
