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

#ifndef _U2_WORKFLOW_DOC_WORKERS_H_
#define _U2_WORKFLOW_DOC_WORKERS_H_

#include "BaseDocWorker.h"
#include "CoreLib.h"
#include <U2Lang/CoreLibConstants.h>
#include <U2Core/IOAdapter.h>


namespace U2 {
namespace LocalWorkflow {

class TextReader : public BaseDocReader {
    Q_OBJECT
public:
    TextReader(Actor* a) : BaseDocReader(a, CoreLibConstants::TEXT_TYPESET_ID, BaseDocumentFormats::PLAIN_TEXT), io(NULL), done(false){}
    void init();
    bool isDone();
    bool isReady();
    Task *tick();
protected:
    virtual void doc2data(Document* doc);
private:
    IOAdapter *io;
    QStringList urls;
    QString url;
    bool done;
};

class TextWriter : public BaseDocWriter {
    Q_OBJECT
public:
    TextWriter(Actor* a) : BaseDocWriter(a, BaseDocumentFormats::PLAIN_TEXT){}
protected:
    virtual void data2doc(Document*, const QVariantMap&);
};

class FastaWriter : public BaseDocWriter {
    Q_OBJECT
public:
    FastaWriter(Actor* a) : BaseDocWriter(a, BaseDocumentFormats::PLAIN_FASTA){}
protected:
    virtual void data2doc(Document*, const QVariantMap&);
public:
    static void data2document(Document*, const QVariantMap&);
};

class GenbankWriter : public BaseDocWriter {
    Q_OBJECT
public:
    GenbankWriter(Actor* a) : BaseDocWriter(a, BaseDocumentFormats::PLAIN_GENBANK){}
protected:
    virtual void data2doc(Document*, const QVariantMap&);
public:
    static void data2document(Document*, const QVariantMap&);
};

class FastQWriter : public BaseDocWriter {
    Q_OBJECT
public:
    FastQWriter(Actor* a) : BaseDocWriter(a, BaseDocumentFormats::FASTQ){}
protected:
    virtual void data2doc(Document*, const QVariantMap&);
public:
    static void data2document(Document*, const QVariantMap&);
};

class RawSeqWriter : public BaseDocWriter {
    Q_OBJECT
public:
    RawSeqWriter(Actor* a) : BaseDocWriter(a, BaseDocumentFormats::RAW_DNA_SEQUENCE){}
protected:
    virtual void data2doc(Document*, const QVariantMap&);
public:
    static void data2document(Document*, const QVariantMap&);
};


// generic sequence writer
class SeqWriter : public BaseDocWriter {
    Q_OBJECT
public:
    SeqWriter( Actor * a ) : BaseDocWriter(a) {}
protected:
    virtual void data2doc(Document*, const QVariantMap&);
};

class MSAWriter : public BaseDocWriter {
    Q_OBJECT
public:
    MSAWriter(Actor* a, const DocumentFormatId& fid) : BaseDocWriter(a, fid){}
    MSAWriter(Actor * a) : BaseDocWriter(a){}
protected:
    virtual void data2doc(Document*, const QVariantMap&);
};

class DataWorkerFactory : public DomainFactory {
public:
    DataWorkerFactory(const Descriptor& d) : DomainFactory(d) {}
    virtual ~DataWorkerFactory() {}
    virtual Worker* createWorker(Actor*);
    static void init();
};

} // Workflow namespace
} // U2 namespace

#endif

