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

#include "BlastAllSupportTask.h"
#include "BlastAllSupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SequenceUtils.h>

#include <QtXml/QDomDocument>

#include <U2Core/CreateAnnotationTask.h>
//#include <U2Core/AddDocumentTask.h>

namespace U2 {

BlastAllSupportTask::BlastAllSupportTask(const BlastTaskSettings& _settings) :
        Task("Run NCBI BlastAll task", TaskFlags_NR_FOSCOE),
        settings(_settings)
{
    GCOUNTER( cvar, tvar, "BlastAllSupportTask" );
    blastAllTask=NULL;
    logParser=NULL;
    tmpDoc=NULL;
    saveTemporaryDocumentTask=NULL;
    sequenceObject=NULL;
    addTaskResource(TaskResourceUsage(RESOURCE_THREAD, settings.numberOfProcessors));
}

void BlastAllSupportTask::prepare(){
    //Add new subdir for temporary files
    //Directory name is ExternalToolName + CurrentDate + CurrentTime

    QString tmpDirName = "BlastAll_"+QString::number(this->getTaskId())+"_"+
                         QDate::currentDate().toString("dd.MM.yyyy")+"_"+
                         QTime::currentTime().toString("hh.mm.ss.zzz")+"_"+
                         QString::number(QCoreApplication::applicationPid())+"/";
    //Check and remove subdir for temporary files
    QString blastTmpDir = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(BLASTALL_TMP_DIR);
    QDir tmpDir(blastTmpDir + "/"+ tmpDirName);
    if(tmpDir.exists()){
        foreach(const QString& file, tmpDir.entryList()){
            tmpDir.remove(file);
        }
        if(!tmpDir.rmdir(tmpDir.absolutePath())){
            stateInfo.setError(tr("Subdirectory for temporary files exists. Can not remove this directory."));
            return;
        }
    }
    if (!tmpDir.mkpath(tmpDir.absolutePath())) {
        stateInfo.setError(tr("Can not create directory for temporary files."));
        return;
    }
    //Create ncbi.ini for windows or .ncbirc for unix like systems
    //See issue UGENE-791 (https://ugene.unipro.ru/tracker/browse/UGENE-791)
#ifdef Q_OS_UNIX
    QString iniNCBIFile=tmpDir.absolutePath()+QString("/.ncbirc");
#else
    QString iniNCBIFile=tmpDir.absolutePath()+QString("\\ncbi.ini");
#endif
    if(!QFileInfo(iniNCBIFile).exists()){
        QFile file(iniNCBIFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            algoLog.details(tr("Can not create fake NCBI ini file"));
        }else{
            file.close();
        }
    }

    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::FASTA);
    tmpDoc = df->createNewLoadedDocument(IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), GUrl(url), stateInfo);
    CHECK_OP(stateInfo, );

    U2EntityRef seqRef = U2SequenceUtils::import(tmpDoc->getDbiRef(), DNASequence(settings.querySequence, settings.alphabet), stateInfo);
    CHECK_OP(stateInfo, );
    sequenceObject = new U2SequenceObject("input sequence", seqRef);
    tmpDoc->addObject(sequenceObject);
    url = tmpDir.absolutePath() + "/tmp.fa";
    
    saveTemporaryDocumentTask = new SaveDocumentTask(tmpDoc, AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE), url);
    saveTemporaryDocumentTask->setSubtaskProgressWeight(5);
    addSubTask(saveTemporaryDocumentTask);
}
QList<Task*> BlastAllSupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    if(hasError() || isCanceled()) {
        return res;
    }
    if(subTask==saveTemporaryDocumentTask){
        delete tmpDoc;//sequenceObject also deleted at this place
        QStringList arguments;
        if("cuda-blastp" != settings.programName) {
            arguments <<"-p"<< settings.programName;
        }
        if(!settings.filter.isEmpty()){
            arguments <<"-F"<<settings.filter;
        }
        arguments <<"-d"<< settings.databaseNameAndPath;
        arguments <<"-e"<< QString::number(settings.expectValue);
        if("cuda-blastp" != settings.programName) {
            arguments <<"-n"<< (settings.megablast ? "T" : "F");
        }
        arguments <<"-W"<< QString::number(settings.wordSize);
        if(!settings.isDefaultCosts){
            arguments <<"-G"<< QString::number(settings.gapOpenCost);
            arguments <<"-E"<< QString::number(settings.gapExtendCost);
        }
        //if(settings.isNucleotideSeq && (!settings.isDefautScores)){
        if((settings.programName == "blastn") && (!settings.isDefautScores)){
            arguments <<"-q"<< QString::number(settings.mismatchPenalty);
            arguments <<"-r"<< QString::number(settings.matchReward);
        }else{
            if(!settings.isDefaultMatrix){
                arguments <<"-M"<< settings.matrix;
            }
        }
        if("cuda-blastp" != settings.programName) {
            if(settings.numberOfHits != 0){
                arguments <<"-K" << QString::number(settings.numberOfHits);
            }
        }
        arguments <<"-i"<< url;
        if (settings.programName != "tblastx"){
            if((settings.programName == "blastn" && settings.xDropoffGA != 30) ||
                    (settings.programName == "blastn" && settings.megablast && settings.xDropoffGA != 20) ||
                    (settings.programName != "blastn" && settings.xDropoffGA != 15))
            {
                arguments << "-X" << QString::number(settings.xDropoffGA);
            }
            if((settings.programName == "blastn" && settings.xDropoffFGA != 100) ||
                    (settings.programName != "blastn" && settings.xDropoffFGA != 25))
            {
                arguments << "-Z" << QString::number(settings.xDropoffFGA);
            }
        }
        if((settings.programName == "blastn" && settings.xDropoffUnGA != 20) ||
                (settings.programName == "blastn" && settings.megablast && settings.xDropoffUnGA != 10) ||
                (settings.programName != "blastn" && settings.xDropoffUnGA != 7))
        {
            arguments << "-y" << QString::number(settings.xDropoffUnGA);
        }
        if((settings.programName=="blastn" && settings.windowSize != 0)||
                (settings.programName != "blastn" && settings.windowSize !=40))
        {
            arguments << "-A" << QString::number(settings.windowSize);
        }
        if(!settings.isDefaultThreshold){
            arguments << "-f" << QString::number(settings.threshold);
        }

//        arguments <<"-I" << "T";
        arguments <<"-a"<< QString::number(settings.numberOfProcessors);
        arguments <<"-m"<< QString::number(settings.outputType);//"7";//By default set output file format to xml
        if(settings.programName != "tblastx"){
            if(!settings.isGappedAlignment){
                arguments << "-g" << "F";
            }
        }
        if(settings.outputOriginalFile.isEmpty()){
            arguments <<"-o"<< url+".xml";//settings.outputRepFile;
            settings.outputOriginalFile = url+".xml";
        }else{
            arguments <<"-o"<< settings.outputOriginalFile;
        }

        logParser=new ExternalToolLogParser();
        QString workingDirectory=QFileInfo(url).absolutePath();
        if("cuda-blastp" == settings.programName) {
            blastAllTask=new ExternalToolRunTask(CUDA_BLASTP_TOOL_NAME,arguments, logParser, workingDirectory);
        } else {
            blastAllTask=new ExternalToolRunTask(BLASTALL_TOOL_NAME,arguments, logParser, workingDirectory);
        }
        blastAllTask->setSubtaskProgressWeight(95);
        res.append(blastAllTask);
    }
    else if(subTask==blastAllTask){
        assert(logParser);
        delete logParser;
        if(settings.outputType == 7 || settings.outputType == 8){
            if(!QFileInfo(settings.outputOriginalFile).exists()){
                if(AppContext::getExternalToolRegistry()->getByName(BLASTALL_TOOL_NAME)->isValid()){
                    stateInfo.setError(tr("Output file not found"));
                }else{
                    stateInfo.setError(tr("Output file not found. May be %1 tool path '%2' not valid?")
                                       .arg(AppContext::getExternalToolRegistry()->getByName(BLASTALL_TOOL_NAME)->getName())
                                       .arg(AppContext::getExternalToolRegistry()->getByName(BLASTALL_TOOL_NAME)->getPath()));
                }
                emit si_stateChanged();
                return res;
            }
            if(settings.outputType == 7){
                parseXMLResult();
            }else if(settings.outputType == 8){
                parseTabularResult();
            }
            if((!result.isEmpty())&&(settings.needCreateAnnotations)) {
               // Document* d = AppContext::getProject()->findDocumentByURL(url);
                //assert(d==NULL);
                if(!settings.outputResFile.isEmpty()) {
                    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
                    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);
                    Document *d = df->createNewLoadedDocument(iof, settings.outputResFile, stateInfo);
                    CHECK_OP(stateInfo, res);
                    d->addObject(settings.aobj);
                    AppContext::getProject()->addDocument(d);
                }

                for(QMutableListIterator<SharedAnnotationData> it_ad(result); it_ad.hasNext(); ) {
                    AnnotationData * ad = it_ad.next().data();
                    U2Region::shift(settings.offsInGlobalSeq, ad->location->regions);
                }

                res.append(new CreateAnnotationsTask(settings.aobj, settings.groupName, result));
            }
        }
    }
    return res;
}
Task::ReportResult BlastAllSupportTask::report(){
    if( url.isEmpty() ) {
        return ReportResult_Finished;
    }
    
    //Remove subdir for temporary files, that created in prepare
    QDir tmpDir(QFileInfo(url).absoluteDir());
    foreach(QString file, tmpDir.entryList(QDir::Files|QDir::Hidden)){
        tmpDir.remove(file);
    }
    if(!tmpDir.rmdir(tmpDir.absolutePath())){
        stateInfo.setError(tr("Can not remove directory for temporary files."));
        emit si_stateChanged();
    }
    return ReportResult_Finished;
}

BlastTaskSettings BlastAllSupportTask::getSettings() const {
    return settings;
}
QList<SharedAnnotationData> BlastAllSupportTask::getResultedAnnotations() const {
    return result;
}

void BlastAllSupportTask::parseTabularResult() {
    QFile file(settings.outputOriginalFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        stateInfo.setError("Can't open output file");
        return;
    }
    while(!file.atEnd()){
        QByteArray line = file.readLine();
        if (line.startsWith("#")){//skip comment
            continue;
        }
        parseTabularLine(line);
    }
    file.close();
}
void BlastAllSupportTask::parseTabularLine(const QByteArray &line){
    SharedAnnotationData ad(new AnnotationData());
    bool isOk;
    int from = -1,to = -1,align_len = -1,gaps = -1, identities = -1, hitFrom = -1,hitTo = -1;
    //Fields: Query id (0), Subject id(1), % identity(2), alignment length(3), mismatches(4), gap openings(5), q. start(6), q. end(7), s. start(8), s. end(9), e-value(10), bit score(11)
    QList<QByteArray> elements=line.split('\t');
    if(elements.size()!=12){
        stateInfo.setError(tr("Incorrect number of fields in line: %1").arg(elements.size()));
        return;
    }
    from = elements.at(6).toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get location");
        return;
    }
    to = elements.at(7).toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get location");
        return;
    }
    if( from != -1 && to != -1 ) {
        if(from <= to){
            ad->location->regions << U2Region( from-1, to - from + 1);
        }else{
            ad->location->regions << U2Region( to-1, from - to + 1);
        }
    } else {
        stateInfo.setError("Can't evaluate location");
        return;
    }

    hitFrom = elements.at(8).toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get hit location");
        return;
    }
    hitTo = elements.at(9).toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get hit location");
        return;
    }
    if( hitFrom != -1 && hitTo != -1 ) {
        if(hitFrom <= hitTo){
            ad->setStrand(U2Strand::Direct);
            ad->qualifiers.push_back(U2Qualifier( "source_frame","direct"));
            ad->qualifiers.push_back(U2Qualifier("hit-from", QString::number(hitFrom)));
            ad->qualifiers.push_back(U2Qualifier("hit-to", QString::number(hitTo)));
        }else{
            ad->setStrand(U2Strand::Complementary);
            ad->qualifiers.push_back(U2Qualifier( "source_frame","complement"));
            ad->qualifiers.push_back(U2Qualifier("hit-to", QString::number(hitFrom)));
            ad->qualifiers.push_back(U2Qualifier("hit-from", QString::number(hitTo)));
        }
    } else {
        stateInfo.setError("Can't evaluate hit location");
        return;
    }
    QString elem=QString(elements.at(11));
    if(elem.endsWith('\n')){
        elem.resize(elem.size()-1);//remove \n symbol
    }
    double bitScore = elem.toDouble(&isOk);
    if (isOk){
        ad->qualifiers.push_back(U2Qualifier("bit-score", QString::number(bitScore)));
    }

    align_len = elements.at(3).toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get align length");
        return;
    }
    gaps = elements.at(4).toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get gaps");
        return;
    }
    float identitiesPercent = elements.at(2).toFloat(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get identity");
        return;
    }
    if( align_len != -1 ) {
        if( gaps != -1 ) {
            float percent = (float)gaps / (float)align_len * 100;
            QString str = QString::number(gaps) + "/" + QString::number(align_len) + " (" + QString::number(percent,'g',4) + "%)";
            ad->qualifiers.push_back(U2Qualifier( "gaps", str ));
        }
        if( identitiesPercent != -1 ) {
            //float percent = (float)identities / (float)align_len * 100;
            identities=(float)align_len*identitiesPercent/100.;
            QString str = QString::number(identities) + '/' + QString::number(align_len) + " (" + QString::number(identitiesPercent,'g',4) + "%)";
            ad->qualifiers.push_back(U2Qualifier( "identities", str ));
        }
    }
    if(!elements.at(10).isEmpty()) {
        ad->qualifiers.push_back(U2Qualifier("E-value", elements.at(10)));
    }

    ad->qualifiers.push_back(U2Qualifier("id",elements.at(1)));
    ad->name = "blast result";
    result.append(ad);
}
void BlastAllSupportTask::parseXMLResult() {

    QDomDocument xmlDoc;
    QFile file(settings.outputOriginalFile);
    if (!file.open(QIODevice::ReadOnly)){
        stateInfo.setError("Can't open output file");
        return;
    }
    if (!xmlDoc.setContent(&file)) {
        //stateInfo.setError("Can't read output file");
        file.close();
        return;
    }
    file.close();

    QDomNodeList hits = xmlDoc.elementsByTagName("Hit");
    for(int i = 0; i<hits.count();i++) {
        parseXMLHit(hits.at(i));
    }
}

void BlastAllSupportTask::parseXMLHit(const QDomNode &xml) {
    QString id,def,accession;

    QDomElement tmp = xml.lastChildElement("Hit_id");
    id = tmp.text();
    tmp = xml.lastChildElement("Hit_def");
    def = tmp.text();
    tmp = xml.lastChildElement("Hit_accession");
    accession = tmp.text();

    QDomNodeList nodes = xml.childNodes();
    for(int i = 0; i < nodes.count(); i++) {
        if(nodes.at(i).isElement()) {
            if(nodes.at(i).toElement().tagName()=="Hit_hsps") {
                QDomNodeList hsps = nodes.at(i).childNodes();
                for(int j = 0;j<hsps.count();j++)
                    if(hsps.at(j).toElement().tagName()=="Hsp")
                        parseXMLHsp(hsps.at(j),id,def,accession);
            }
        }
    }
}

void BlastAllSupportTask::parseXMLHsp(const QDomNode &xml,const QString &id, const QString &def, const QString &accession) {
    SharedAnnotationData ad(new AnnotationData());
    bool isOk;
    int from = -1,to = -1,align_len = -1,gaps = -1,identities = -1;

    QDomElement elem = xml.lastChildElement("Hsp_bit-score");
    if(!elem.isNull()) {
        ad->qualifiers.push_back(U2Qualifier("bit-score", elem.text()));
    }

    elem = xml.lastChildElement("Hsp_score");
    if(!elem.isNull()) {
        ad->qualifiers.push_back(U2Qualifier("score", elem.text()));
    }

    elem = xml.lastChildElement("Hsp_evalue");
    if(!elem.isNull()) {
        ad->qualifiers.push_back(U2Qualifier("E-value", elem.text()));
    }

    elem = xml.lastChildElement("Hsp_query-from");
    QString fr = elem.text();
    from = elem.text().toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get location");
        return;
    }

    elem = xml.lastChildElement("Hsp_query-to");
    to = elem.text().toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get location");
        return;
    }

    elem = xml.lastChildElement("Hsp_hit-from");
    if(!elem.isNull()) {
        ad->qualifiers.push_back(U2Qualifier("hit-from", elem.text()));
    }

    elem = xml.lastChildElement("Hsp_hit-to");
    if(!elem.isNull()) {
        ad->qualifiers.push_back(U2Qualifier("hit-to", elem.text()));
    }

    elem = xml.lastChildElement("Hsp_hit-frame");
    int frame = elem.text().toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get location");
        return;
    }
    QString frame_txt = (frame < 0) ? "complement" : "direct";
    ad->qualifiers.push_back(U2Qualifier( "source_frame", frame_txt ));
    ad->setStrand(frame < 0 ? U2Strand::Complementary :  U2Strand::Direct);

    elem = xml.lastChildElement("Hsp_identity");
    identities = elem.text().toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get identity");
        return;
    }

    elem = xml.lastChildElement("Hsp_gaps");
    if(!elem.isNull()) {
        gaps = elem.text().toInt(&isOk);
        if(!isOk) {
            stateInfo.setError("Can't get gaps");
            return;
        }
    }

    elem = xml.lastChildElement("Hsp_align-len");
    align_len = elem.text().toInt(&isOk);
    if(!isOk) {
        stateInfo.setError("Can't get align length");
        return;
    }

    if( from != -1 && to != -1 ) {
        if(from <= to){
            ad->location->regions << U2Region( from-1, to - from + 1);
        }else{
            ad->location->regions << U2Region( to-1, from - to + 1);
        }
    } else {
        stateInfo.setError("Can't evaluate location");
        return;
    }

    if( align_len != -1 ) {
        if( gaps != -1 ) {
            float percent = (float)gaps / (float)align_len * 100;
            QString str = QString::number(gaps) + "/" + QString::number(align_len) + " (" + QString::number(percent,'g',4) + "%)";
            ad->qualifiers.push_back(U2Qualifier( "gaps", str ));
        }
        if( identities != -1 ) {
            float percent = (float)identities / (float)align_len * 100;
            QString str = QString::number(identities) + '/' + QString::number(align_len) + " (" + QString::number(percent,'g',4) + "%)";
            ad->qualifiers.push_back(U2Qualifier( "identities", str ));
            ad->qualifiers.push_back(U2Qualifier( "identity_percent", QString::number(percent) ));
        }
    }

    // parse alignment
    {
        elem = xml.lastChildElement("Hsp_qseq");
        QByteArray qSeq = elem.text().toAscii();

        elem = xml.lastChildElement("Hsp_hseq");
        QByteArray hSeq = elem.text().toAscii();

        elem = xml.lastChildElement("Hsp_midline");
        QByteArray midline = elem.text().toAscii();

        QByteArray cigar;
        int length = midline.length();
        char prevCigarChar = '*';
        int cigarCount = 0;

        for (int i = 0; i < length; ++i) {
            char cigarChar;
            char gapChar = midline.at(i);
            if (gapChar == '|') {
                cigarChar = 'M';
            } else {
                if (qSeq.at(i) == '-') {
                    cigarChar = 'D';
                } else if ( hSeq.at(i) == '-')  {
                    cigarChar = 'I';
                } else {
                    cigarChar = 'X';
                }
            }

            if ( i > 0 && prevCigarChar != cigarChar ) {
                cigar.append( QByteArray::number(cigarCount) ).append(prevCigarChar);
                cigarCount = 0;
            }

            prevCigarChar = cigarChar;
            cigarCount++;

            if (i == length - 1 ) {
                cigar.append( QByteArray::number(cigarCount) ).append( cigarChar );
            }
        }

        ad->qualifiers.append( U2Qualifier("subj_seq", hSeq) );
        ad->qualifiers.append( U2Qualifier("cigar", cigar) );

    }

    ad->qualifiers.push_back(U2Qualifier("id",id));
    ad->qualifiers.push_back(U2Qualifier("def",def));
    ad->qualifiers.push_back(U2Qualifier("accession",accession));
    ad->name = "blast result";
    result.append(ad);
}
///////////////////////////////////////
//BlastAllSupportMultiTask
BlastAllSupportMultiTask::BlastAllSupportMultiTask(QList<BlastTaskSettings>& _settingsList, QString& _url):
        Task("Run NCBI BlastAll multitask", TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported),
        settingsList(_settingsList), doc(NULL), url(_url)
{
}
void BlastAllSupportMultiTask::prepare(){
    //create document
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);
    doc = df->createNewLoadedDocument(iof, url, stateInfo);
    CHECK_OP(stateInfo, );

    foreach(BlastTaskSettings settings, settingsList){
        settings.needCreateAnnotations=false;
        addSubTask(new BlastAllSupportTask(settings));
    }
}
QList<Task*> BlastAllSupportMultiTask::onSubTaskFinished(Task *subTask){
    QList<Task*> res;
    if(subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    if(hasError() || isCanceled()) {
        return res;
    }
    BlastAllSupportTask* s=qobject_cast<BlastAllSupportTask*>(subTask);
    if(s != NULL){
        BlastTaskSettings settings=s->getSettings();
        assert(settings.aobj!=NULL);
        QList<SharedAnnotationData> result=s->getResultedAnnotations();
        if(result.length()>0){
            doc->addObject(settings.aobj);
            for(QMutableListIterator<SharedAnnotationData> it_ad(result); it_ad.hasNext(); ) {
                AnnotationData * ad = it_ad.next().data();
                U2Region::shift(settings.offsInGlobalSeq, ad->location->regions);
            }
            res.append(new CreateAnnotationsTask(settings.aobj, settings.groupName, result));
        }
    }
    return res;
}
Task::ReportResult BlastAllSupportMultiTask::report(){
    if(!hasError()){
        if(doc->getObjects().length() > 0){
            AppContext::getProject()->addDocument(doc);
        }else{
            setReportingEnabled(true);
        }
    }
    return ReportResult_Finished;
}

QString BlastAllSupportMultiTask::generateReport() const {
    QString res;

    res+="<table>";
    res+="<tr><td width=200><b>" + tr("Source file") + "</b></td><td>" + settingsList.at(0).queryFile + "</td></tr>";
    res+="<tr><td width=200><b>" + tr("Used databse") + "</b></td><td>" + settingsList.at(0).databaseNameAndPath + "</td></tr>";
    res+="<tr></tr>";
    res+="<tr><td width=200><b>" + tr("No any results found") + "</b></td><td></td></tr>";
    res+="</table>";
    return res;
}

}//namespace
