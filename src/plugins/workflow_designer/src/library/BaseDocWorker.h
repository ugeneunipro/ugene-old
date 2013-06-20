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

#ifndef _U2_WORKFLOW_BASEDOC_WORKERS_H_
#define _U2_WORKFLOW_BASEDOC_WORKERS_H_

#include <U2Lang/LocalDomain.h>

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/SaveDocumentTask.h>
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

    static QString getUniqueObjectName(const Document *doc, const QString &name);

protected:
    virtual void data2doc(Document*, const QVariantMap&) = 0;
    virtual bool hasDataToWrite(const QVariantMap &data) const = 0;
    virtual bool isStreamingSupport() const;
    virtual bool isSupportedSeveralMessages() const;
    virtual void storeEntry(IOAdapter *, const QVariantMap &, int) {}
    virtual Task * getWriteDocTask(Document *doc, const SaveDocFlags &flags);
    virtual void takeParameters(U2OpStatus &os);
    virtual QStringList takeUrlList(const QVariantMap &data, U2OpStatus &os);

protected:
    DocumentFormat *format;

private:
    CommunicationChannel *ch;
    bool append;
    uint fileMode;
    QSet<QString> usedUrls;
    QMap<QString, int> counters; // url <-> count suffix
    QMap<QString, IOAdapter*> adapters;
    QMap<IOAdapter*, Document*> docs;

private:
    bool ifCreateAdapter(const QString &url) const;
    /**
     * Creates an adapter for @url or returns existing one.
     * The url of the adapter could be not equal to @url.
     */
    IOAdapter * getAdapter(const QString &url, U2OpStatus &os);
    void openAdapter(IOAdapter *io, const QString &url, const SaveDocFlags &flags, U2OpStatus &os);
    /** Creates a document for @io or returns existing one. */
    Document * getDocument(IOAdapter *io, U2OpStatus &os);
    Task * processDocs();
    SaveDocFlags getDocFlags() const;
    void storeData(const QStringList &urls, const QVariantMap &data, U2OpStatus &os);
};

}// Workflow namespace
}// U2 namespace

#endif
