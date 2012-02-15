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

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_


#include <U2Core/Log.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/DataBaseRegistry.h>

#include <QtCore/QtCore>
#include <QtXml/QDomNode>
#include <QtXml/QDomDocument>

#include "RemoteBLASTConsts.h"

namespace U2 {

class RemoteBLASTTask;

struct ResponseBuffer {
    ResponseBuffer(){}
    ~ResponseBuffer() { buf.close(); }
    void setBuffer(QByteArray *arr) { buf.setBuffer(arr); }
    bool open(QIODevice::OpenMode openMode) { return buf.open(openMode); }
    void close() { buf.close(); }
    QByteArray readLine() { return buf.readLine(); }

    QBuffer buf;
};


class HttpRequestBLAST:public HttpRequest {
public:
    HttpRequestBLAST(Task *_task):HttpRequest(_task){};
    virtual void sendRequest(const QString &program,const QString &query);
    virtual void parseResult(const QByteArray &buf);
    virtual QByteArray getOutputFile();

private:
    class Waiter: public QThread {
    public:
        static void await(int mseconds) {
            msleep(mseconds);
        }
    };
    static const QString host;
    QByteArray output;
    void parseHit(const QDomNode &xml);
    void parseHsp(const QDomNode &xml,const QString &id, const QString &def, const QString &accession);
};

class HttpRequestCDD:public HttpRequest {
public:
    HttpRequestCDD(Task *_task):HttpRequest(_task){};
    virtual void sendRequest(const QString &program,const QString &query);
    virtual void parseResult(ResponseBuffer &buf);

private:
    class Waiter: public QThread {
    public:
        static void await(int mseconds) {
            msleep(mseconds);
        }
    };
    static const QString host;
    void parseHit(QByteArray &b,ResponseBuffer &buf);
    QString extractText(const QByteArray &b);
    bool getLocations(QByteArray &b,int &from, int &to);
};

}

#endif
