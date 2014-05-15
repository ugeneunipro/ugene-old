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

#include "HttpRequest.h"
#include "RemoteBLASTTask.h"
#include <U2Core/NetworkConfiguration.h>

namespace U2 {

const QString HttpRequestBLAST::host = "http://blast.ncbi.nlm.nih.gov/Blast.cgi?"; 

QString HttpRequestBLAST::runHttpRequest(QString request){
    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( BaseIOAdapters::HTTP_FILE );
    IOAdapter * io = iof->createIOAdapter();
    if(!io->open( request, IOAdapterMode_Read )) {
        connectionError = true; 
        error = QObject::tr("Cannot open the IO adapter");
        return "";
    }
    int offs = 0;
    int read = 0;
    int CHUNK_SIZE = 1024;
    QByteArray response( CHUNK_SIZE, 0 );
    do {
        if(task->isCanceled()) {
            io->close();
            return "";
        }
        read = io->readBlock( response.data() + offs, CHUNK_SIZE );
        offs += read;
        response.resize( offs + read );
    } while( read == CHUNK_SIZE );
    io->close();
    if(read<0) {
        connectionError = true; 
        error = QObject::tr("Cannot load a page. %1").arg(io->errorString());
        return "";
    }
    response.truncate(offs);
    QString ret(response);

    return QString(response);
}

void HttpRequestBLAST::sendRequest(const QString &params,const QString &query) {
    QString request = host;
    request.append(params);
    request.append("&OLD_VIEW=true");
    request.append("&TOOL=ugene&EMAIL=ugene-ncbi-blast@unipro.ru");
    request.append(RemoteRequestConfig::HTTP_BODY_SEPARATOR);
    request.append(ReqParams::sequence + "=" + query);
    QString response = runHttpRequest(request);
    if(response.indexOf("301 Moved Permanently") != -1) {
        int start = response.indexOf("href=") + 6;
        QString req2 = response.mid(start, response.lastIndexOf(">here</a>")-start - 1);
        req2.remove("amp;");
        response = runHttpRequest(req2);
    }
    ResponseBuffer buf;
    QByteArray qbResponse(response.toLatin1());
    buf.setBuffer(&qbResponse);
    buf.open(QIODevice::ReadOnly);
    QByteArray b = buf.readLine();
    while(b!=QString("<!--QBlastInfoBegin\n").toLatin1()) {
        if(task->isCanceled()) {
            return;
        }
        b = buf.readLine();
        if(b.indexOf("Error: Failed to read the Blast query: Nucleotide FASTA provided for protein sequence") != -1) {
            connectionError = false;
            return;
        }
    }
    QString requestID = buf.readLine();
    requestID = requestID.split("=")[1];
    requestID = requestID.mid(1,requestID.length()-2);
    if(requestID=="") {
        connectionError = true; 
        error = QObject::tr("Cannot get the request ID");
        return;
    }
    QString roe = buf.readLine().split('=')[1];
    bool isOk;
    int rtoe = roe.toInt(&isOk);
    if(!isOk) {
        connectionError = true; 
        error = QObject::tr("Cannot get the waiting time");
        return;
    }
    request = host + "CMD=Get&FORMAT_TYPE=XML&RID=";
    request.append(requestID);
    buf.close();
    RemoteBLASTTask *rTask = qobject_cast<RemoteBLASTTask*>(task);
    int progr,timeout;
    progr = 50;
    timeout = (rtoe + 5) * 10;
    int slowdown = 1;
    rTask->resetProgress();

    do {
        for(int i = 0;i< (progr / slowdown); i++) {
            if(rTask->isCanceled())
                return;
            Waiter::await(timeout*slowdown);
            rTask->updateProgress();
        }
        response = runHttpRequest(request);
        if(response.isEmpty()){
            connectionError = true; 
            error = QObject::tr("The response is empty");
            return;
        } 
        if(response.indexOf("301 Moved Permanently") != -1) {
            int start = response.indexOf("href=") + 6;
            QString req2 = response.mid(start, response.lastIndexOf(">here</a>")-start - 1);
            req2.remove("amp;");
            response = runHttpRequest(req2);
        }
        if(slowdown < 32) {
            slowdown *= 2; //If attempt was unsuccessful, progress bar slows down
        }
    }
    while(response.indexOf("Status=WAITING")!=-1 && rTask->timeout);

    if(response.indexOf("Status=WAITING")!=-1 || response.indexOf("<BlastOutput>")==-1 || response.indexOf("</BlastOutput>")==-1){
        connectionError = true; 
        error = QObject::tr("Database couldn't prepare the response. You can increase timeout and perform search again.");
        return;
    }

    if(response.contains("Error: CPU usage limit was exceeded")) {
        error = QObject::tr("CPU usage limit in BLAST was exceeded, probably query sequence is too large");
        return;
    }

    output = response.toLatin1();
    parseResult(response.toLatin1());
}

QByteArray HttpRequestBLAST::getOutputFile() {
    return output;
}

void HttpRequestBLAST::parseResult(const QByteArray &buf) {
    QDomDocument xmlDoc;
    QString xmlError;
    xmlDoc.setContent(buf,false,&xmlError);
    if(!xmlError.isEmpty()) {
        connectionError = true;
        error = QObject::tr("Cannot read the response");
        return;
    }
    QDomNodeList hits = xmlDoc.elementsByTagName("Hit");
    for(int i = 0; i<hits.count();i++) {
        parseHit(hits.at(i));
    }
    connectionError = false;

    RemoteBLASTTask *rTask = qobject_cast<RemoteBLASTTask*>(task);
    for(int i = rTask->getProgress() ; i < 100 ; i++) {
        rTask->updateProgress();
    }
}

void HttpRequestBLAST::parseHit(const QDomNode &xml) {
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
                        parseHsp(hsps.at(j),id,def,accession);
            }
        }
    }
}

void HttpRequestBLAST::parseHsp(const QDomNode &xml,const QString &id, const QString &def, const QString &accession) {
    AnnotationData ad;
    bool isOk;
    int from = -1,to = -1,align_len = -1,gaps = -1,identities = -1;	

    QDomElement elem = xml.lastChildElement("Hsp_bit-score");
    if(!elem.isNull()) {
        ad.qualifiers.push_back(U2Qualifier("bit-score", elem.text()));
    }
    
    elem = xml.lastChildElement("Hsp_score");
    if(!elem.isNull()) {
        ad.qualifiers.push_back(U2Qualifier("score", elem.text()));
    }

    elem = xml.lastChildElement("Hsp_evalue");
    if(!elem.isNull()) {
        ad.qualifiers.push_back(U2Qualifier("E-value", elem.text()));
    }

    elem = xml.lastChildElement("Hsp_query-from");
    QString fr = elem.text();
    from = elem.text().toInt(&isOk);
    if(!isOk) {
        connectionError = true; 
        error = QObject::tr("Cannot get the location");
        return;
    }

    elem = xml.lastChildElement("Hsp_query-to");
    to = elem.text().toInt(&isOk);  
    if(!isOk) {
        connectionError = true; 
        error = QObject::tr("Cannot get the location");
        return;
    }

    elem = xml.lastChildElement("Hsp_hit-from");
    if(!elem.isNull()) {
        ad.qualifiers.push_back(U2Qualifier("hit-from", elem.text()));
    }

    elem = xml.lastChildElement("Hsp_hit-to");
    if(!elem.isNull()) {
        ad.qualifiers.push_back(U2Qualifier("hit-to", elem.text()));
    }

    elem = xml.lastChildElement("Hsp_hit-frame");
    int frame = elem.text().toInt(&isOk);
    if(!isOk) {
        connectionError = true; 
        error = QObject::tr("Cannot get the location");
        return;
    }
    QString frame_txt = (frame < 0) ? "complement" : "direct";
    ad.qualifiers.push_back(U2Qualifier( "source_frame", frame_txt ));
    ad.setStrand(frame < 0 ? U2Strand::Complementary : U2Strand::Direct);

    elem = xml.lastChildElement("Hsp_identity");
    identities = elem.text().toInt(&isOk);
    if(!isOk) {
        connectionError = true; 
        error = QObject::tr("Cannot get the identity");
        return;
    }

    elem = xml.lastChildElement("Hsp_gaps");
    gaps = elem.text().toInt(&isOk);
    if(!isOk) {
        connectionError = true; 
        error = QObject::tr("Cannot evaluate the gaps");
        return;
    }

    elem = xml.lastChildElement("Hsp_align-len");
    align_len = elem.text().toInt(&isOk);
    if(!isOk) {
        connectionError = true; 
        error = QObject::tr("Cannot get the alignment length");
        return;
    }

    if( from != -1 && to != -1 ) {
        if (to > from){ //direct
            ad.location->regions << U2Region( from-1, to - from + 1);
            ad.setStrand(U2Strand::Direct);
        } else { //complement
            ad.location->regions << U2Region( to-1, from - to + 1);
            ad.setStrand(U2Strand::Complementary);
        }
    } else {
        connectionError = true; 
        error = QObject::tr("Cannot evaluate the location");
        return;
    }

    if( align_len != -1 ) {
        if( gaps != -1 ) {
            float percent = (float)gaps / (float)align_len * 100.;
            QString str = QString::number(gaps) + "/" + QString::number(align_len) + " (" + QString::number(percent,'g',4) + "%)";
            ad.qualifiers.push_back(U2Qualifier( "gaps", str ));
        }
        if( identities != -1 ) {
            float percent = (float)identities / (float)align_len * 100.;
            QString str = QString::number(identities) + '/' + QString::number(align_len) + " (" + QString::number(percent,'g',4) + "%)";
            ad.qualifiers.push_back(U2Qualifier( "identities", str ));
        }
    }

    ad.qualifiers.push_back(U2Qualifier("id",id));
    ad.qualifiers.push_back(U2Qualifier("def",def));
    ad.qualifiers.push_back(U2Qualifier("accession",accession));
    ad.name = "blast result";
    result.append(ad);
}


}

