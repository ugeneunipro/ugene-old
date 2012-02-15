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

#include <QtCore/QByteArray>

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/Counter.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/DNASequenceObject.h>

#include <gobject/uHMMObject.h>
#include <format/uHMMFormat.h>
#include <task_local_storage/uHMMSearchTaskLocalStorage.h>

#include <util/uhmm3Utilities.h>
#include "uHMM3SearchTask.h"

#define UHMM3_SEARCH_LOG_CAT "hmm3_search_log_category"

using namespace U2;

namespace U2 {

static int countSearchMemInMB( qint64 seqLen, int hmmLen ) {
    return ( ( 77 * seqLen + 10500 * hmmLen ) / ( 1024 * 1024 ) ) + 2;
}

/*****************************************************
* UHMM3SWSearchTask
*****************************************************/

static void recountRegion( U2Region& region, bool isAmino, bool isCompl, U2Region globalR ) {
    int len = isAmino? region.length * 3 : region.length;
    int start = isAmino? region.startPos * 3 : region.startPos;

    if( isCompl ) {
        start = globalR.length - start - len;
    }
    region.startPos = globalR.startPos + start;
    region.length = len;
}

static void recountUHMM3SWSearchRegions( UHMM3SearchSeqDomainResult& res, bool isAmino, bool isCompl, U2Region globalR ) {
    recountRegion( res.seqRegion, isAmino, isCompl, globalR );
    recountRegion( res.envRegion, isAmino, isCompl, globalR );
}

UHMM3SWSearchTask::UHMM3SWSearchTask( const P7_HMM* h, const DNASequence& s, const UHMM3SearchTaskSettings& set, int ch )
: Task( "", TaskFlag_NoRun ), hmm( h ), sequence( s ), settings( set ), 
  complTranslation( NULL ), aminoTranslation( NULL ), swTask( NULL ), loadHmmTask( NULL ), searchChunkSize(ch) {
    GCOUNTER( cvar, tvar, "UHMM3SWSearchTask" );
    
    assert(searchChunkSize > 0);
    if( NULL == hmm ) {
        setTaskName( tr( "Sequence_walker_HMM_search_task" ) );
        stateInfo.setError( L10N::badArgument( "hmm" ) );
        return;
    }
    assert( NULL != hmm->name );
    setTaskName( tr( "Sequence_walker_hmm_search_with_'%1'" ).arg( hmm->name ) );
    
    if( !sequence.seq.length() ) {
        stateInfo.setError( L10N::badArgument( "sequence" ) );
        return;
    }
}

UHMM3SWSearchTask::UHMM3SWSearchTask( const QString& hF, const DNASequence& seq, const UHMM3SearchTaskSettings& s, int ch)
: Task( "", TaskFlag_NoRun ), hmm( NULL ), sequence( seq ), settings( s ), 
  complTranslation( NULL ), aminoTranslation( NULL ), swTask( NULL ), loadHmmTask( NULL ), hmmFilename( hF ), searchChunkSize(ch) {
    
    assert(searchChunkSize > 0);
    if( hmmFilename.isEmpty() ) {
        setTaskName( tr( "Sequence_walker_HMM_search_task" ) );
        stateInfo.setError( L10N::badArgument( "hmm_filename" ) );
        return;
    }
    setTaskName( tr( "Sequence_walker_hmm_search_with_'%1'" ).arg( hmmFilename ) );
    
    if( !sequence.seq.length() ) {
        stateInfo.setError( L10N::badArgument( "sequence" ) );
        return;
    }
}

SequenceWalkerTask* UHMM3SWSearchTask::getSWSubtask() {
    assert( !hasError() );
    assert( NULL != hmm );
    
    bool ok = checkAlphabets( hmm->abc->type, sequence.alphabet );
    if( !ok ) {
        assert( hasError() );
        return NULL;
    }
    ok = setTranslations( hmm->abc->type, sequence.alphabet );
    if( !ok ) {
        assert( hasError() );
        return NULL;
    }
    
    SequenceWalkerConfig config;
    config.seq                  = sequence.seq.data();
    config.seqSize              = sequence.seq.size();
    config.complTrans           = complTranslation;
    config.strandToWalk         = complTranslation == NULL ? StrandOption_DirectOnly : StrandOption_Both;
    config.aminoTrans           = aminoTranslation;
    /*config.overlapSize          = 2 * hmm->M;
    config.chunkSize            = qMax(searchChunkSize, 6 * hmm->M);*/
    config.overlapSize          = 0;
    config.chunkSize            = config.seqSize;
    config.lastChunkExtraLen    = config.chunkSize / 2;
    config.nThreads             = MAX_PARALLEL_SUBTASKS_AUTO;
    
    return new SequenceWalkerTask( config, this, tr( "sequence_walker_hmmer3_search_task" ) );
}

void UHMM3SWSearchTask::prepare() {
    if( hasError() ) {
        return;
    }
    
    if( NULL != hmm ) {
        swTask = getSWSubtask();
        if( NULL == swTask ) {
            assert( hasError() );
            return;
        }
        addSubTask( swTask );
    } else {
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( IOAdapterUtils::url2io( hmmFilename ) );
        assert( NULL != iof );
        loadHmmTask = new LoadDocumentTask( UHMMFormat::UHHMER_FORMAT_ID, hmmFilename, iof, QVariantMap() );
        addSubTask( loadHmmTask );
    }
}

QList< Task* > UHMM3SWSearchTask::onSubTaskFinished( Task* subTask ) {
    assert( NULL != subTask );
    QList< Task* > res;
    if( subTask->hasError() ) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    
    if( loadHmmTask == subTask ) {
        hmm = UHMM3Utilities::getHmmFromDocument( loadHmmTask->getDocument(), stateInfo );
        swTask = getSWSubtask();
        if( NULL == swTask ) {
            assert( hasError() );
            return res;
        }
        res << swTask;
    } else {
        if( swTask != subTask ) {
            assert( 0 && "undefined_subtask_finished" );
        }
    }
    
    return res;
}

void UHMM3SWSearchTask::onRegion( SequenceWalkerSubtask* t, TaskStateInfo& ti ) {
    assert( NULL != t );
    if( stateInfo.hasError() || ti.hasError() ) {
        return;
    }
    
    const char* seq = t->getRegionSequence();
    int seqLen      = t->getRegionSequenceLen();
    
    UHMM3SearchTaskLocalStorage::createTaskContext( t->getTaskId() );
    int wholeSeqSz = t->getGlobalConfig().seqSize;
    wholeSeqSz = t->isAminoTranslated() ? (wholeSeqSz / 3) : wholeSeqSz;
    UHMM3SearchResult generalRes = UHMM3Search::search( hmm, seq, seqLen, settings.inner, ti, wholeSeqSz );
    if( ti.hasError() ) {
        UHMM3SearchTaskLocalStorage::freeTaskContext( t->getTaskId() );
        return;
    }
    
    QMutexLocker locker( &writeResultsMtx );
    writeResults(generalRes.domainResList, t, results, overlaps, hmm->M);
    UHMM3SearchTaskLocalStorage::freeTaskContext( t->getTaskId() );
}

void UHMM3SWSearchTask::writeResults(const QList<UHMM3SearchSeqDomainResult> & domains, SequenceWalkerSubtask * t, 
        QList<UHMM3SWSearchTaskDomainResult> & results, QList<UHMM3SWSearchTaskDomainResult> & overlaps, int halfOverlap) {
    bool isCompl     = t->isDNAComplemented();
    bool isAmino    = t->isAminoTranslated();
    U2Region globalRegion = t->getGlobalRegion();
    foreach( const UHMM3SearchSeqDomainResult& domainRes, domains ) {
        UHMM3SWSearchTaskDomainResult res;
        res.generalResult = domainRes;
        res.onCompl = isCompl;
        res.onAmino = isAmino;
        recountUHMM3SWSearchRegions( res.generalResult, isAmino, isCompl, globalRegion );
        if(t->intersectsWithOverlaps(res.generalResult.seqRegion)) {
            bool add = true;
            if(!res.onCompl && t->hasRightOverlap()) { // if it will be found in a next chunk
                    U2Region nextChunkRegion(globalRegion.endPos() - halfOverlap, halfOverlap);
                    add = !nextChunkRegion.contains(res.generalResult.seqRegion);
            } else if(res.onCompl && t->hasLeftOverlap()) { // if it will be found on prev chunk
                U2Region prevChunkRegion(globalRegion.startPos, halfOverlap);
                add = !prevChunkRegion.contains(res.generalResult.seqRegion);
            }
            if(add) {
                res.borderResult = (t->hasLeftOverlap() && res.generalResult.seqRegion.startPos == globalRegion.startPos) ||
                    (t->hasRightOverlap() && res.generalResult.seqRegion.endPos() == globalRegion.endPos());
                overlaps.append(res);
            }
        } else { // no intersections
         results.append( res );
        }
    }
}

QList< TaskResourceUsage > UHMM3SWSearchTask::getResources( SequenceWalkerSubtask * t ) {
    assert( NULL != t );
    assert( !sequence.isNull() && NULL != hmm );
    
    QList< TaskResourceUsage > res;
    int howManyMem = countSearchMemInMB( t->getRegionSequenceLen(), hmm->M );
    res << TaskResourceUsage( RESOURCE_MEMORY, howManyMem );
    algoLog.trace( QString( "%1 requires %2 of memory" ).arg( getTaskName() ).arg( howManyMem ) );
    return res;
}

/* Same as in HMMSearchTask */
void
UHMM3SWSearchTask::processOverlaps(QList<UHMM3SWSearchTaskDomainResult> & overlaps, 
                                   QList<UHMM3SWSearchTaskDomainResult> & results, int maxCommonLen) {
    for(int i = 0; i < overlaps.count(); ++i){
        UHMM3SWSearchTaskDomainResult & r1 = overlaps[i];
        if (r1.filtered) {
            continue;
        }
        for(int j = i + 1; j < overlaps.count(); ++j){
            UHMM3SWSearchTaskDomainResult & r2 = overlaps[j];
            if (r2.filtered) {
                continue;
            }
            if (r1.onCompl != r2.onCompl) { //check both regions are on the same strand
                continue;
            }
            if (r1.onAmino) { //check both regions have the same amino frame
                int s1 = r1.onCompl ? r1.generalResult.seqRegion.endPos() % 3 : r1.generalResult.seqRegion.startPos % 3;
                int s2 = r2.onCompl ? r2.generalResult.seqRegion.endPos() % 3 : r2.generalResult.seqRegion.startPos % 3;
                if (s1 != s2) {
                    continue;
                }
            }
            if (r1.generalResult.seqRegion.contains(r2.generalResult.seqRegion) && 
                r1.generalResult.seqRegion != r2.generalResult.seqRegion) {
                    r2.filtered = true;
            } else if (r2.generalResult.seqRegion.contains(r1.generalResult.seqRegion) && 
                r2.generalResult.seqRegion != r1.generalResult.seqRegion) {
                    r1.filtered = true;
                    break;
            } else if (r1.generalResult.seqRegion.intersect(r2.generalResult.seqRegion).length >= maxCommonLen) {
                bool useR1 = r2.generalResult.score <= r1.generalResult.score;
                if (r1.generalResult.score == r2.generalResult.score && r1.generalResult.ival == r2.generalResult.ival
                    && r1.borderResult && !r2.borderResult) {
                        useR1 = false;
                }
                if (useR1) {
                    r2.filtered = true;
                } else {
                    r1.filtered = true;
                    break;
                }
            }
        }
    }
    
    foreach(const UHMM3SWSearchTaskDomainResult& r, overlaps) {
        if (!r.filtered) {
            results.append(r);
        }
    }
}

bool
UHMM3SWSearchTask::uhmm3SearchDomainResultLessThan(const UHMM3SWSearchTaskDomainResult & r1, const UHMM3SWSearchTaskDomainResult & r2) {
    if( r1.generalResult.score == r2.generalResult.score ) {
        if(r1.generalResult.seqRegion == r2.generalResult.seqRegion) {
            if( r1.onCompl == r2.onCompl ) {
                return &r1 < &r2;
            }
            return r2.onCompl;
        }
        return r1.generalResult.seqRegion < r2.generalResult.seqRegion;
    }
    return r1.generalResult.score > r2.generalResult.score;
}

Task::ReportResult UHMM3SWSearchTask::report() {
    if(hasError()) {
        return ReportResult_Finished;
    }
    processOverlaps(overlaps, results, hmm->M / 2);
    qSort(results.begin(), results.end(), uhmm3SearchDomainResultLessThan);
    return ReportResult_Finished;
}

bool UHMM3SWSearchTask::checkAlphabets( int hmmAl, DNAAlphabet* seqAl ) {
    assert( !hasError() );
    assert( NULL != seqAl );
    assert( 0 <= hmmAl );
    
    if( eslUNKNOWN == hmmAl || eslNONSTANDARD == hmmAl ) {
        stateInfo.setError( tr( "unknown_alphabet_type" ) );
        return false;
    }
    if( seqAl->isRaw() ) {
        stateInfo.setError( tr( "invalid_sequence_alphabet_type" ) );
        return false;
    }
    
    if( eslDNA == hmmAl || eslRNA == hmmAl ) {
        if( seqAl->isAmino() ) {
            stateInfo.setError( tr( "cannot_search_for_nucleic_hmm_in_amino_sequence" ) );
            return false;
        }
    }
    return true;
}

bool UHMM3SWSearchTask::setTranslations( int hmmAl, DNAAlphabet* seqAl ) {
    assert( !hasError() );
    assert( NULL != seqAl );
    assert( 0 <= hmmAl );
    
    if( seqAl->isNucleic() ) {
        DNATranslationRegistry* transReg = AppContext::getDNATranslationRegistry();
        assert( NULL != transReg );
        QList< DNATranslation* > complTs = transReg->lookupTranslation( seqAl, DNATranslationType_NUCL_2_COMPLNUCL );
        if (!complTs.empty()) {
            complTranslation = complTs.first();
        }
        if( hmmAl == eslAMINO ) {
            QList< DNATranslation* > aminoTs = transReg->lookupTranslation( seqAl, DNATranslationType_NUCL_2_AMINO );
            if( !aminoTs.empty() ) {
                aminoTranslation = aminoTs.first();
            }
        }
    } else {
        if( !seqAl->isAmino() ) {
            stateInfo.setError( "unrecognized_sequence_alphabet_found" );
            return false;
        }
    }
    
    return true;
}

QList<UHMM3SWSearchTaskDomainResult> UHMM3SWSearchTask::getResults() const {
    return results;
}

QList< SharedAnnotationData >
UHMM3SWSearchTask::getResultsAsAnnotations( const QList<UHMM3SWSearchTaskDomainResult> & results, const P7_HMM * hmm, const QString & name ) {
    assert( !name.isEmpty() );
    QList< SharedAnnotationData > annotations;
    
    foreach( const UHMM3SWSearchTaskDomainResult & res, results ) {
        AnnotationData * annData = new AnnotationData();
        annData->name = name;
        annData->setStrand(res.onCompl ? U2Strand::Complementary : U2Strand::Direct);
        annData->location->regions << res.generalResult.seqRegion;
        
        assert( NULL != hmm );
        QString hmmInfo = hmm->name;
        if( NULL != hmm->acc ) {
            hmmInfo += QString().sprintf( "\n Accession number in PFAM database: %s", hmm->acc );
        }
        if( NULL != hmm->desc ) {
            hmmInfo += QString().sprintf( "\n Description: %s", hmm->desc );
        }
        assert( !hmmInfo.isEmpty() );
        annData->qualifiers << U2Qualifier( "HMM model", hmmInfo );
        res.generalResult.writeQualifiersToAnnotation( annData );
        
        annotations << SharedAnnotationData( annData );
    }
    
    return annotations;
}

QList< SharedAnnotationData > UHMM3SWSearchTask::getResultsAsAnnotations( const QString & aname ) {
    return getResultsAsAnnotations( results, hmm, aname );
}

/*****************************************************
 * UHMM3SearchTaskSettings
 *****************************************************/

UHMM3SearchTaskSettings::UHMM3SearchTaskSettings() {
    setDefaultUHMM3SearchSettings( &inner );
}

/*****************************************************
* UHMM3SearchTask
*****************************************************/

UHMM3SearchTask::UHMM3SearchTask(const UHMM3SearchTaskSettings &_settings, P7_HMM *_hmmProfile, const QByteArray &_sequence)
        : Task(tr("HMM search task"), TaskFlag_None),
          settings(_settings), hmmProfile(_hmmProfile), sequence(_sequence)
{
    assert(hmmProfile && "Bad HMM profile given");
    assert(hmmProfile->name);

    setTaskName(tr("HMM search with '%1'").arg(hmmProfile->name));
}

void UHMM3SearchTask::prepare() {
    assert(hmmProfile->M > 0);

    int howManyMem = countSearchMemInMB(sequence.length(), hmmProfile->M);
    addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, howManyMem));

    algoLog.trace(QString("%1 needs %2 of memory").arg(getTaskName()).arg(howManyMem));
}

void UHMM3SearchTask::run() {   
    UHMM3SearchTaskLocalStorage::createTaskContext( getTaskId() );
    result = UHMM3Search::search(hmmProfile, sequence.data(), sequence.length(), settings.inner, stateInfo, sequence.length());
    UHMM3SearchTaskLocalStorage::freeTaskContext( getTaskId() );
}

/*****************************************************
* UHMM3LoadProfileAndSearchTask
*****************************************************/

UHMM3LoadProfileAndSearchTask::UHMM3LoadProfileAndSearchTask(const UHMM3SearchTaskSettings &_settings, const QString &_hmmProfileFile, const QByteArray &_sequence)
        : Task(tr("HMM search with '%1' HMM profile file").arg(_hmmProfileFile), TaskFlags_NR_FOSCOE),
          loadHmmProfileTask(0), hmmSearchTask(0),
          hmmProfile(0), settings(_settings), sequence(_sequence)
{
    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(_hmmProfileFile));
    assert(iof);

    loadHmmProfileTask = new LoadDocumentTask(UHMMFormat::UHHMER_FORMAT_ID, _hmmProfileFile, iof);
    addSubTask(loadHmmProfileTask);
}

QList<Task*> UHMM3LoadProfileAndSearchTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> subTasks;

    propagateSubtaskError();
    if(subTask->hasError() || subTask->isCanceled()) {
        return subTasks;
    }

    if (loadHmmProfileTask == subTask) {
        hmmProfile = UHMM3Utilities::getHmmFromDocument(loadHmmProfileTask->getDocument(), stateInfo);
        assert(hmmProfile && "Bad HMM profile");

        hmmSearchTask = new UHMM3SearchTask(settings, hmmProfile, sequence);
        subTasks << hmmSearchTask;
    }
    else if (hmmSearchTask == subTask) {
        // pass
    }
    else {
        assert(!"Undefined task");
    }

    return subTasks;
}


/*****************************************************
* UHMM3SWSearchToAnnotationsTask
*****************************************************/

void UHMM3SWSearchToAnnotationsTask::checkArgs() {
    if( hmmfile.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( tr("hmm profile filename") ) );
        return;
    }
    if( NULL == annotationObj.data() ) {
        stateInfo.setError( L10N::badArgument( tr("annotation object") ) );
        return;
    }
    if( agroup.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( tr( "annotations group name" ) ) );
        return;
    }
    if( aname.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( tr( "annotations name" ) ) );
        return;
    }
}

UHMM3SWSearchToAnnotationsTask::UHMM3SWSearchToAnnotationsTask( const QString & hmmf, const DNASequence & s,
                                                                AnnotationTableObject * o, const QString & gr,
                                                                const QString & name, const UHMM3SearchTaskSettings & set )
: Task( "", TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled ),
hmmfile( hmmf ), sequence( s ), annotationObj( o ), agroup( gr ), aname( name ), searchSettings( set ),
loadSequenceTask( NULL ), searchTask( NULL ), createAnnotationsTask( NULL ) {
    
    setTaskName( tr( "HMMER3 search task" ) );
    checkArgs();
    if( sequence.isNull() ) {
        stateInfo.setError( L10N::badArgument( tr("dna sequence" ) ) );
    }
    if( stateInfo.hasError() ) {
        return;
    }
    setTaskName( tr( "HMMER3 search task with '%1' profile" ).arg( hmmfile ) );
    
    searchTask = new UHMM3SWSearchTask( hmmfile, sequence, searchSettings );
    addSubTask( searchTask );
}

UHMM3SWSearchToAnnotationsTask::UHMM3SWSearchToAnnotationsTask( const QString & hmmf, const QString & seqFile,
                                                                AnnotationTableObject * obj, const QString & gr,
                                                                const QString & name,
                                                                const UHMM3SearchTaskSettings & set )
: Task( "", TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled ), 
hmmfile( hmmf ), annotationObj( obj ), agroup( gr ), aname( name ), searchSettings( set ),
loadSequenceTask( NULL ), searchTask( NULL ), createAnnotationsTask( NULL ) {
    
    setTaskName( tr( "HMMER3 search task" ) );
    checkArgs();
    if( seqFile.isEmpty() ) {
        stateInfo.setError( L10N::badArgument( tr( "Sequence file" ) ) );
    }
    if( stateInfo.hasError() ) {
        return;
    }
    setTaskName( tr( "HMMER3 search task with '%1' profile" ).arg( hmmfile ) );
    
    loadSequenceTask = LoadDocumentTask::getDefaultLoadDocTask( seqFile );
    if( NULL == loadSequenceTask ) {
        stateInfo.setError( L10N::errorOpeningFileRead( seqFile ) );
        return;
    } else {
        addSubTask( loadSequenceTask );
    }
}

QString UHMM3SWSearchToAnnotationsTask::generateReport() const {
    QString res;
    res += "<table>";
    res+="<tr><td width=200><b>" + tr("HMM profile used") + "</b></td><td>" + QFileInfo( hmmfile ).absoluteFilePath() + "</td></tr>";
    
    if( hasError() || isCanceled() ) {
        res += "<tr><td width=200><b>" + tr("Task was not finished") + "</b></td><td></td></tr>";
        res += "</table>";
        return res;
    }
    
    res += "<tr><td><b>" + tr("Result annotation table") + "</b></td><td>" + annotationObj->getDocument()->getName() + "</td></tr>";
    res += "<tr><td><b>" + tr("Result annotation group") + "</b></td><td>" + agroup + "</td></tr>";
    res += "<tr><td><b>" + tr("Result annotation name") +  "</b></td><td>" + aname + "</td></tr>";
    
    int nResults = createAnnotationsTask == NULL ? 0 : createAnnotationsTask->getAnnotations().size();
    res += "<tr><td><b>" + tr("Results count") +  "</b></td><td>" + QString::number( nResults )+ "</td></tr>";
    res += "</table>";
    return res;
}

void UHMM3SWSearchToAnnotationsTask::setSequence() {
    assert( NULL != loadSequenceTask );
    
    Document * seqDoc = loadSequenceTask->getDocument();
    if( NULL == seqDoc ) {
        stateInfo.setError( tr( "Cannot load sequence document" ) );
        return;
    }
    QList< GObject* > objs = seqDoc->findGObjectByType( GObjectTypes::SEQUENCE );
    if( objs.isEmpty() ) {
        stateInfo.setError( tr( "No sequence objects loaded" ) );
        return;
    }
    U2SequenceObject * seqObj = qobject_cast< U2SequenceObject* >( objs.first() );
    CHECK_EXT(seqObj != NULL, setError( tr( "Unknown sequence type loaded" ) ), )
    
    sequence = seqObj->getWholeSequence();
    CHECK_EXT(sequence.length() > 0, setError(tr( "Empty sequence loaded")),  );
}

QList< Task* > UHMM3SWSearchToAnnotationsTask::onSubTaskFinished( Task * subTask ) {
    QMutexLocker locker( &mtx );
    QList< Task* > res;
    if( hasError() ) {
        return res;
    }
    assert( NULL != subTask );
    if( subTask->hasError() ) {
        stateInfo.setError( subTask->getError() );
        return res;
    }
    
    if( annotationObj.isNull() ) {
        stateInfo.setError( tr( "Annotation object removed" ) );
        return res;
    }
    
    if( loadSequenceTask == subTask ) {
        setSequence();
        if( hasError() ) {
            return res;
        }
        searchTask = new UHMM3SWSearchTask( hmmfile, sequence, searchSettings );
        res << searchTask;
    } else if( searchTask == subTask ) {
        QList< SharedAnnotationData > annotations = searchTask->getResultsAsAnnotations( aname );
        if( annotations.isEmpty() ) {
            return res;
        }
        
        createAnnotationsTask = new CreateAnnotationsTask( annotationObj, agroup, annotations );
        res << createAnnotationsTask;
    } else if( createAnnotationsTask != subTask ) {
        assert( false && "undefined task finished" );
    }
    
    return res;
}

} // U2
