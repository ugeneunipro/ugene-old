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

#include "uhmmer3SearchTests.h"
#include <gobject/uHMMObject.h>

#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QList>
#include <memory>

namespace U2 {

/*******************************
* GTest_GeneralUHMM3Search
********************************/

const QString GTest_UHMM3Search::SEQ_DOC_CTX_NAME_TAG        = "seqDoc";
const QString GTest_UHMM3Search::HMM_FILENAME_TAG            = "hmm";
const QString GTest_UHMM3Search::HMMSEARCH_TASK_CTX_NAME_TAG = "taskCtxName";
const QString GTest_UHMM3Search::ALGORITHM_TYPE_OPTION_TAG   = "algo";
const QString GTest_UHMM3Search::SW_CHUNK_SIZE_OPTION_TAG    = "chunk";

const QString GTest_UHMM3Search::SEQ_E_OPTION_TAG            = "seqE";
const QString GTest_UHMM3Search::SEQ_T_OPTION_TAG            = "seqT";
const QString GTest_UHMM3Search::Z_OPTION_TAG                = "z";
const QString GTest_UHMM3Search::DOM_E_OPTION_TAG            = "domE";
const QString GTest_UHMM3Search::DOM_T_OPTION_TAG            = "domT";
const QString GTest_UHMM3Search::DOM_Z_OPTION_TAG            = "domZ";
const QString GTest_UHMM3Search::USE_BIT_CUTOFFS_OPTION_TAG  = "ubc";
const QString GTest_UHMM3Search::INC_SEQ_E_OPTION_TAG        = "incE";
const QString GTest_UHMM3Search::INC_SEQ_T_OPTION_TAG        = "incT";
const QString GTest_UHMM3Search::INC_DOM_E_OPTION_TAG        = "incdomE";
const QString GTest_UHMM3Search::INC_DOM_T_OPTION_TAG        = "incdomT";
const QString GTest_UHMM3Search::MAX_OPTION_TAG              = "max";
const QString GTest_UHMM3Search::F1_OPTION_TAG               = "f1";
const QString GTest_UHMM3Search::F2_OPTION_TAG               = "f2";
const QString GTest_UHMM3Search::F3_OPTION_TAG               = "f3";
const QString GTest_UHMM3Search::NOBIAS_OPTION_TAG           = "nobias";
const QString GTest_UHMM3Search::NONULL2_OPTION_TAG          = "nonull2";
const QString GTest_UHMM3Search::SEED_OPTION_TAG             = "seed";
const QString GTest_UHMM3Search::REMOTE_MACHINE_VAR          = "MACHINE";

static void setDoubleOption( double& num, const QDomElement& el, const QString& optionName, TaskStateInfo& si ) {
    if( si.hasError() ) {
        return;
    }
    QString numStr = el.attribute( optionName );
    if( numStr.isEmpty() ) {
        return;
    }
    
    bool ok = false;
    double ret = numStr.toDouble( &ok );
    if( !ok ) {
        si.setError( QString( "cannot_parse_double_number_from %1. Option: %2" ).arg( numStr ).arg( optionName ) );
        return;
    }
    num = ret;
}

static void setUseBitCutoffsOption( int& ret, const QDomElement& el, const QString& opName, TaskStateInfo& si ) {
    if( si.hasError() ) {
        return;
    }

    QString str = el.attribute( opName ).toLower();

    if( "ga" == str ) {
        ret = p7H_GA;
    } else if( "nc" == str ) {
        ret = p7H_NC;
    } else if( "tc" == str ) {
        ret = p7H_TC;
    } else if( !str.isEmpty() ) {
        si.setError( QString( "unrecognized_value_in %1 option" ).arg( opName ) );
    }
}

static void setBooleanOption( int& ret, const QDomElement& el, const QString& opName, TaskStateInfo& si ) {
    if( si.hasError() ) {
        return;
    }
    QString str = el.attribute( opName ).toLower();

    if( !str.isEmpty() &&  "n" != str && "no" != str ) {
        ret = TRUE;
    } else {
        ret = FALSE;
    }
}

static void setIntegerOption( int& num, const QDomElement& el, const QString& optionName, TaskStateInfo& si ) {
    if( si.hasError() ) {
        return;
    }
    QString numStr = el.attribute( optionName );
    if( numStr.isEmpty() ) {
        return;
    }

    bool ok = false;
    int ret = numStr.toInt( &ok );
    if( !ok ) {
        si.setError( QString( "cannot_parse_integer_number_from %1. Option: %2" ).arg( numStr ).arg( optionName ) );
        return;
    }
    num = ret;
}

void GTest_UHMM3Search::setSearchTaskSettings( UHMM3SearchSettings& settings, const QDomElement& el, TaskStateInfo& si ) {
    setDoubleOption( settings.e,       el, GTest_UHMM3Search::SEQ_E_OPTION_TAG,     si );
    setDoubleOption( settings.t,       el, GTest_UHMM3Search::SEQ_T_OPTION_TAG,     si );
    setDoubleOption( settings.z,       el, GTest_UHMM3Search::Z_OPTION_TAG,         si );
    setDoubleOption( settings.f1,      el, GTest_UHMM3Search::F1_OPTION_TAG,        si );
    setDoubleOption( settings.f2,      el, GTest_UHMM3Search::F2_OPTION_TAG,        si );
    setDoubleOption( settings.f3,      el, GTest_UHMM3Search::F3_OPTION_TAG,        si );
    setDoubleOption( settings.domE,    el, GTest_UHMM3Search::DOM_E_OPTION_TAG,     si );
    setDoubleOption( settings.domT,    el, GTest_UHMM3Search::DOM_T_OPTION_TAG,     si );
    setDoubleOption( settings.domZ,    el, GTest_UHMM3Search::DOM_Z_OPTION_TAG,     si );
    setDoubleOption( settings.incE,    el, GTest_UHMM3Search::INC_SEQ_E_OPTION_TAG, si );
    setDoubleOption( settings.incT,    el, GTest_UHMM3Search::INC_SEQ_T_OPTION_TAG, si );
    setDoubleOption( settings.incDomE, el, GTest_UHMM3Search::INC_DOM_E_OPTION_TAG, si );
    setDoubleOption( settings.incDomT, el, GTest_UHMM3Search::INC_DOM_T_OPTION_TAG, si );

    setBooleanOption( settings.doMax,        el, GTest_UHMM3Search::MAX_OPTION_TAG,     si );
    setBooleanOption( settings.noBiasFilter, el, GTest_UHMM3Search::NOBIAS_OPTION_TAG,  si );
    setBooleanOption( settings.noNull2,      el, GTest_UHMM3Search::NONULL2_OPTION_TAG, si );

    setIntegerOption( settings.seed, el, GTest_UHMM3Search::SEED_OPTION_TAG, si );

    setUseBitCutoffsOption( settings.useBitCutoffs, el, GTest_UHMM3Search::USE_BIT_CUTOFFS_OPTION_TAG, si );
}

static void setSearchAlgoType( GTest_UHMM3SearchAlgoType& alType, const QString& s ) {
    QString str = s.toLower();

    if( "general" == str ) {
        alType = GENERAL_SEARCH;
    } else if( "sw" == str ) {
        alType = SEQUENCE_WALKER_SEARCH;
    } else {
        alType = UNKNOWN_SEARCH;
    }
}

static P7_HMM * takeHmmFromDoc( Document * doc ) {
    assert( NULL != doc );
    QList< GObject* > objs = doc->getObjects();
    assert( 1 == objs.size() );
    UHMMObject * hmmObj = qobject_cast< UHMMObject* >( objs.at( 0 ) );
    if( NULL == hmmObj ) {
        return NULL;
    }
    return hmmObj->takeHMM();
}

void GTest_UHMM3Search::init( XMLTestFormat *tf, const QDomElement& el ) {
    Q_UNUSED( tf );

    hmmFilename = el.attribute( HMM_FILENAME_TAG );
    searchTaskCtxName = el.attribute( HMMSEARCH_TASK_CTX_NAME_TAG );

    searchTaskToCtx     = NULL;
    hmm = NULL;
    
    seqDocCtxName = el.attribute( SEQ_DOC_CTX_NAME_TAG );
    setSearchAlgoType( algo, el.attribute( ALGORITHM_TYPE_OPTION_TAG ) );
    setSearchTaskSettings( settings.inner, el, stateInfo );
    
    swChunk = UHMM3SWSearchTask::DEFAULT_CHUNK_SIZE;
    QString chunkStr = el.attribute(SW_CHUNK_SIZE_OPTION_TAG);
    if( !chunkStr.isEmpty() ) {
        bool ok = false;
        int candidate = chunkStr.toInt(&ok);
        if(ok && candidate > 0) {
            swChunk = candidate;
        }
    }
    
    cleanuped = false;
    ctxAdded = false;
    
    machinePath = env->getVar( REMOTE_MACHINE_VAR );
    if( !machinePath.isEmpty() ) {
        algo = SEQUENCE_WALKER_SEARCH;
    }
}

void GTest_UHMM3Search::setAndCheckArgs() {
    assert( !stateInfo.hasError() );
    if( hmmFilename.isEmpty() ) {
        stateInfo.setError( "hmm_filename_is_empty" );
        return;
    }
    hmmFilename = env->getVar( "COMMON_DATA_DIR" ) + "/" + hmmFilename;

    if( searchTaskCtxName.isEmpty() ) {
        stateInfo.setError( "task_ctx_name_is_empty" );
        return;
    }

    if( seqDocCtxName.isEmpty() ) {
        stateInfo.setError( "sequence_document_ctx_name_is_empty" );
        return;
    }

    if( UNKNOWN_SEARCH == algo ) {
        stateInfo.setError( "unknown_algorithm_type" );
        return;
    }

    Document* seqDoc = getContext<Document>( this, seqDocCtxName );
    if( NULL == seqDoc ) {
        stateInfo.setError( QString( "context %1 not found" ).arg( seqDocCtxName ) );
        return;
    }
    QList< GObject* > objsList = seqDoc->findGObjectByType( GObjectTypes::SEQUENCE );
    CHECK_EXT(!objsList.isEmpty(),setError("No sequence objects found!"), );
    U2SequenceObject* seqObj = qobject_cast< U2SequenceObject* >( objsList.first() );
    sequence = seqObj->getWholeSequence();
    CHECK_EXT(sequence.length() > 0, setError(tr("Sequence is empty")), );
    
    if( !machinePath.isEmpty() ) {
        machinePath = env->getVar( "COMMON_DATA_DIR" ) + "/" + machinePath;
    }
}

void GTest_UHMM3Search::prepare() {
    assert( !hasError() );
    setAndCheckArgs();
    if( hasError() ) {
        return;
    }

    switch( algo ) {
    case GENERAL_SEARCH:
        searchTaskToCtx = new UHMM3LoadProfileAndSearchTask(settings, hmmFilename, sequence.seq);
        addSubTask( searchTaskToCtx );
        break;
    case SEQUENCE_WALKER_SEARCH:
        if( machinePath.isEmpty() ) { /* search task on local machine */
            searchTaskToCtx = new UHMM3SWSearchTask( hmmFilename, sequence, settings, swChunk );
            addSubTask( searchTaskToCtx );
        } else { /* search on remote machine */
            addSubTask( LoadDocumentTask::getDefaultLoadDocTask( hmmFilename ) );
        }
        break;
    default:
        assert( 0 && "undefined_algorithm_type" );
    }
}

QList< Task* > GTest_UHMM3Search::onSubTaskFinished( Task * sub ) {
    assert( NULL != sub );
    QList< Task* > res;
    LoadDocumentTask * loadHmmTask = qobject_cast<LoadDocumentTask*>( sub );
    if( NULL == loadHmmTask ) {
        return res;
    }
    if( loadHmmTask->hasError() ) {
        setError( loadHmmTask->getError() );
        return res;
    }
    
    hmm = takeHmmFromDoc( loadHmmTask->getDocument() );
    assert( NULL != hmm );
    return res;
}

Task::ReportResult GTest_UHMM3Search::report() {
    if( stateInfo.hasError() ) {
        return ReportResult_Finished;
    }
    
    if(searchTaskToCtx != NULL && !searchTaskToCtx->hasError() && !searchTaskToCtx->isCanceled() ) {
        addContext( searchTaskCtxName, searchTaskToCtx );
        ctxAdded = true;
    }
    return ReportResult_Finished;
}

void GTest_UHMM3Search::cleanup() {
    if( cleanuped ) {
        return;
    }
    if( ctxAdded ) {
        removeContext( searchTaskCtxName );
    }
    if( NULL != hmm ) {
        p7_hmm_Destroy( hmm );
    }
    cleanuped = true;
}

GTest_UHMM3Search::~GTest_UHMM3Search() {
    if( !cleanuped ) {
        cleanup();
    }
}

/**************************
* GTest_GeneralUHMM3SearchCompare
**************************/

const int   BUF_SZ      = 2048;
const char  TERM_SYM    = '\0';

static void readLine( IOAdapter* io, QByteArray& to, QStringList* tokens = NULL ) {
    assert( NULL != io );
    to.clear();
    QByteArray buf( BUF_SZ, TERM_SYM );
    bool there = false;
    int bytes = 0;
    while( !there ) {
        int ret = io->readUntil( buf.data(), BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &there );
        if( 0 > ret ) {
            throw QString( "read_error_occurred" );
        }
        if( 0 == ret ) {
            break;
        }
        to.append( QByteArray( buf.data(), ret ) );
        bytes += ret;
    }
    to = to.trimmed();
    if( 0 == bytes ) {
        throw QString( "unexpected_end_of_file_found" );
    }

    if( NULL != tokens ) {
        *tokens = QString( to ).split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
    }
}

static QByteArray getNextToken( QStringList& tokens ) {
    if( tokens.isEmpty() ) {
        throw QString( "unexpected_end_of_line:token_is_missing" );
    }
    return tokens.takeFirst().toLatin1();
}

static double getDouble( const QByteArray& numStr ) {
    bool ok = false;
    double ret = numStr.toDouble( &ok );
    if( ok ) {
        return ret;
    }
    throw QString( GTest_UHMM3SearchCompare::tr( "cannot_parse_double_number_from_string:%1" ).arg( QString( numStr ) ) );
}

static float getFloat( const QByteArray& numStr ) {
    return (float)getDouble( numStr );
}

static bool getSignificance( const QByteArray& str ) {
    if( "!" == str ) {
        return true;
    } else if( "?" == str ) {
        return false;
    }
    throw QString( GTest_UHMM3SearchCompare::tr( "cannot_parse_significance:%1" ).arg( QString( str ) ) );
}

static UHMM3SearchSeqDomainResult getDomainRes( QStringList& tokens ) {
    UHMM3SearchSeqDomainResult res;

    getNextToken( tokens );
    res.isSignificant = getSignificance( getNextToken( tokens ) );
    res.score   = getFloat( getNextToken( tokens ) );
    res.bias    = getFloat( getNextToken( tokens ) );
    res.cval    = getDouble( getNextToken( tokens ) );
    res.ival    = getDouble( getNextToken( tokens ) );

    int hmmFrom = (int)getFloat( getNextToken( tokens ) );
    int hmmTo   = (int)getFloat( getNextToken( tokens ) );
    res.queryRegion = U2Region( hmmFrom, hmmTo - hmmFrom );
    getNextToken( tokens );

    int aliFrom = (int)getFloat( getNextToken( tokens ) );
    int aliTo   = (int)getFloat( getNextToken( tokens ) );
    res.seqRegion = U2Region( aliFrom - 1, aliTo - aliFrom + 1 );
    getNextToken( tokens );

    int envFrom = (int)getFloat( getNextToken( tokens ) );
    int envTo   = (int)getFloat( getNextToken( tokens ) );
    res.envRegion = U2Region( envFrom, envTo - envFrom );
    getNextToken( tokens );

    res.acc = getDouble( getNextToken( tokens ) );
    return res;
}

const double COMPARE_PERCENT_BORDER = 0.01; // 1 percent

template<class T>
static bool compareNumbers( T f1, T f2 ) {
    bool ret = false;
    if( 0 == f1 ) {
        ret = 0 == f2 ? true : f2 < COMPARE_PERCENT_BORDER;
    } else if( 0 == f2 ) {
        ret = f1 < COMPARE_PERCENT_BORDER;
    } else {
        ret = ( qAbs( f1 - f2 ) ) / f1 < COMPARE_PERCENT_BORDER;
    }

    if( !ret ) {
        qDebug() << "!!! compare numbers mismatch: " << f1 << " and " << f2 << " !!!\n";
    }
    
    return ret;
}

void GTest_UHMM3SearchCompare::generalCompareResults( const UHMM3SearchResult& myRes, const UHMM3SearchResult& trueRes, TaskStateInfo& ti ) {
    const UHMM3SearchCompleteSeqResult& myFull = myRes.fullSeqResult;
    const UHMM3SearchCompleteSeqResult& trueFull = trueRes.fullSeqResult;
    
    if( myFull.isReported != trueFull.isReported ) {
        ti.setError( QString( "reported_flag_not_matched: %1 and %2" ).arg( myFull.isReported ).arg( trueFull.isReported ) );
        return;
    }
    
    if( myFull.isReported ) {
        if( !compareNumbers<float>( myFull.bias, trueFull.bias )   ) { 
            ti.setError( QString( "full_seq_bias_not_matched: %1 and %2" ).arg( myFull.bias ).arg( trueFull.bias ) );  return; 
        }
        if( !compareNumbers<double>( myFull.eval, trueFull.eval )  ) {
            ti.setError( QString( "full_seq_eval_not_matched: %1 and %2" ).arg( myFull.eval ).arg( trueFull.eval ) );  return; 
        }
        if( !compareNumbers<float>( myFull.score, trueFull.score ) ) {
            ti.setError( QString( "full_seq_score_not_matched: %1 and %2" ).arg( myFull.score ).arg( trueFull.score ) ); return; 
        }
        if( !compareNumbers<float>( myFull.expectedDomainsNum, trueFull.expectedDomainsNum ) ) {
            ti.setError( QString( "full_seq_exp_not_matched: %1 and %2" ).arg( myFull.expectedDomainsNum ).arg( trueFull.expectedDomainsNum ) ); 
            return; 
        }
        if( myFull.reportedDomainsNum != trueFull.reportedDomainsNum ) { 
            ti.setError( QString( "full_seq_n_not_matched: %1 and %2" ).arg( myFull.reportedDomainsNum ).arg( trueFull.reportedDomainsNum ) );
            return; 
        }
    }
    
    const QList< UHMM3SearchSeqDomainResult >& myDoms = myRes.domainResList;
    const QList< UHMM3SearchSeqDomainResult >& trueDoms = trueRes.domainResList;
    if( myDoms.size() != trueDoms.size() ) {
        ti.setError( QString( "domain_res_number_not_matched: %1 and %2" ).arg( myDoms.size() ).arg( trueDoms.size() ) );
        return;
    }
    for( int i = 0; i < myDoms.size(); ++i ) {
        UHMM3SearchSeqDomainResult myCurDom = myDoms.at( i );
        UHMM3SearchSeqDomainResult trueCurDom = trueDoms.at( i );
        if( !compareNumbers<double>( myCurDom.acc, trueCurDom.acc ) )   { 
            ti.setError( QString( "dom_acc_not_matched: %1 and %2" ).arg( myCurDom.acc ).arg( trueCurDom.acc ) );   return; 
        }
        if( !compareNumbers<float>( myCurDom.bias, trueCurDom.bias ) )  { 
            ti.setError( QString( "dom_bias_not_matched: %1 and %2" ).arg( myCurDom.bias ).arg( trueCurDom.bias ) );  return; 
        }
        if( !compareNumbers<double>( myCurDom.cval, trueCurDom.cval ) )  { 
            ti.setError( QString( "dom_cval_not_matched: %1 and %2" ).arg( myCurDom.cval ).arg( trueCurDom.cval ) );  return; 
        }
        if( !compareNumbers<double>( myCurDom.ival, trueCurDom.ival ) )  { 
            ti.setError( QString( "dom_ival_not_matched: %1 and %2" ).arg( myCurDom.ival ).arg( trueCurDom.ival ) );  return; 
        }
        if( !compareNumbers<float>( myCurDom.score, trueCurDom.score ) ) { 
            ti.setError( QString( "dom_score_not_matched: %1 and %2" ).arg( myCurDom.score ).arg( trueCurDom.score ) ); return; 
        }
        if( myCurDom.envRegion != trueCurDom.envRegion ) { 
            ti.setError( QString( "dom_env_region_not_matched: %1---%2 and %3---%4" ).
                arg( myCurDom.envRegion.startPos ).arg( myCurDom.envRegion.length ).arg( trueCurDom.envRegion.startPos ).
                arg( trueCurDom.envRegion.length ) ); return;
        }
        if( myCurDom.queryRegion != trueCurDom.queryRegion ) { 
            ti.setError( QString( "dom_hmm_region_not_matched: %1---%2 and %3---%4" ).
                arg( myCurDom.queryRegion.startPos ).arg( myCurDom.queryRegion.length ).arg( trueCurDom.queryRegion.startPos ).
                arg( trueCurDom.queryRegion.length ) ); return;
        }
        if( myCurDom.seqRegion != trueCurDom.seqRegion ) {
            ti.setError( QString( "dom_seq_region_not_matched: %1---%2 and %3---%4" ).
                arg( myCurDom.seqRegion.startPos ).arg( myCurDom.seqRegion.length ).arg( trueCurDom.seqRegion.startPos ).
                arg( trueCurDom.seqRegion.length ) ); return;
        }
        if( myCurDom.isSignificant != trueCurDom.isSignificant ) { 
            ti.setError( QString( "dom_sign_not_matched: %1 and %2" ).arg( myCurDom.isSignificant ).arg( trueCurDom.isSignificant ) );
            return; 
        }
    }
}

static QList<int>
findEqualDomain(const QList<UHMM3SWSearchTaskDomainResult>& res, const UHMM3SearchSeqDomainResult & dres, bool compareSeqRegion) {
    QList<int> diff;
    for(int i = 0; i < res.size(); ++i) {
        UHMM3SearchSeqDomainResult dom = res.at(i).generalResult;
        int count = 0;
        if( !compareNumbers<double>( dom.acc, dres.acc ) )    { count++; }
        if( !compareNumbers<float>( dom.bias, dres.bias ) )   { count++; }
        if( !compareNumbers<double>( dom.cval, dres.cval ) )  { count++; }
        if( !compareNumbers<double>( dom.ival, dres.ival ) )  { count++; }
        if( !compareNumbers<float>( dom.score, dres.score ) ) { count++; }
        if( dom.queryRegion != dres.queryRegion ) { count++; }
        if( compareSeqRegion && dom.seqRegion != dres.seqRegion ) { count++; }
        if( compareSeqRegion && dom.envRegion != dres.envRegion ) { count++; }
        if( dom.isSignificant != dres.isSignificant ) { count++; }
        diff << count;
    }
    return diff;
}

static QString seqDomainResult2String(const UHMM3SearchSeqDomainResult & r) {
    return QString("score=%1, eval=%2, bias=%3, acc=%4, query=%5 seq=%6").arg(r.score).arg(r.ival).arg(r.bias).arg(r.acc).
        arg(QString("%1..%2").arg(r.queryRegion.startPos).arg(r.queryRegion.endPos())).
        arg(QString("%1..%2").arg(r.seqRegion.startPos).arg(r.seqRegion.endPos()));
}

/* we compare here that every domain of trueResult is included in myResult */
void
GTest_UHMM3SearchCompare::swCompareResults( const QList<UHMM3SWSearchTaskDomainResult>& myR, const UHMM3SearchResult& trueR, 
                                            TaskStateInfo& ti, bool compareSeqRegion ) {
    int sz = trueR.domainResList.size();
    int i = 0;
    for( i = 0; i < sz; ++i ) {
        const UHMM3SearchSeqDomainResult & trueDom = trueR.domainResList.at(i);
        if(trueDom.score < 2) {
            continue;
        }
        QList<int> diff = findEqualDomain(myR, trueDom, compareSeqRegion);
        if(!diff.contains(0)) {
            int minPos = 0;
            int min = 1000000;
            for(int j = 0; j < myR.size(); ++j) {
                float d = qAbs(myR.at(j).generalResult.score - trueR.domainResList.at(i).score);
                if( d < min ) {
                    min = d;
                    minPos = j;
                }
            }
            if(!myR.isEmpty()) {
                ti.setError( QString( "Cannot find result #%1: %2, most close result: %3" ).
                    arg(i).
                    arg(seqDomainResult2String(trueR.domainResList.at(i))).
                    arg(seqDomainResult2String(myR.at(minPos).generalResult)));
            } else {
                ti.setError( QString( "Cannot find result #%1: %2" ).
                    arg(i).arg(seqDomainResult2String(trueR.domainResList.at(i))));
            }
            return;
        }
    }
}

const QString GTest_UHMM3SearchCompare::SEARCH_TASK_CTX_NAME_TAG = "searchTask";
const QString GTest_UHMM3SearchCompare::TRUE_OUT_FILE_TAG        = "trueOut";

void GTest_UHMM3SearchCompare::init( XMLTestFormat *tf, const QDomElement& el ) {
    Q_UNUSED( tf );

    searchTaskCtxName = el.attribute( SEARCH_TASK_CTX_NAME_TAG );
    trueOutFilename = el.attribute( TRUE_OUT_FILE_TAG );
}

void GTest_UHMM3SearchCompare::setAndCheckArgs() {
    assert( !hasError() );

    if( searchTaskCtxName.isEmpty() ) {
        stateInfo.setError( "search_task_ctx_name_is_empty" );
        return;
    }

    if( trueOutFilename.isEmpty() ) {
        stateInfo.setError( "true_out_filename_is_empty" );
        return;
    }
    trueOutFilename = env->getVar( "COMMON_DATA_DIR" ) + "/" + trueOutFilename;

    Task* searchTask = getContext<Task>( this, searchTaskCtxName );
    if( NULL == searchTask ) {
        stateInfo.setError( tr( "cannot_find_search_task_in_context" ) );
        return;
    }
    
    generalTask = qobject_cast< UHMM3LoadProfileAndSearchTask* >( searchTask );
    swTask      = qobject_cast< UHMM3SWSearchTask*  >( searchTask );
    
    if( NULL != generalTask ) {
        algo = GENERAL_SEARCH;
    } else if (NULL != swTask) {
        algo = SEQUENCE_WALKER_SEARCH;
    } else {
        assert( 0 && "cannot_cast_task_to_search_task" );
    }
}

bool GTest_UHMM3SearchCompare::searchResultLessThan(const UHMM3SearchSeqDomainResult & r1, const UHMM3SearchSeqDomainResult & r2) {
    if( r1.score == r2.score ) {
        if(r1.seqRegion == r2.seqRegion) {
            return &r1 < &r2;
        }
        return r1.seqRegion < r2.seqRegion;
    }
    return r1.score > r2.score;
}

UHMM3SearchResult GTest_UHMM3SearchCompare::getOriginalSearchResult( const QString & filename ) {
    assert( !filename.isEmpty() );
    
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( IOAdapterUtils::url2io( filename ) );
    std::auto_ptr< IOAdapter > io( iof->createIOAdapter() );
    if( NULL == io.get() ) {
        throw QString( "cannot_create_io_adapter_for_'%1'_file" ).arg( filename );
    }
    if( !io->open( filename, IOAdapterMode_Read ) ) {
        throw QString( "cannot_open_'%1'_file" ).arg( filename );
    }
    
    UHMM3SearchResult res;
    QByteArray buf;
    QStringList tokens;
    bool wasHeader = false;
    bool wasFullSeqResult = false;
    readLine( io.get(), buf ); /* the first line. starts with # search or # phmmer */
    do {
        readLine( io.get(), buf );
        if( buf.isEmpty() ) { /* but no error - empty line here */
            continue;
        }
        if( buf.startsWith( "# HMMER 3" ) ) {
            wasHeader = true;
            continue;
        }
        if( buf.startsWith( "Scores for complete sequences" ) ) {
            if( !wasHeader ) {
                throw QString( "hmmer_output_header_is_missing" );
            }
            UHMM3SearchCompleteSeqResult& fullSeqRes = res.fullSeqResult;
            readLine( io.get(), buf );
            readLine( io.get(), buf );
            readLine( io.get(), buf );
            readLine( io.get(), buf, &tokens );
            if( buf.startsWith( "[No hits detected" ) ) {
                fullSeqRes.isReported = false;
                break;
            } else {
                fullSeqRes.eval     = getDouble( getNextToken( tokens ) );
                fullSeqRes.score    = getFloat( getNextToken( tokens ) );
                fullSeqRes.bias     = getFloat( getNextToken( tokens ) );
                /* skip best domain res. we will check it later */
                getNextToken( tokens );getNextToken( tokens );getNextToken( tokens );
                fullSeqRes.expectedDomainsNum = getFloat( getNextToken( tokens ) );
                fullSeqRes.reportedDomainsNum = (int)getFloat( getNextToken( tokens ) );
                fullSeqRes.isReported = true;
                wasFullSeqResult = true;
            }
            continue;
        }
        if( buf.startsWith( "Domain annotation for each sequence" ) ) {
            if( !wasFullSeqResult ) {
                throw QString( "full_seq_result_is_missing" );
            }
            readLine( io.get(), buf );
            readLine( io.get(), buf );
            readLine( io.get(), buf );
            QList< UHMM3SearchSeqDomainResult >& domainResList = res.domainResList;
            assert( domainResList.isEmpty() );

            int nDomains = res.fullSeqResult.reportedDomainsNum;
            int i = 0;
            for( i = 0; i < nDomains; ++i ) {
                readLine( io.get(), buf, &tokens );
                domainResList << getDomainRes( tokens );
            }
            break;
        }
    } while ( 1 );
    return res;
}

Task::ReportResult GTest_UHMM3SearchCompare::report() {
    assert( !hasError() );
    setAndCheckArgs();
    if( hasError() ) {
        return ReportResult_Finished;
    }
    
    UHMM3SearchResult trueRes;
    try {
        trueRes = getOriginalSearchResult( trueOutFilename );
    } catch( const QString& ex ) {
        stateInfo.setError( ex );
    } catch(...) {
        stateInfo.setError( "undefined_error_occurred" );
    }

    if( hasError() ) {
        return ReportResult_Finished;
    }

    switch( algo ) {
    case GENERAL_SEARCH:{
        assert( NULL != generalTask );
        QList<UHMM3SearchResult> res = generalTask->getResult();
        if(res.size() < 1){
            stateInfo.setError("no result");
            return ReportResult_Finished;    
        }
        generalCompareResults(res.first() , trueRes, stateInfo );
        break;
    }
    case SEQUENCE_WALKER_SEARCH:
        {
            QList<UHMM3SWSearchTaskDomainResult> result;
            if( NULL != swTask ) {
                result = swTask->getResults();
            } else {
                assert( false );
            }
            qSort(trueRes.domainResList.begin(), trueRes.domainResList.end(), searchResultLessThan);
            swCompareResults( result, trueRes, stateInfo );
        }
        break;
    default:
        assert( 0 && "unknown_algo_type" );
    }
    
    return ReportResult_Finished;
}

} // U2
