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

