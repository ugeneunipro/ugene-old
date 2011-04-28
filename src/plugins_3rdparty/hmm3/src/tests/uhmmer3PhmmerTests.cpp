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

#include <U2Core/AppContext.h>
#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2Core/L10n.h>

#include "uhmmer3BuildTests.h"
#include "uhmmer3SearchTests.h"
#include "uhmmer3PhmmerTests.h"

namespace U2 {

/****************************************
 * GTest_UHMM3Phmmer
 ****************************************/

const QString GTest_UHMM3Phmmer::QUERY_FILENAME_TAG             = "query";
const QString GTest_UHMM3Phmmer::DB_FILENAME_TAG                = "db";
const QString GTest_UHMM3Phmmer::PHMMER_TASK_CTX_NAME_TAG       = "ctxName";

const QString GTest_UHMM3Phmmer::GAP_OPEN_PROBAB_OPTION_TAG     = "popen";
const QString GTest_UHMM3Phmmer::GAP_EXTEND_PROBAB_OPTION_TAG   = "pextend";
const QString GTest_UHMM3Phmmer::SUBST_MATR_NAME_OPTION_TAG     = "substMatr";
const QString GTest_UHMM3Phmmer::SEARCH_CHUNK_OPTION_TAG        = "chunk";

const double BAD_DOUBLE_OPTION = -1.0;

static void setDoubleOption( double & to, const QString & str, TaskStateInfo & ti ) {
    if( str.isEmpty() ) {
        return;
    }
    bool ok = false;
    to = str.toDouble( &ok );
    if( !ok ) {
        to = BAD_DOUBLE_OPTION;
        ti.setError( QString( "cannot_parse_double_from: %1" ).arg( str ) );
    }
}

static void setSubstMatrixOption( SMatrix& to, const QString & str, TaskStateInfo & ti ) {
    if( str.isEmpty() ) {
        to = SMatrix();
        return;
    }
    SubstMatrixRegistry * smr = AppContext::getSubstMatrixRegistry();
    assert( NULL != smr );
    
    if( smr->getMatrix( str ).isEmpty() ) {
        ti.setError( QString( "matrix %1 not registered" ).arg( str ) );
        return;
    }
    to = smr->getMatrix(str);
    assert(!to.isEmpty());
}

void GTest_UHMM3Phmmer::init( XMLTestFormat *tf, const QDomElement& el ) {
    Q_UNUSED( tf );
    
    phmmerTask      = NULL;
    queryFilename   = el.attribute( QUERY_FILENAME_TAG );
    dbFilename      = el.attribute( DB_FILENAME_TAG );
    taskCtxName     = el.attribute( PHMMER_TASK_CTX_NAME_TAG );
    ctxAdded        = false;
    loadDbTask      = NULL;
    
    UHMM3SearchSettings searchSettings;
    setDefaultUHMM3SearchSettings( &searchSettings );
    GTest_UHMM3Search::setSearchTaskSettings( searchSettings, el, settigsStateInfo );
    if( settigsStateInfo.hasError() ) { return; }
    settings.setSearchSettings( searchSettings );
    
    UHMM3BuildSettings buildSettings;
    setDefaultUHMM3BuildSettings( &buildSettings );
    GTest_UHMMER3Build::setBuildSettings( buildSettings, el, settigsStateInfo );
    settings.setBuildSettings( buildSettings );
    if( settigsStateInfo.hasError() ) { return; }
    
    setDoubleOption( settings.popen, el.attribute( GAP_OPEN_PROBAB_OPTION_TAG ), scoringSystemStateInfo );
    if( scoringSystemStateInfo.hasError() ) { return; }
    setDoubleOption( settings.pextend, el.attribute( GAP_EXTEND_PROBAB_OPTION_TAG ), scoringSystemStateInfo );
    if( scoringSystemStateInfo.hasError() ) { return; }
    setSubstMatrixOption( settings.substMatr, el.attribute( SUBST_MATR_NAME_OPTION_TAG ), scoringSystemStateInfo );
    assert( settings.substMatr.isEmpty() );
    if( scoringSystemStateInfo.hasError() ) { return; }
    
    chunk = UHMM3SWPhmmerTask::DEFAULT_CHUNK_SIZE;
    QString chunkStr = el.attribute(SEARCH_CHUNK_OPTION_TAG);
    if(!chunkStr.isEmpty()) {
        bool ok = false;
        int chunkCandidate = chunkStr.toInt(&ok);
        if(ok && chunkCandidate > 0) {
            chunk = chunkCandidate;
        }
    }
}

void GTest_UHMM3Phmmer::setAndCheckArgs() {
    assert( !stateInfo.hasError() );
    
    if( queryFilename.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( "query sequence filename" ) );
        return;
    }
    queryFilename = env->getVar( "COMMON_DATA_DIR" ) + "/" +queryFilename;
    
    if( dbFilename.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( "db sequence filename" ) );
        return;
    }
    dbFilename = env->getVar( "COMMON_DATA_DIR" ) + "/" + dbFilename;
    
    if( taskCtxName.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( "phmmer task context name" ) );
        return;
    }
    
    if( settigsStateInfo.hasError() ) {
        stateInfo.setError( settigsStateInfo.getError() );
        return;
    }
    if( scoringSystemStateInfo.hasError() ) {
        stateInfo.setError( scoringSystemStateInfo.getError() );
        return;
    }
}

void GTest_UHMM3Phmmer::prepare() {
    assert( !hasError() && NULL == phmmerTask );
    setAndCheckArgs();
    if( hasError() ) {
        return;
    }
    loadDbTask = LoadDocumentTask::getDefaultLoadDocTask(dbFilename);
    if(loadDbTask == NULL) {
        setError(L10N::errorOpeningFileRead(dbFilename));
        return;
    }
    addSubTask(loadDbTask);
}

QList<Task*> GTest_UHMM3Phmmer::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask->hasError()) {
        setError(subTask->getError());
        return res;
    }
    
    if( loadDbTask == subTask ) {
        DNASequence dbSeq = UHMM3PhmmerTask::getSequenceFromDocument( loadDbTask->getDocument(), stateInfo );
        if( hasError() ) {
            setError(getError());
        }
        phmmerTask = new UHMM3SWPhmmerTask( queryFilename, dbSeq, settings, chunk );
        res << phmmerTask;
    } else if(phmmerTask != subTask) {
        assert(false);
    }
    return res;
}

Task::ReportResult GTest_UHMM3Phmmer::report() {
    if( hasError() ) {
        return ReportResult_Finished;
    }
    assert( NULL != phmmerTask );
    
    if( !phmmerTask->isCanceled() && !phmmerTask->hasError() ) {
        addContext( taskCtxName, phmmerTask );
        ctxAdded = true;
    }
    return ReportResult_Finished;
}

void GTest_UHMM3Phmmer::cleanup() {
    if( NULL != phmmerTask && ctxAdded ) {
        removeContext( taskCtxName );
    }
}

GTest_UHMM3Phmmer::~GTest_UHMM3Phmmer() {
}

/****************************************
* GTest_UHMM3PhmmerCompare
****************************************/
const QString GTest_UHMM3PhmmerCompare::PHMMER_TASK_CTX_NAME_TAG    = "phmmerTask";
const QString GTest_UHMM3PhmmerCompare::TRUE_OUT_FILENAME_TAG       = "trueOut";

void GTest_UHMM3PhmmerCompare::init( XMLTestFormat *tf, const QDomElement& el ) {
    Q_UNUSED( tf );
    
    phmmerTaskCtxName = el.attribute( PHMMER_TASK_CTX_NAME_TAG );
    trueOutFilename   = el.attribute( TRUE_OUT_FILENAME_TAG );
}

void GTest_UHMM3PhmmerCompare::setAndCheckArgs() {
    if( phmmerTaskCtxName.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( "phmmer task context name" ) );
        return;
    }
    if( trueOutFilename.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( "true out filename" ) );
        return;
    }
    trueOutFilename = env->getVar( "COMMON_DATA_DIR" ) + "/" + trueOutFilename;
    
    phmmerTask = getContext< UHMM3SWPhmmerTask >( this, phmmerTaskCtxName );
    if( NULL == phmmerTask ) {
        stateInfo.setError( QString( "cannot find phmmer task %1 in context" ).arg( phmmerTaskCtxName ) );
        return;
    }
}

Task::ReportResult GTest_UHMM3PhmmerCompare::report() {
    assert( !hasError() );
    setAndCheckArgs();
    if( hasError() ) {
        return ReportResult_Finished;
    }
    
    UHMM3SearchResult trueRes;
    try {
        trueRes = GTest_UHMM3SearchCompare::getOriginalSearchResult( trueOutFilename );
    } catch( const QString& ex ) {
        stateInfo.setError( ex );
    } catch(...) {
        stateInfo.setError( "undefined_error_occurred" );
    }
    
    if( hasError() ) {
        return ReportResult_Finished;
    }
    
    assert( NULL != phmmerTask );
    qSort(trueRes.domainResList.begin(), trueRes.domainResList.end(), GTest_UHMM3SearchCompare::searchResultLessThan);
    GTest_UHMM3SearchCompare::swCompareResults(phmmerTask->getResult(), trueRes, stateInfo, true);
    return ReportResult_Finished;
}

} // U2
