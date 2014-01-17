/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "LoadDASDocumentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/PicrApiTask.h>

#include <QtXml/QDomDocument>

namespace U2 {

LoadDASDocumentTask::LoadDASDocumentTask( const QString& accId, const QString& _fullPath, const DASSource& _referenceSource, const QList<DASSource>& _featureSources )
: BaseLoadRemoteDocumentTask(_fullPath, TaskFlags(TaskFlag_FailOnSubtaskCancel | TaskFlag_MinimizeSubtaskErrorText | TaskFlag_NoRun))
,accNumber(accId)
,featureSources(_featureSources)
,referenceSource(_referenceSource)
,loadSequenceTask(NULL)
,saveDocumentTask(NULL)
,seq(NULL)
{

}

void LoadDASDocumentTask::prepare(){
    BaseLoadRemoteDocumentTask::prepare();
    if (!isCached()){
        //load sequence
        loadSequenceTask = new LoadDASObjectTask(accNumber, referenceSource, DASSequence);
        addSubTask(loadSequenceTask);

        //load annotations
        foreach(const DASSource& s, featureSources){
            LoadDASObjectTask* featureTask = new LoadDASObjectTask(accNumber, s, DASFeatures);
            loadFeaturesTasks.append(featureTask);
            addSubTask(featureTask);
        }
    }
}

QString LoadDASDocumentTask::getFileFormat( const QString & dbid ){
    Q_UNUSED(dbid);
    return GENBANK_FORMAT;
}

GUrl LoadDASDocumentTask::getSourceURL(){
    return GUrl();
}

QString LoadDASDocumentTask::getFileName(){
    format = getFileFormat("");
    accNumber.replace(";",",");
    QStringList accIds = accNumber.split(",");
    if (accIds.size() == 1 ) {
        return accNumber + "_das" +"." + format;
    } else if (accIds.size() > 1) {
        return accIds.first() + "_das_misc." + format;
    }

    return "";
}

QList<Task*> LoadDASDocumentTask::onSubTaskFinished( Task *subTask ){
    QList<Task *> subTasks;

    if ( subTask == loadDocumentTask ) {
        if ( subTask->hasError( ) ) {
            setError( tr( "Cannot load cached document: %1" ).arg( accNumber ) );
            return subTasks;
        }
        resultDocument = loadDocumentTask->takeDocument( );
    } else if ( subTask == saveDocumentTask ) {
        if ( saveDocumentTask->hasError( ) ) {
            setError( tr( "Cannot save document: %1" ).arg( accNumber ) );
            return subTasks;
        }
        if ( !subTask->isCanceled( ) ) {
            RecentlyDownloadedCache * cache = AppContext::getRecentlyDownloadedCache( );
            if ( NULL != cache ) {
                cache->append( fullPath );
            }
        }
    } else {
        if ( subTask == loadSequenceTask ) {
            if ( loadSequenceTask->hasError( ) ) {
                setError( tr( "Cannot find DAS reference sequence: %1" ).arg( accNumber ) );
                return subTasks;
            }
            if ( !isCanceled( ) ) {
                seq = loadSequenceTask->getSequence( );
            }

            loadSequenceTask = NULL;
        } else {
            LoadDASObjectTask *ftask = qobject_cast<LoadDASObjectTask *>( subTask );
            if ( NULL != ftask && loadFeaturesTasks.contains( ftask ) ) {
                const int idx = loadFeaturesTasks.indexOf( ftask );
                if ( idx == -1 ) {
                    return subTasks;
                }

                loadFeaturesTasks.removeAt( idx );

                if ( ftask->hasError( ) ) {
                    ioLog.error( tr( "Cannot find DAS features for: %1" ).arg( accNumber ) );
                } else {
                    //merge features
                    if ( !isCanceled( ) ) {
                        mergeFeatures( ftask->getAnnotationData( ) );
                    }
                }
            }
        }

        if ( isAllDataLoaded( ) ) {
            AnnotationTableObject *annotationTableObject = NULL;
            if ( !annotationData.isEmpty( ) ) {
                U2DbiRegistry *dbiReg = AppContext::getDbiRegistry();
                SAFE_POINT_EXT(NULL != dbiReg, setError("NULL DBI registry"), subTasks);

                U2DbiRef dbiRef = dbiReg->getSessionTmpDbiRef(stateInfo);
                CHECK_OP(stateInfo, subTasks);

                annotationTableObject = new AnnotationTableObject( "das_annotations", dbiRef );
                
                foreach ( const QString &grname, annotationData.keys( ) ) {
                    const QList<AnnotationData> sdata = annotationData[grname];
                    if ( !sdata.isEmpty( ) ) {
                        foreach ( AnnotationData d, sdata ) {
                            //setRegion
                            if ( NULL != seq ) {
                                if ( d.location->isSingleRegion( )
                                    && d.location->regions.first() == U2_REGION_MAX)
                                {
                                    U2Location newLoc = d.location;
                                    newLoc->regions.clear( );
                                    newLoc->regions.append( U2Region( 0, seq->length( ) ) );
                                    d.location = newLoc;
                                }
                            }
                            annotationTableObject->addAnnotation( d, grname );
                        }
                    }
                }
            }
            if ( NULL != seq ) {
                createLoadedDocument( );
                if ( NULL == resultDocument ) {
                    return subTasks;
                }
                
                U2EntityRef seqRef = U2SequenceUtils::import( resultDocument->getDbiRef( ), *seq, stateInfo );
                if ( stateInfo.isCoR( ) ) {
                    return subTasks;
                } 
                U2SequenceObject *danseqob = new U2SequenceObject( seq->getName( ), seqRef );
                resultDocument->addObject( danseqob );

                if ( NULL != annotationTableObject ) {
                    annotationTableObject->addObjectRelation( GObjectRelation( danseqob,
                        GObjectRelationRole::SEQUENCE ) );
                    resultDocument->addObject( annotationTableObject );
                    
                }

                saveDocumentTask = new SaveDocumentTask( resultDocument );
                subTasks.append( saveDocumentTask );
            }
         }
    }
    return subTasks;

}

bool LoadDASDocumentTask::isAllDataLoaded( ) {
    return ( NULL == loadSequenceTask && loadFeaturesTasks.isEmpty( ) );
}

void LoadDASDocumentTask::mergeFeatures( const QMap<QString, QList<AnnotationData> > &newAnnotations ) {
    const QStringList &keys =  newAnnotations.keys( );
    foreach ( const QString &key, keys ) {
        if ( annotationData.contains(key ) ) {
            const QList<AnnotationData> &curList = annotationData[key];
            const QList<AnnotationData> &tomergeList = newAnnotations[key];
            foreach ( const AnnotationData &d, tomergeList ) {
                if ( !curList.contains(d ) ) {
                    annotationData[key].append( d );
                }
            }
        } else {
            annotationData.insert( key, newAnnotations[key] );
        }
    }

}



//////////////////////////////////////////////////////////////////////////
//LoadDASObjectTask
LoadDASObjectTask::LoadDASObjectTask( const QString& accId, const DASSource& _source, DASObjectType objType)
:Task(tr("Load DAS data for: %1").arg(accId), TaskFlags_FOSCOE | TaskFlag_MinimizeSubtaskErrorText)
,accNumber(accId)
,source(_source)
,objectType(objType)
,loop(NULL)
,downloadReply(NULL)
,networkManager(NULL)
,seq(NULL)
{
    
}

LoadDASObjectTask::~LoadDASObjectTask(){
    delete loop;
    delete networkManager;
}

void LoadDASObjectTask::run(){
    if (stateInfo.isCanceled()){
        return;
    }
    stateInfo.progress = 0;
    ioLog.trace("Start loading data from DAS...");

    loop = new QEventLoop;

    networkManager = new QNetworkAccessManager();
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(sl_replyFinished(QNetworkReply*)));
    NetworkConfiguration* nc = AppContext::getAppSettings()->getNetworkConfiguration();
    
    ioLog.trace("Downloading xml file...");

    QString fetchUrl = DASSourceRegistry::getRequestURLString(source, accNumber, objectType);
    QNetworkProxy proxy = nc->getProxyByUrl(fetchUrl);
    networkManager->setProxy(proxy);
    ioLog.trace(fetchUrl);

    QUrl requestUrl(fetchUrl);
    downloadReply = networkManager->get(QNetworkRequest(requestUrl));
    connect(downloadReply, SIGNAL(error(QNetworkReply::NetworkError)),
        this, SLOT(sl_onError(QNetworkReply::NetworkError)));
    connect( downloadReply, SIGNAL(uploadProgress( qint64, qint64 )),
        this, SLOT(sl_uploadProgress(qint64,qint64)) );

    loop->exec();
    ioLog.trace("Download finished.");
    
    QByteArray result = downloadReply->readAll();
    if ( ( result.size() < 100 ) && result.contains("Nothing has been found")) {
        setError(tr("Sequence with ID=%1 is not found.").arg(accNumber));
        return;
    }

    //parse output
    if (objectType == DASSequence){
        XMLDASSequenceParser parser;
        parser.parse(result);
        if (!parser.getError().isEmpty()){
            setError(parser.getError());
        }else{
            seq = parser.getSequence();
        }
    }else if(objectType == DASFeatures){
        XMLDASFeaturesParser parser;
        parser.parse(result);
        if (!parser.getError().isEmpty()){
            setError(parser.getError());
        }else{
            annotationData = parser.getAnnotationData();
        }
    }
    
}

void LoadDASObjectTask::sl_replyFinished( QNetworkReply* reply ) {
    Q_UNUSED(reply);
    loop->exit();
}

void LoadDASObjectTask::sl_onError( QNetworkReply::NetworkError error ){
    stateInfo.setError(QString("NetworkReply error %1").arg(error));
    loop->exit();

}

void LoadDASObjectTask::sl_uploadProgress( qint64 bytesSent, qint64 bytesTotal ){
    stateInfo.progress = bytesSent/ bytesTotal * 100;
}

//////////////////////////////////////////////////////////////////////////
//ConvertAndLoadDASDocumentTask
ConvertIdAndLoadDASDocumentTask::ConvertIdAndLoadDASDocumentTask(const QString& accId,
                                                                 const QString& _fullPath,
                                                                 const DASSource& _referenceSource,
                                                                 const QList<DASSource>& _featureSources,
                                                                 bool _convertId) :
    Task(QString("Convert ID and load DAS document for: %1").arg(accId), TaskFlags(TaskFlag_CancelOnSubtaskCancel | TaskFlag_MinimizeSubtaskErrorText)),
    convertDasIdTask(NULL),
    loadDasDocumentTask(NULL),
    accessionNumber(accId),
    fullPath(_fullPath),
    referenceSource(_referenceSource),
    featureSources(_featureSources),
    convertId(_convertId)
{

}

void ConvertIdAndLoadDASDocumentTask::prepare() {
    if (convertId) {
        convertDasIdTask = new ConvertDasIdTask(accessionNumber);
        addSubTask(convertDasIdTask);
    } else {
        loadDasDocumentTask = new LoadDASDocumentTask(accessionNumber, fullPath, referenceSource, featureSources);
        addSubTask(loadDasDocumentTask);
    }
}

QList<Task*> ConvertIdAndLoadDASDocumentTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> subTasks;

    if (subTask->isCanceled()) {
        return subTasks;
    }

    if (subTask == convertDasIdTask) {
        if (!convertDasIdTask->getAccessionNumber().isEmpty() && !convertDasIdTask->hasError()) {
            ioLog.details(QString("\"%1\" was converted into \"%2\"").
                          arg(accessionNumber).
                          arg(convertDasIdTask->getAccessionNumber()));
            accessionNumber = convertDasIdTask->getAccessionNumber();
        }
        loadDasDocumentTask = new LoadDASDocumentTask(accessionNumber, fullPath, referenceSource, featureSources);
        subTasks << loadDasDocumentTask;
    }
    if (subTask == loadDasDocumentTask && loadDasDocumentTask->hasError()) {
        setError(loadDasDocumentTask->getError());
    }

    return subTasks;
}

//////////////////////////////////////////////////////////////////////////
//XMLDASSequenceParser
XMLDASSequenceParser::XMLDASSequenceParser(){
    seq = NULL;
}

#define DAS_SEQ_DASSEQUENCE "DASSEQUENCE"
#define DAS_SEQ_SEQUENCE "SEQUENCE"
#define DAS_SEQ_ID "id"
void XMLDASSequenceParser::parse( const QByteArray& data ){
    //http://www.biodas.org/documents/spec-1.6.html
    QDomDocument pDoc;
    pDoc.setContent(data);

    QDomElement dasSeq = pDoc.documentElement();
    if(dasSeq.tagName() != DAS_SEQ_DASSEQUENCE){
        setError(QString("No %1 tag").arg(DAS_SEQ_DASSEQUENCE));
        return;
    }
    //here may be multiple sequence tags, but we take only the first one
    QDomNode seqNode = dasSeq.firstChild();
    if (!seqNode.isNull()){
        QDomElement seqElement = seqNode.toElement();
        if(seqElement.tagName() != DAS_SEQ_SEQUENCE){
            setError(QString("No %1 tag").arg(DAS_SEQ_SEQUENCE));
            return;
        }
        QString sequenceId = seqElement.attribute(DAS_SEQ_ID).trimmed();
        QByteArray sequence = seqElement.text().toLatin1().trimmed();

        const DNAAlphabet* a = U2AlphabetUtils::findBestAlphabet(sequence.data(), sequence.size());

        seq = new DNASequence(sequenceId, sequence, a);
        
    }else{
        setError(QString("No %1 tag").arg(DAS_SEQ_SEQUENCE));
        return;
    }

}

//////////////////////////////////////////////////////////////////////////
//XMLDASFeaturesParser
XMLDASFeaturesParser::XMLDASFeaturesParser( ) {

}

#define DAS_FEATURE_DASGFF "DASGFF"
#define DAS_FEATURE_GFF "GFF"
#define DAS_FEATURE_SEGMENT "SEGMENT"
#define DAS_FEATURE_ID "id"
#define DAS_FEATURE_START "start"
#define DAS_FEATURE_STOP  "stop"
#define DAS_FEATURE_FEATURE "FEATURE"
#define DAS_FEATURE_LABEL "label"
#define DAS_FEATURE_HREF "href"

#define DAS_FEATURE_TYPE "TYPE"
#define DAS_FEATURE_METHOD "METHOD"
#define DAS_FEATURE_START_POS "START"
#define DAS_FEATURE_END_POS "END"
#define DAS_FEATURE_SCORE "SCORE"
#define DAS_FEATURE_ORIENTATION "ORIENTATION"
#define DAS_FEATURE_PHASE "PHASE"
#define DAS_FEATURE_NOTE "NOTE"
#define DAS_FEATURE_LINK "LINK"
#define DAS_FEATURE_TARGET "TARGET"
#define DAS_FEATURE_PARENT "PARENT"
#define DAS_FEATURE_PART "PART"

void XMLDASFeaturesParser::parse( const QByteArray& data ){
    //http://www.biodas.org/documents/spec-1.6.html
    QDomDocument pDoc;
    pDoc.setContent(data);

    //DASGFF
    QDomElement dasGff = pDoc.documentElement();
    if(dasGff.tagName() != DAS_FEATURE_DASGFF){
        setError(QString("No %1 tag").arg(DAS_FEATURE_DASGFF));
        return;
    }

    //GFF
    QDomNode gff = dasGff.firstChild();
    if (!gff.isNull()){
        QDomElement gffElement = gff.toElement();
        if(gffElement.tagName() != DAS_FEATURE_GFF){
            setError(QString("No %1 tag").arg(DAS_FEATURE_GFF));
            return;
        }

        //SEGMENT
        QDomNode gffSegment = gffElement.firstChild();
        while (!gffSegment.isNull()){
            QDomElement gffSegmentElement = gffSegment.toElement();
            if(gffSegmentElement.tagName() != DAS_FEATURE_SEGMENT){
                //no annotations
                return;
            }
            QString sequenceId = gffSegmentElement.attribute(DAS_FEATURE_ID);
            qint64 start = gffSegmentElement.attribute(DAS_FEATURE_START).toInt();
            qint64 stop = gffSegmentElement.attribute(DAS_FEATURE_STOP).toInt();

            //FEATURE
            QDomNode featureSegment = gffSegmentElement.firstChild();
            while (!featureSegment.isNull()){
                QDomElement featureSegmentElement = featureSegment.toElement();
                if(featureSegmentElement.tagName() != DAS_FEATURE_FEATURE){
                    setError(QString("No %1 tag").arg(DAS_FEATURE_FEATURE));
                    return;
                }
                //annotation data
                QString featureLabel = featureSegmentElement.attribute(DAS_FEATURE_LABEL);
                QString featureId = featureSegmentElement.attribute(DAS_FEATURE_ID);

                QString groupName = "";
                QString groupId = "";
                QString methodQual = "";
                qint64 startPos = -1;
                qint64 endPos = -1;
                float score = -1.0f;
                bool complemented = false;
                QString note = "";
                QString link = "";
                QString target = "";

                //FEATURE_ATTRS
                QDomNode featureAttrSegment = featureSegmentElement.firstChild();
                while (!featureAttrSegment.isNull()){
                    QDomElement featureAttrElement = featureAttrSegment.toElement();
                    
                    QString tagName = featureAttrElement.tagName();
                    if (tagName == DAS_FEATURE_TYPE){
                        //group name
                        groupName = featureAttrElement.text();
                        groupId = featureAttrElement.attribute(DAS_FEATURE_ID);
                        if (groupName.isEmpty()){
                            groupName = groupId;
                        }
                    }else if (tagName == DAS_FEATURE_METHOD){
                        methodQual = featureAttrElement.text();
                        if (methodQual.isEmpty()){
                            methodQual = featureAttrElement.attribute(DAS_FEATURE_ID);
                        } 

                    }else if (tagName == DAS_FEATURE_START_POS){
                        QString startText = featureAttrElement.text();
                        if (!startText.isEmpty()){
                            startPos = startText.toInt();
                        }
                        
                    }else if (tagName == DAS_FEATURE_END_POS){
                        QString endText = featureAttrElement.text();
                        if (!endText.isEmpty()){
                            endPos = endText.toInt();
                        }
                    }else if (tagName == DAS_FEATURE_SCORE){
                        QString scoreText = featureAttrElement.text();
                        if (!scoreText.isEmpty() && scoreText != "-"){
                            score = scoreText.toFloat();
                        }

                    }else if (tagName == DAS_FEATURE_ORIENTATION){
                        QString complText = featureAttrElement.text();
                        if (complText == "-"){
                            complemented = true;
                        }

                    }else if (tagName == DAS_FEATURE_PHASE){
                        //skip

                    }else if (tagName == DAS_FEATURE_NOTE){
                        note = featureAttrElement.text();

                    }else if (tagName == DAS_FEATURE_LINK){
                        link = featureAttrElement.attribute(DAS_FEATURE_HREF);

                    }else if (tagName == DAS_FEATURE_TARGET){
                        target = featureAttrElement.text();
                        if (target.isEmpty()){
                            target = featureAttrElement.attribute(DAS_FEATURE_ID);
                        }

                    }else if (tagName == DAS_FEATURE_PARENT){
                        //skip

                    }else if (tagName == DAS_FEATURE_PART){
                        //skip

                    }


                    featureAttrSegment = featureAttrSegment.nextSibling();
                }

                featureSegment = featureSegment.nextSibling();

                AnnotationData data;
                data.name = featureId.simplified( );
                U2Region reg; //1-based start

                
                if (startPos == -1 || endPos == -1){//non-positional
                    data.qualifiers.append(U2Qualifier("non-positional", "yes"));
                    if ((start == 0 && stop == 0) || (start > stop)){
                        reg = U2_REGION_MAX;
                    }else{
                        reg = U2Region(start - 1, stop - start + 1);
                    }
                }else{
                    if ((startPos == 0 && endPos == 0) || (startPos > endPos)){
                        reg = U2_REGION_MAX;
                    }else{
                        reg = U2Region(startPos - 1, endPos - startPos + 1);
                    }
                }
                data.location->regions << reg;
                data.setStrand(complemented ? U2Strand::Complementary : U2Strand::Direct);

                if (!methodQual.isEmpty()){
                    data.qualifiers.append(U2Qualifier("method", methodQual.simplified()));
                }

                if (score != -1.0f){
                    data.qualifiers.append(U2Qualifier("score", QString::number(score)));
                }

                if (!note.isEmpty()){
                    data.qualifiers.append(U2Qualifier("note", note.simplified()));
                }

                if (!link.isEmpty()){
                    data.qualifiers.append(U2Qualifier("link", link.simplified()));
                }

                if (!target.isEmpty()){
                    data.qualifiers.append(U2Qualifier("target", target.simplified()));
                }

                if (!sequenceId.isEmpty()){
                    data.qualifiers.append(U2Qualifier("seq_id", sequenceId.simplified()));
                }

                if (groupName.isEmpty()){
                    groupName = "das_features";
                }

                data.qualifiers.append(U2Qualifier("type", groupName.simplified()));

                if (!groupId.isEmpty()){
                    data.qualifiers.append(U2Qualifier("type_id", groupId.simplified()));
                }

                data.qualifiers.append(U2Qualifier("feature_id", featureId.simplified()));
                if (!featureLabel.isEmpty()){
                    data.qualifiers.append(U2Qualifier("feature_label", featureLabel.simplified()));
                }
                
                annotationData[groupName].append(data);
            }


            gffSegment = gffSegment.nextSibling();
        }

    }else{
        setError(QString("No %1 tag").arg(DAS_FEATURE_GFF));
        return;
    }
}

} //namespace
