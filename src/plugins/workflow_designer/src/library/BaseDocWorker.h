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

#ifndef _U2_WORKFLOW_BASEDOC_WORKERS_H_
#define _U2_WORKFLOW_BASEDOC_WORKERS_H_

#include <U2Lang/LocalDomain.h>

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/U2OpStatus.h>

namespace U2 {

class Document;
class DocumentFormat;
class IOAdapter;

namespace LocalWorkflow {

class BaseDocReader : public BaseWorker {
    Q_OBJECT
public:
    BaseDocReader(Actor* a, const QString& tid, const DocumentFormatId& fid);
    virtual ~BaseDocReader() {}
    
    virtual void init() ;
    virtual Task* tick() ;
    virtual bool isDone() ;
    virtual void cleanup() ;
    
protected:
    virtual void doc2data(Document* ) = 0;
    
protected:
    CommunicationChannel* ch;
    DocumentFormatId fid;
    QMap<Document*, bool> docs;
    bool attachDoc2Proj;
    QList<Message> cache;
    DataTypePtr mtype;
    
};

class BaseDocWriter : public BaseWorker {
    Q_OBJECT
public:
    BaseDocWriter(Actor* a, const DocumentFormatId& fid);
    BaseDocWriter( Actor * a );
    virtual ~BaseDocWriter(){}
    virtual void init() ;
    virtual Task* tick() ;
    virtual void cleanup() ;
    virtual QStringList getOutputFiles();
protected:
    virtual void data2doc(Document*, const QVariantMap&) = 0;
    virtual void storeEntry(IOAdapter *, const QVariantMap &, int) {}
    Task* processDocs();
protected:
    CommunicationChannel *ch;
    DocumentFormat *format;
    bool append;
    QString url;
    QMap<QString, int> counter;
    uint fileMode;
    QStringList outputs;

    QMap<QString, Document*> docs;
    QMap<QString, IOAdapter*> adapters;
    QSet<QString> usedUrls;

    virtual void takeParameters(U2OpStatus &os);
    virtual QStringList takeUrlList(const QVariantMap &data, U2OpStatus &os);

private:
    void createAdaptersAndDocs(const QStringList &urls);
};

}// Workflow namespace
}// U2 namespace

#endif
