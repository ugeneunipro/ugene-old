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

#include "HttpRequest.h"

namespace U2 {

const QString HttpRequestCDD::host = "http://www.ncbi.nlm.nih.gov/Structure/cdd/wrpsb.cgi?"; 

void HttpRequestCDD::sendRequest(const QString &params,const QString &query) {
    QString request = host;
    request.append(params);
    request.append("&seqinput=");
    request.append(query);

    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( BaseIOAdapters::HTTP_FILE );
    IOAdapter * io = iof->createIOAdapter();
    if(!io->open( request, IOAdapterMode_Read )) {
        connectionError = true; 
        error = QObject::tr("Cannot open the IO adapter");
        return;
    }
    int offs = 0;
    int read = 0;
    int CHUNK_SIZE = 1024;
    QByteArray response( CHUNK_SIZE, 0 );
    do {
        if(task->isCanceled()) {
            io->close();
            return;
        }
        read = io->readBlock( response.data() + offs, CHUNK_SIZE );
        offs += read;
        response.resize( offs + read );
    } while( read == CHUNK_SIZE );
    response.resize(offs);
    io->close();
    if(read<0){
        connectionError = true;
        error = QObject::tr("Cannot load page");
        return;
    }

    int ind = response.indexOf("RID = ") + 6;
    int ind2 = response.indexOf("<",ind);
    QString rid = response.mid(ind, ind2 - ind);

    while(response.indexOf("<title>Loading...</title>")!=-1) {
        Waiter::await(5000);

        io = iof->createIOAdapter();
        if(!io->open( host + "RID=" + rid, IOAdapterMode_Read )) {
            connectionError = true; 
            error = QObject::tr("Cannot open the IO adapter");
            return;
        }
        int offs = 0;
        int read = 0;
        int CHUNK_SIZE = 1024;
        response.resize(CHUNK_SIZE);
        response.fill(0);
        do {
            if(task->isCanceled()) {
                io->close();
                return;
            }
            read = io->readBlock( response.data() + offs, CHUNK_SIZE );
            offs += read;
            response.resize( offs + read );
        } while( read == CHUNK_SIZE );
        response.resize(offs);
        io->close();
        if(read<0){
            connectionError = true;
            error = QObject::tr("Cannot load page");
            return;
        }
    }

    if((response.indexOf("<title>Loading...</title>")!=-1) || (response.isEmpty())) {
        connectionError = true; 
        error = QObject::tr("The database did not respond");
        return;
    }

    if(response.indexOf("Query Exception:")!=-1) {
        connectionError = true;
        error = QObject::tr("Incorrect query");
        return;
    }

    ResponseBuffer buf;
    buf.setBuffer(&response);
    buf.open(QIODevice::ReadOnly);
    parseResult(buf);
}


void HttpRequestCDD::parseResult(ResponseBuffer &buf) {
    QByteArray b = buf.readLine();
    int fl = 0;
    while(b!= QString("</html>\n").toAscii()&&fl<2) {
        if(task->isCanceled()) {
            return;
        }
        b = buf.readLine();
        if(b.indexOf("<td class=\"descr\">Description</td>\n")!=-1)
            fl++;
        if(b.indexOf("<td class=\"descr\"><div>")!=-1) {
            parseHit(b,buf);
        }
    }
    connectionError = false;
}

void HttpRequestCDD::parseHit(QByteArray &b,ResponseBuffer &buf) {
    SharedAnnotationData ad(new AnnotationData());
    QString str(b);
    str = str.split("<div>").last();
    str = str.split("</div>")[0];

    ad->qualifiers.push_back(U2Qualifier("Description",str.split(",").last()));
    b = buf.readLine();
    ad->qualifiers.push_back(U2Qualifier("PssmId",extractText(b)));
    b = buf.readLine();
    ad->qualifiers.push_back(U2Qualifier("MultiDom",extractText(b)));
    b = buf.readLine();
    ad->qualifiers.push_back(U2Qualifier("E-Value",extractText(b)));
    buf.readLine();
    buf.readLine();
    b = buf.readLine();

    int ind = b.indexOf("<b>Cd Length:&nbsp;</b>") + 23;
    QByteArray t;
    for(int i = ind;b[i]!='&';i++)
        t[i-ind] = b[i];
    ad->qualifiers.push_back(U2Qualifier("cd_length",QString(t)));
    ind = b.indexOf("Bit Score: </b>") + 15;
    for(int i = ind;b[i]!='&';i++)
        t[i-ind] = b[i];
    ad->qualifiers.push_back(U2Qualifier("bit_score",QString(t)));
    
    int begLoc = 1000000;
    int endLoc = 0;
    QString id;
    while(!b.contains("</TABLE>")) {
        
        while(!b.contains("lcl|") && !b.contains("</TABLE>")) {
            b = buf.readLine();
        } 

        if(b.contains("</TABLE>")) {
            break;
        }

        int from,to;
        if(!getLocations(b,from,to)) {
            connectionError = true;
            error = QObject::tr("Cannot evaluate the location");
            return;
        }
        if(from < begLoc) {
            begLoc = from;
        }
        endLoc = to;

        b = buf.readLine();
        //b = buf.readLine();
        ind = b.indexOf("Cdd:") + 4;
        for(int i = ind;b[i]!='<' && i < b.length();i++) {
            t[i-ind] = b[i];
        }
        id = QString(t);
    }
    ad->location->regions << U2Region( begLoc-1, endLoc - begLoc + 1);
    ad->qualifiers.push_back(U2Qualifier("id",QString(t)));

    ad->name = "CDD result";
    result.append(ad);
}

QString HttpRequestCDD::extractText(const QByteArray &b) {
    QString str(b);
    str = str.split("</")[0];
    str = str.split(">").last();
    return str;
}

bool HttpRequestCDD::getLocations(QByteArray &b,int &from, int &to) {
    QString str(b);
    QStringList L = str.split("</font>");
    bool isOk;
    from = L[0].split(">").last().toInt(&isOk);
    if(!isOk) {
        return false;
    }
    to = L[L.count()-2].split(">").last().toInt(&isOk);
    if(!isOk) {
        return false;
    }
    if(from < 0 && to < 0) {
        from *= -1;
        to *= -1;
    }
    return true;
}

}
