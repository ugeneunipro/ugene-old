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

#include <QtCore/QMutexLocker>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AppResources.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/L10n.h>
#include <U2Core/Counter.h>
#include <U2Core/Log.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AppContext.h>

#include <task_local_storage/uHMMSearchTaskLocalStorage.h>

#include "uhmm3phmmer.h"
#include "uhmm3PhmmerTask.h"

#define UHMM3_PHMMER_LOG_CAT "uhmm3_phmmer_log_category"

using namespace U2;

namespace U2 {


static int countPhmmerMemInMB( qint64 dbLen, int queryLen ) {
    assert( 0 < dbLen && 0 < queryLen );
    return qMax( ( ( double )dbLen * queryLen / ( 1024 * 1024 ) ) * 10, 2.0 );
}

/**************************************
* General hmmer3 phmmer task.
**************************************/

UHMM3PhmmerTask::UHMM3PhmmerTask( const DNASequence & q, const DNASequence & d, const UHMM3PhmmerSettings & set )
: Task( tr( "HMM Phmmer task" ), TaskFlag_None ), query( q ), db( d ), settings( set ), loadQueryTask( NULL ), loadDbTask( NULL ) {
    GCOUNTER( cvar, tvar, "UHMM3PhmmerTask" );
    if( 0 == query.length() ) {
        stateInfo.setError( L10N::badArgument( tr( "query_sequence" ) ) );
        return;
    }
    if( 0 == db.length() ) {
        stateInfo.setError( L10N::badArgument( tr( "sequence_to_search_in" ) ) );
        return;
    }
    setTaskName( tr( "HMM Phmmer search %1 sequence in %2 database" ).arg( query.getName() ).arg( db.getName() ) );
    
    addMemResource();
}

UHMM3PhmmerTask::UHMM3PhmmerTask( const QString & queryFilename, const QString & dbFilename, const UHMM3PhmmerSettings & set ) 
: Task( tr( "HMM Phmmer task" ), TaskFlag_None ), settings( set ), loadQueryTask( NULL ), loadDbTask( NULL ) {
    if( queryFilename.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( tr( "query_sequence_filename" ) ) );
        return;
    }
    if( dbFilename.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( tr( "db_sequence_to_search_in" ) ) );
        return;
    }
    setTaskName( tr( "HMM Phmmer search %1 sequence with %2 database" ).arg( queryFilename ).arg( dbFilename ) );
    
    loadQueryTask = LoadDocumentTask::getDefaultLoadDocTask( queryFilename );
    if( NULL == loadQueryTask ) {
        stateInfo.setError( tr( "cannot_create_load_query_doc_task" ) );
        return;
    }
    addSubTask( loadQueryTask );
    loadDbTask = LoadDocumentTask::getDefaultLoadDocTask( dbFilename );
    if( NULL == loadDbTask ) {
        stateInfo.setError( tr( "cannot_create_load_db_doc_task" ) );
        return;
    }
    addSubTask( loadDbTask );
}

UHMM3PhmmerTask::UHMM3PhmmerTask( const QString & queryFilename, const DNASequence & d, const UHMM3PhmmerSettings & s )
: Task( tr( "HMM Phmmer task" ), TaskFlag_None ), db( d ), settings( s ), loadQueryTask( NULL ), loadDbTask( NULL ) {
    if( queryFilename.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( tr( "query_sequence_filename" ) ) );
        return;
    }
    if( 0 == db.length() ) {
        stateInfo.setError( L10N::badArgument( tr( "sequence_to_search_in" ) ) );
        return;
    }
    setTaskName( tr( "HMM Phmmer search %1 sequence in %2 database" ).arg( queryFilename ).arg( db.getName() ) );
    
    loadQueryTask = LoadDocumentTask::getDefaultLoadDocTask( queryFilename );
    if( NULL == loadQueryTask ) {
        stateInfo.setError( tr( "cannot_create_load_query_doc_task" ) );
        return;
    }
    addSubTask( loadQueryTask );
}

void UHMM3PhmmerTask::addMemResource() {
    assert( !db.isNull() && !query.isNull() );
    
    int howManyMem = countPhmmerMemInMB( db.length(), query.length() );
    addTaskResource(TaskResourceUsage( RESOURCE_MEMORY, howManyMem ));
    algoLog.trace( QString( "%1 requires %2 of memory" ).arg( getTaskName() ).arg( howManyMem ) );
}

DNASequence UHMM3PhmmerTask::getSequenceFromDocument( Document * doc, TaskStateInfo & ti ) {
    DNASequence ret;
    if( NULL == doc ) {
        ti.setError( tr( "cannot load document from:" ) );
        return ret;
    }
    
    QList< GObject* > objsList = doc->findGObjectByType( GObjectTypes::SEQUENCE );
    if( objsList.isEmpty() ) {
        ti.setError( tr( "no_dna_sequence_objects_in_document" ) );
        return ret;
    }
    DNASequenceObject* seqObj = qobject_cast< DNASequenceObject* >( objsList.first() );
    if( NULL == seqObj ) {
        ti.setError( tr( "cannot_cast_to_dna_object" ) );
        return ret;
    }
    ret = seqObj->getDNASequence();
    if( !ret.length() ) {
        ti.setError( tr( "empty_sequence_given" ) );
        return ret;
    }
    return ret;
}

QList< Task* > UHMM3PhmmerTask::onSubTaskFinished( Task* subTask ) {
    QMutexLocker locker( &loadTasksMtx );
    QList< Task* > ret;
    assert( NULL != subTask );
    if( hasError() ) {
        return ret;
    }
    if( subTask->hasError() ) {
        stateInfo.setError( subTask->getError() );
        return ret;
    }
    
    if( loadQueryTask == subTask ) {
        query = getSequenceFromDocument( loadQueryTask->getDocument(), stateInfo );
        if( hasError() ) {
            stateInfo.setError( getError() + tr( "query sequence" ) );
        }
        loadQueryTask = NULL;
    } else if( loadDbTask == subTask ) {
        db = getSequenceFromDocument( loadDbTask->getDocument(), stateInfo );
        if( hasError() ) {
            stateInfo.setError( getError() + tr( "db sequence" ) );
        }
        loadDbTask = NULL;
    } else {
        assert( false && "undefined task finished" );
    }
    
    if( NULL == loadQueryTask && NULL == loadDbTask ) {
        addMemResource();
    }
    return ret;
}

UHMM3SearchResult UHMM3PhmmerTask::getResult() const {
    return result;
}

QList< SharedAnnotationData > UHMM3PhmmerTask::getResultsAsAnnotations( const QString & name ) const {
    assert( !name.isEmpty() );
    QList< SharedAnnotationData > annotations;
    
    foreach( const UHMM3SearchSeqDomainResult & domain, result.domainResList ) {
        AnnotationData * annData = new AnnotationData();
        
        annData->name = name;
        annData->setStrand(U2Strand::Direct);
        annData->location->regions << domain.seqRegion;
        annData->qualifiers << U2Qualifier( "Query sequence", query.getName() );
        domain.writeQualifiersToAnnotation( annData );
        
        annotations << SharedAnnotationData( annData );
    }
    
    return annotations;
}

void UHMM3PhmmerTask::run() {
    if( hasError() ) {
        return;
    }
    
    UHMM3SearchTaskLocalStorage::createTaskContext( getTaskId() );
    result = UHMM3Phmmer::phmmer( query.seq.data(), query.length(), db.seq.data(), db.length(), settings, stateInfo, db.length() );
    UHMM3SearchTaskLocalStorage::freeTaskContext( getTaskId() );
}

/*******************************************
 * HMMER3 phmmer sequence walker task
 ********************************************/
UHMM3SWPhmmerTask::UHMM3SWPhmmerTask(const QString & qF, const DNASequence & db, const UHMM3PhmmerSettings & s, int ch )
: Task( "", TaskFlag_NoRun ), queryFilename(qF), dbSeq(db), settings(s), searchChunkSize(ch), loadQueryTask(NULL), swTask(NULL),
  complTranslation(NULL), aminoTranslation(NULL) {
    GCOUNTER( cvar, tvar, "UHMM3SWPhmmerTask" );
    
    assert(searchChunkSize > 0);
    setTaskName(tr("HMM Phmmer search %1 sequence in %2 database").arg(queryFilename).arg(db.getName()));
    if( queryFilename.isEmpty() ) {
        setError(L10N::badArgument("querySeq filename"));
        return;
    }
    if(!dbSeq.seq.length()) {
        setError(L10N::badArgument("sequence"));
        return;
    }
    
    loadQueryTask = LoadDocumentTask::getDefaultLoadDocTask( queryFilename );
    if( NULL == loadQueryTask ) {
        stateInfo.setError( tr( "cannot_create_load_query_doc_task" ) );
        return;
    }
    addSubTask( loadQueryTask );
}

QList<Task*> UHMM3SWPhmmerTask::onSubTaskFinished(Task* subTask) {
    assert(subTask != NULL);
    QList<Task*> res;
    if( subTask->hasError() ) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    
    if( loadQueryTask == subTask ) {
        querySeq = UHMM3PhmmerTask::getSequenceFromDocument( loadQueryTask->getDocument(), stateInfo );
        if( hasError() ) {
            setError(getError() + tr( "querySeq sequence" ));
        }
        swTask = getSWSubtask();
        res << swTask;
    } else {
        assert(swTask == subTask);
    }
    return res;
}

void UHMM3SWPhmmerTask::checkAlphabets() {
    assert(!hasError());
    if(dbSeq.alphabet->isRaw()) {
        setError(tr("Invalid db sequence alphabet: %1").arg(dbSeq.alphabet->getName()));
        return;
    }
    if(querySeq.alphabet->isRaw()) {
        setError(tr("Invalid query sequence alphabet: %1").arg(querySeq.alphabet->getName()));
        return;
    }
}

void UHMM3SWPhmmerTask::setTranslations() {
    assert(!hasError());
    if(dbSeq.alphabet->isNucleic()) {
        DNATranslationRegistry* transReg = AppContext::getDNATranslationRegistry();
        assert( NULL != transReg );
        QList< DNATranslation* > complTs = transReg->lookupTranslation( dbSeq.alphabet, DNATranslationType_NUCL_2_COMPLNUCL );
        if (!complTs.empty()) {
            complTranslation = complTs.first();
        }
        if( querySeq.alphabet->isAmino() ) {
            QList< DNATranslation* > aminoTs = transReg->lookupTranslation( dbSeq.alphabet, DNATranslationType_NUCL_2_AMINO );
            if( !aminoTs.empty() ) {
            aminoTranslation = aminoTs.first();
            }
        }
    } else {
        assert(dbSeq.alphabet->isAmino());
        if( querySeq.alphabet->isNucleic() ) {
            setError(tr("Cannot search for nucleic query in amino sequence"));
            return;
        }
    }
}

SequenceWalkerTask * UHMM3SWPhmmerTask::getSWSubtask() {
    assert(!hasError());
    assert(querySeq.length());
    
    checkAlphabets();
    if(hasError()) {
        return NULL;
    }
    setTranslations();
    if(hasError()) {
        return NULL;
    }
    
    SequenceWalkerConfig config;
    config.seq                  = dbSeq.seq.data();
    config.seqSize              = dbSeq.seq.size();
    config.complTrans           = complTranslation;
    config.strandToWalk         = complTranslation == NULL ? StrandOption_DirectOnly : StrandOption_Both;
    config.aminoTrans           = aminoTranslation;
    /*config.overlapSize          = 2 * querySeq.length();
    config.chunkSize            = qMax(searchChunkSize, 6 * querySeq.length());*/
    config.overlapSize          = 0;
    config.chunkSize            = config.seqSize;
    config.lastChunkExtraLen    = config.chunkSize / 2;
    config.nThreads             = MAX_PARALLEL_SUBTASKS_AUTO;
    return new SequenceWalkerTask( config, this, tr( "HMMER3 phmmer sequence walker search task" ) );
}

void UHMM3SWPhmmerTask::onRegion(SequenceWalkerSubtask * t, TaskStateInfo & ti) {
    assert(t != NULL);
    if(hasError() || ti.hasError() || isCanceled() || ti.cancelFlag) {
        return;
    }
    
    const char * seq    = t->getRegionSequence();
    int seqLen          = t->getRegionSequenceLen();
    bool isAmino        = t->isAminoTranslated();
    U2Region globalRegion = t->getGlobalRegion();
    
    UHMM3SearchTaskLocalStorage::createTaskContext( t->getTaskId() );
    int wholeSeqSz = t->getGlobalConfig().seqSize;
    wholeSeqSz = isAmino ? (wholeSeqSz / 3) : wholeSeqSz;
    UHMM3SearchResult generalRes = UHMM3Phmmer::phmmer(querySeq.seq.constData(), querySeq.length(), 
                                                       seq, seqLen, settings, stateInfo, wholeSeqSz);
    if( ti.hasError() ) {
        UHMM3SearchTaskLocalStorage::freeTaskContext( t->getTaskId() );
        return;
    }
    
    QMutexLocker locker(&writeResultsMtx);
    UHMM3SWSearchTask::writeResults(generalRes.domainResList, t, results, overlaps, querySeq.length());
    UHMM3SearchTaskLocalStorage::freeTaskContext( t->getTaskId() );
}

Task::ReportResult UHMM3SWPhmmerTask::report() {
    if(hasError()) {
        return ReportResult_Finished;
    }
    UHMM3SWSearchTask::processOverlaps(overlaps, results, querySeq.length() / 2);
    qSort(results.begin(), results.end(), UHMM3SWSearchTask::uhmm3SearchDomainResultLessThan);
    return ReportResult_Finished;
}

QList<TaskResourceUsage> UHMM3SWPhmmerTask::getResources(SequenceWalkerSubtask * t) {
    assert(t != NULL);
    QList<TaskResourceUsage> res;
    int howManyMem = countPhmmerMemInMB( dbSeq.length(), querySeq.length() );
    res << TaskResourceUsage( RESOURCE_MEMORY, howManyMem );
    algoLog.trace( QString( "%1 requires %2 of memory" ).arg( getTaskName() ).arg( howManyMem ) );
    return res;
}

QList< SharedAnnotationData >
UHMM3SWPhmmerTask::getResultsAsAnnotations( const QString & name ) const {
    assert( !name.isEmpty() );
    QList< SharedAnnotationData > annotations;
    foreach( const UHMM3SWSearchTaskDomainResult & res, results ) {
        AnnotationData * annData = new AnnotationData();
        annData->name = name;
        annData->setStrand(res.onCompl ? U2Strand::Complementary : U2Strand::Direct);
        annData->location->regions << res.generalResult.seqRegion;
        annData->qualifiers << U2Qualifier( "Query sequence", querySeq.getName() );
        res.generalResult.writeQualifiersToAnnotation( annData );
        annotations << SharedAnnotationData( annData );
    }
    return annotations;
}

QList<UHMM3SWSearchTaskDomainResult> UHMM3SWPhmmerTask::getResult() const {
    return results;
}

/*******************************************
 * HMMER3 phmmer search to annotations task.
 ********************************************/
void UHMM3PhmmerToAnnotationsTask::checkArgs() {
    if( queryfile.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( tr( "querySeq sequence file path" ) ) );
        return;
    }
    if( dbSeq.isNull() ) {
        stateInfo.setError( L10N::badArgument( tr( "db sequence" ) ) );
        return;
    }
    if( NULL == annotationObj.data() ) {
        stateInfo.setError( L10N::badArgument( tr( "annotation object" ) ) );
        return;
    }
    if( annName.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( tr( "annotation name" ) ) );
        return;
    }
    if( annGroup.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( tr( "annotation group" ) ) );
        return;
    }
}

UHMM3PhmmerToAnnotationsTask::UHMM3PhmmerToAnnotationsTask( const QString & qfile, const DNASequence & db,
                                                            AnnotationTableObject * o, const QString & gr, 
                                                            const QString & name, const UHMM3PhmmerSettings & set )
: Task( "HMM Phmmer task", TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled ), 
queryfile( qfile ), dbSeq( db ), annotationObj( o ), annGroup( gr ), annName( name ), settings( set ),
phmmerTask( NULL ), createAnnotationsTask( NULL ) {
    
    checkArgs();
    if( hasError() ) {
        return;
    }
    setTaskName( tr( "HMM Phmmer search %1 sequence with %2 database" ).arg( queryfile ).arg( dbSeq.getName() ) );
    phmmerTask = new UHMM3SWPhmmerTask(queryfile, dbSeq, settings);
    addSubTask( phmmerTask );
}

QList< Task* > UHMM3PhmmerToAnnotationsTask::onSubTaskFinished( Task * subTask ) {
    assert( NULL != subTask );
    QList< Task* > res;
    if( hasError() ) {
        return res;
    }
    if( subTask->hasError() ) {
        stateInfo.setError( subTask->getError() );
        return res;
    }
    
    if( annotationObj.isNull() ) {
        stateInfo.setError( tr( "Annotation object removed" ) );
        return res;
    }
    
    if( phmmerTask == subTask ) {
        QList< SharedAnnotationData > annotations = phmmerTask->getResultsAsAnnotations( annName );
        if( annotations.isEmpty() ) {
            return res;
        }
        createAnnotationsTask = new CreateAnnotationsTask( annotationObj, annGroup, annotations );
        res << createAnnotationsTask;
    } else if( createAnnotationsTask != subTask ) {
        assert( false && "undefined_task_finished!" );
    }
    
    return res;
}

QString UHMM3PhmmerToAnnotationsTask::generateReport() const {
    QString res;
    res += "<table>";
    res+="<tr><td width=200><b>" + tr("Query sequence") + "</b></td><td>" + QFileInfo( queryfile ).absoluteFilePath() + "</td></tr>";
    
    if( hasError() || isCanceled() ) {
        res += "<tr><td width=200><b>" + tr("Task was not finished") + "</b></td><td></td></tr>";
        res += "</table>";
        return res;
    }
    
    res += "<tr><td><b>" + tr("Result annotation table") + "</b></td><td>" + annotationObj->getDocument()->getName() + "</td></tr>";
    res += "<tr><td><b>" + tr("Result annotation group") + "</b></td><td>" + annGroup + "</td></tr>";
    res += "<tr><td><b>" + tr("Result annotation name") +  "</b></td><td>" + annName + "</td></tr>";
    
    int nResults = createAnnotationsTask == NULL ? 0 : createAnnotationsTask->getAnnotations().size();
    res += "<tr><td><b>" + tr("Results count") +  "</b></td><td>" + QString::number( nResults )+ "</td></tr>";
    res += "</table>";
    return res;
}

} // U2
