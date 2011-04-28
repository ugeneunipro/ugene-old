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

#include <memory>

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/TextUtils.h>

#include "uhmmer3BuildTests.h"

namespace U2 {

/**************************
* GTest_UHMMER3Build
**************************/

const QString GTest_UHMMER3Build::INPUT_FILE_TAG    = "inputFile";
const QString GTest_UHMMER3Build::OUTPUT_FILE_TAG   = "outputFile";
const QString GTest_UHMMER3Build::DEL_OUTPUT_TAG    = "delOutput";

const QString GTest_UHMMER3Build::SEED_OPTION_TAG   = "seed";

const QString GTest_UHMMER3Build::MODEL_CONSTRUCTION_OPTION_TAG     = "mc";
const QString GTest_UHMMER3Build::RELATIVE_WEIGHTING_OPTION_TAG     = "rw";
const QString GTest_UHMMER3Build::EFFECTIVE_WEIGHTING_OPTION_TAG    = "ew";
const QString GTest_UHMMER3Build::E_VALUE_CALIBATION_OPTION_TAG     = "eval";

static void setSeedOption( UHMM3BuildSettings & settings, TaskStateInfo & stateInfo, const QString& str ) {
    if( str.isEmpty() ) {
        return;
    }

    bool ok = false;
    int num = str.toInt( &ok );
    if( !ok || !( 0 <= num ) ) {
        stateInfo.setError( "cannot_parse_option_seed" );
        return;
    }
    settings.seed = num;
}

static void setModelConstructionOption( UHMM3BuildSettings & settings, TaskStateInfo & stateInfo, const QString& s ) {
    QString str = s.toLower();
    if( str.startsWith( "fast" ) ) {
        settings.archStrategy = p7_ARCH_FAST;
        QString numStr = str.mid( 4 ).trimmed();
        
        if( !numStr.isEmpty() ) {
            bool ok = false;
            float num = numStr.toFloat( &ok );
            if( !ok || !( 0 <= num && 1 >= num ) ) {
                stateInfo.setError( "cannot_parse_mc_option: symfrac" );
                return;
            }
            settings.symfrac = num;
        }
        return;
    }
    if( str.startsWith( "hand" ) ) {
        settings.archStrategy = p7_ARCH_HAND;
        return;
    }
    if( !str.isEmpty() ) {
        stateInfo.setError( "unrecognized_mc_option" );
    }
}

static void setRelativeWeightingOption( UHMM3BuildSettings & settings, TaskStateInfo & stateInfo, const QString& s ) {
    QString str = s.toLower();
    if( str.startsWith( "wgsc" ) ) {
        settings.wgtStrategy = p7_WGT_GSC;
        return;
    }
    if( str.startsWith( "wblosum" ) ) {
        settings.wgtStrategy = p7_WGT_BLOSUM;
        QString numStr = str.mid( 7 ).trimmed();
        if( !numStr.isEmpty() ) {
            bool ok = false;
            float num = numStr.toFloat( &ok );
            if( !ok || !( 0 <= num && 1 >= num ) ) {
                stateInfo.setError( "cannot_parse_rw_option:wid" );
                return;
            }
            settings.wid = num;
        }
        return;
    }
    if( str.startsWith( "wpb" ) ) {
        settings.wgtStrategy = p7_WGT_PB;
        return;
    }
    if( str.startsWith( "wnone" ) ) {
        settings.wgtStrategy = p7_WGT_NONE;
        return;
    }
    if( str.startsWith( "wgiven" ) ) {
        settings.wgtStrategy = p7_WGT_GIVEN;
        return;
    }
    if( !str.isEmpty() ) {
        stateInfo.setError( "unrecognized_rw_option" );
    }
}

static void setEffectiveWeightingOption( UHMM3BuildSettings & settings, TaskStateInfo & stateInfo, const QString& s ) {
    QString str = s.toLower();
    if( str.startsWith( "eent" ) ) {
        settings.effnStrategy = p7_EFFN_ENTROPY;
        QString numStr = str.mid( 4 ).trimmed();

        if( !numStr.isEmpty() ) {
            QStringList nums = numStr.split( " ", QString::SkipEmptyParts );

            if( nums.isEmpty() ) {
                stateInfo.setError( "cannot_parse_ew_option: eent" );
                return;
            }
            bool ok = false;
            double num = nums[0].toDouble( &ok );
            if( !ok ) {
                stateInfo.setError( "cannot_parse_ew_option: ere" );
                return;
            }
            if( num > 0 ) {
                settings.ere = num;
                return;
            }
        }
        return;
    }
    if( str.startsWith( "eclust" ) ) {
        settings.effnStrategy = p7_EFFN_CLUST;
        QString numStr = str.mid( 6 ).trimmed();

        if( !numStr.isEmpty() ) {
            bool ok = false;
            double num = numStr.toDouble( &ok );
            if( !ok || !( 0 <= num && 1 >= num ) ) {
                stateInfo.setError( "cannot_parse_ew_option: eid" );
                return;
            }
            settings.eid = num;
        }
        return;
    }
    if( str.startsWith( "enone" ) ) {
        settings.effnStrategy = p7_EFFN_NONE;
        return;
    }
    if( str.startsWith( "eset" ) ) {
        settings.effnStrategy = p7_EFFN_SET;
        QString numStr = str.mid( 4 ).trimmed();
        bool ok = false;
        double num = numStr.toDouble( &ok );

        if( !ok || 0 > num ) {
            stateInfo.setError( "cannot_parse_ew_option: eset" );
            return;
        }
        settings.eset = num;
        return;
    }
    if( !str.isEmpty() ) {
        stateInfo.setError( "unrecognized_ew_option" );
    }
}

static void setEvalueCalibrationOption( UHMM3BuildSettings & settings, TaskStateInfo & stateInfo, const QString& s ) {
    QString str = s.toLower();
    if( str.isEmpty() ) {
        return;
    }
    QStringList l = str.split( " ", QString::SkipEmptyParts );
    if( 5 != l.size() ) {
        stateInfo.setError( "error_parsing_eval_option" );
        return;
    }
    settings.evl = l[0].toInt();
    settings.evn = l[1].toInt();
    settings.efl = l[2].toInt();
    settings.efn = l[3].toInt();
    settings.eft = l[4].toDouble();
}

void GTest_UHMMER3Build::init( XMLTestFormat *tf, const QDomElement& el ) {
    Q_UNUSED( tf );

    inFile = el.attribute( INPUT_FILE_TAG );
    outFile = el.attribute( OUTPUT_FILE_TAG );

    QString delOutStr = el.attribute( DEL_OUTPUT_TAG );
    delOutFile = !delOutStr.isEmpty() && delOutStr.toLower() != "no" && delOutStr.toLower() != "n";
    setBuildSettings( bldSettings.inner, el, stateInfo );
}

void GTest_UHMMER3Build::setBuildSettings( UHMM3BuildSettings & settings, const QDomElement& el, TaskStateInfo & ti ) {
    setModelConstructionOption(  settings, ti, el.attribute( MODEL_CONSTRUCTION_OPTION_TAG ) );
    setRelativeWeightingOption(  settings, ti, el.attribute( RELATIVE_WEIGHTING_OPTION_TAG ) );
    setEffectiveWeightingOption( settings, ti, el.attribute( EFFECTIVE_WEIGHTING_OPTION_TAG ) );
    setEvalueCalibrationOption(  settings, ti, el.attribute( E_VALUE_CALIBATION_OPTION_TAG ) );
    setSeedOption( settings, ti, el.attribute( SEED_OPTION_TAG ) );
}

void GTest_UHMMER3Build::setAndCheckArgs() {
    if( hasError() ) {
        return;
    }

    if( inFile.isEmpty() ) {
        stateInfo.setError( tr( "input_file_not_given" ) );
        return;
    }
    inFile = env->getVar( "COMMON_DATA_DIR" ) + "/" + inFile;

    if( outFile.isEmpty() ) {
        stateInfo.setError( tr( "output_file_not_given" ) );
        return;
    }
    outFile = env->getVar( "TEMP_DATA_DIR" ) + "/" + outFile;

    /* Build settings */
    bldSettings.outFile = outFile;
}

void GTest_UHMMER3Build::prepare() {
    setAndCheckArgs();
    if( hasError() ) {
        return;
    }

    buildTask = new UHMM3BuildToFileTask( bldSettings, inFile );
    addSubTask( buildTask );
}

Task::ReportResult GTest_UHMMER3Build::report() {
    if( buildTask->hasError() ) {
        if( delOutFile ) {
            QFile::remove( outFile );
        }
        stateInfo.setError( buildTask->getError() );
    }
    return ReportResult_Finished;
}

void GTest_UHMMER3Build::cleanup() {
    if( delOutFile ) {
        QFile::remove( outFile );
    }
}

GTest_UHMMER3Build::~GTest_UHMMER3Build() {
    cleanup();
}

/**************************
* GTest_CompareHmmFiles
**************************/

const QString GTest_CompareHmmFiles::FILE1_NAME_TAG = "file1";
const QString GTest_CompareHmmFiles::FILE2_NAME_TAG = "file2";
const QString GTest_CompareHmmFiles::FILE1_TMP_TAG  = "tmp1";
const QString GTest_CompareHmmFiles::FILE2_TMP_TAG  = "tmp2";

const int   BUF_SZ      = 2048;
const char  TERM_SYM    = '\0';

const QByteArray DATE_STR   = "DATE";
const QByteArray NAME_STR   = "NAME";
const QByteArray HEADER_STR = "HMMER3/";

void GTest_CompareHmmFiles::init( XMLTestFormat *tf, const QDomElement &el ) {
    Q_UNUSED( tf );

    filename1 = el.attribute( FILE1_NAME_TAG );
    filename2 = el.attribute( FILE2_NAME_TAG );

    QString file1TmpStr = el.attribute( FILE1_TMP_TAG );
    file1Tmp = !file1TmpStr.isEmpty() && file1TmpStr.toLower() != "no" && file1TmpStr.toLower() != "n";

    QString file2TmpStr = el.attribute( FILE2_TMP_TAG );
    file2Tmp = !file2TmpStr.isEmpty() && file2TmpStr.toLower() != "no" && file2TmpStr.toLower() != "n";
}

void GTest_CompareHmmFiles::setAndCheckArgs() {
    if( filename1.isEmpty() ) {
        stateInfo.setError( tr( "file1_not_given" ) );
        return;
    }
    filename1 = env->getVar( file1Tmp? "TEMP_DATA_DIR" : "COMMON_DATA_DIR" ) + "/" + filename1;

    if( filename2.isEmpty() ) {
        stateInfo.setError( tr( "file2_not_given" ) );
        return;
    }
    filename2 = env->getVar( file2Tmp? "TEMP_DATA_DIR" : "COMMON_DATA_DIR" ) + "/" + filename2;
}

static const float BUILD_COMPARE_FLOAT_EPS = 0.00002;

static bool compareStr( const QString& s1, const QString& s2 ) {
    assert( s1.size() == s2.size() );

    QStringList words1 = s1.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
    QStringList words2 = s2.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );

    if( words1.size() != words2.size() ) {
        return false;
    }

    int sz = words1.size();
    for( int i = 0; i < sz; ++i ) {
        bool ok1 = false;
        bool ok2 = false;
        float num1 = 0;
        float num2 = 0;

        num1 = words1.at( i ).toFloat( &ok1 );
        num2 = words2.at( i ).toFloat( &ok2 );

        if( ok1 != ok2 ) {
            return false;
        } else {
            if( ok1 ) {
                if( (isfin( num1 ) && !isfin( num2 )) || ( isfin( num2 ) && !isfin( num1 ) ) ) {
                    return false;
                }
                if( qAbs( num1 - num2 ) > BUILD_COMPARE_FLOAT_EPS ) {
                    return false;
                }
                continue;
            } else {
                if( words1.at( i ) != words2.at( i ) ) {
                    return false;
                }
                continue;
            }
        }
    }
    return true;
}

Task::ReportResult GTest_CompareHmmFiles::report() {
    assert( !hasError() );
    setAndCheckArgs();
    if( hasError() ) {
        return ReportResult_Finished;
    }

    IOAdapterFactory* iof1 = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(filename1));
    std::auto_ptr<IOAdapter> io1(iof1->createIOAdapter());
    if( io1.get() == NULL ) {
        stateInfo.setError( tr( "cannot_create_io_adapter_for_1_file" ) );
        return ReportResult_Finished;
    }
    if( !io1->open( filename1, IOAdapterMode_Read ) ) {
        stateInfo.setError( tr( "cannot_open_1_file" ) );
        return ReportResult_Finished;
    }

    IOAdapterFactory* iof2 = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(filename2));
    std::auto_ptr<IOAdapter> io2(iof2->createIOAdapter());
    if( io2.get() == NULL ) {
        stateInfo.setError( tr( "cannot_create_io_adapter_for_2_file" ) );
        return ReportResult_Finished;
    }
    if( !io2->open( filename2, IOAdapterMode_Read ) ) {
        stateInfo.setError( tr( "cannot_open_2_file" ) );
        return ReportResult_Finished;
    }

    QByteArray buf1( BUF_SZ, TERM_SYM );
    QByteArray buf2( BUF_SZ, TERM_SYM );
    int bytes1 = 0;
    int bytes2 = 0;

    do {
        bytes1 = io1->readUntil( buf1.data(), BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Include );
        bytes2 = io2->readUntil( buf2.data(), BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Include );
        if( buf1.startsWith( DATE_STR ) && buf2.startsWith( DATE_STR ) ) {
            continue;
        }
        if( buf1.startsWith( HEADER_STR ) && buf2.startsWith( HEADER_STR ) ) {
            continue;
        }
        if( buf1.startsWith( NAME_STR ) && buf2.startsWith( NAME_STR ) ) {
            QString name1 = QByteArray( buf1.data(), bytes1 ).mid( NAME_STR.size() ).trimmed();
            QString name2 = QByteArray( buf2.data(), bytes2 ).mid( NAME_STR.size() ).trimmed();

            if( name1.startsWith( name2 ) || name2.startsWith( name1 ) ) {
                continue;
            }
            stateInfo.setError( tr( "names_of_alignments_not_similar" ) );
            return ReportResult_Finished;
        }
        if( bytes1 != bytes2 ) {
            stateInfo.setError( tr( "strings_written_from_files:different_length" ) );
            return ReportResult_Finished;
        }

        QString s1 = QString::fromAscii( buf1.data(), bytes1 );
        QString s2 = QString::fromAscii( buf2.data(), bytes2 );
        if( !compareStr( s1, s2 ) ) {
            stateInfo.setError( tr( "files_not_equal: \"%1\" and \"%2\"" ).arg( s1 ).arg( s2 ) );
            return ReportResult_Finished;
        }
    } while( 0 < bytes1 && 0 < bytes2 );

    return ReportResult_Finished;
}

} // U2
