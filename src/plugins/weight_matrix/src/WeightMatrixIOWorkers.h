#ifndef _U2_WEIGHT_MATRIX_IO_WORKER_H_
#define _U2_WEIGHT_MATRIX_IO_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include <U2Core/SaveDocumentTask.h>

#include "WeightMatrixAlgorithm.h"

Q_DECLARE_METATYPE(U2::PWMatrix)
Q_DECLARE_METATYPE(U2::PFMatrix)

namespace U2 {
namespace LocalWorkflow {
    
class PWMatrixIOProto : public IntegralBusActorPrototype {
public:
    PWMatrixIOProto(const Descriptor& desc, const QList<PortDescriptor*>& ports, 
        const QList<Attribute*>& attrs = QList<Attribute*>());
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const = 0;
    bool isAcceptableDrop(const QMimeData*, QVariantMap*, const QString & urlAttrId ) const;
};

class ReadPWMatrixProto : public PWMatrixIOProto {
public:
    ReadPWMatrixProto(const Descriptor& desc, const QList<PortDescriptor*>& ports, 
        const QList<Attribute*>& attrs = QList<Attribute*>());
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const;
};

class WritePWMatrixProto : public PWMatrixIOProto {
public:
    WritePWMatrixProto(const Descriptor& desc, const QList<PortDescriptor*>& ports, 
        const QList<Attribute*>& attrs = QList<Attribute*>());
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const;
};

class PWMatrixReadPrompter : public PrompterBase<PWMatrixReadPrompter> {
    Q_OBJECT
public:
    PWMatrixReadPrompter(Actor* p = 0) : PrompterBase<PWMatrixReadPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class PWMatrixWritePrompter : public PrompterBase<PWMatrixWritePrompter> {
    Q_OBJECT
public:
    PWMatrixWritePrompter(Actor* p = 0) : PrompterBase<PWMatrixWritePrompter>(p) {}
protected:
    QString composeRichDoc();
};


class PWMatrixReader : public BaseWorker {
    Q_OBJECT
public:
    static const QString ACTOR_ID;
    PWMatrixReader(Actor* a) : BaseWorker(a), output(NULL) {}
    virtual void init() ;
    virtual bool isReady() {return !urls.isEmpty();}
    virtual Task* tick() ;
    virtual bool isDone() {return urls.isEmpty();}
    virtual void cleanup() {}
    private slots:
        void sl_taskFinished();

protected:
    CommunicationChannel *output;
    QStringList urls;
    QList<Task*> tasks;
    DataTypePtr mtype;
}; 

class PWMatrixWriter : public BaseWorker {
    Q_OBJECT
public:
    static const QString ACTOR_ID;
    PWMatrixWriter(Actor* a) : BaseWorker(a), input(NULL), done(false), fileMode(SaveDoc_Overwrite) {}
    virtual void init() ;
    virtual bool isReady() {return input && input->hasMessage();}
    virtual Task* tick() ;
    virtual bool isDone() {return !input || input->isEnded();}
    virtual void cleanup() {}

protected:
    CommunicationChannel *input;
    QString url;
    QMap<QString,int> counter;
    bool done;
    uint fileMode;
}; 

//////////////////////////////////////////////////////////////////////////
// PFMatrix
//////////////////////////////////////////////////////////////////////////

class PFMatrixIOProto : public IntegralBusActorPrototype {
public:
    PFMatrixIOProto(const Descriptor& desc, const QList<PortDescriptor*>& ports, 
        const QList<Attribute*>& attrs = QList<Attribute*>());
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const = 0;
    bool isAcceptableDrop(const QMimeData*, QVariantMap*, const QString & urlAttrId ) const;
};

class ReadPFMatrixProto : public PFMatrixIOProto {
public:
    ReadPFMatrixProto(const Descriptor& desc, const QList<PortDescriptor*>& ports, 
        const QList<Attribute*>& attrs = QList<Attribute*>());
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const;
};

class WritePFMatrixProto : public PFMatrixIOProto {
public:
    WritePFMatrixProto(const Descriptor& desc, const QList<PortDescriptor*>& ports, 
        const QList<Attribute*>& attrs = QList<Attribute*>());
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const;
};

class PFMatrixReadPrompter : public PrompterBase<PFMatrixReadPrompter> {
    Q_OBJECT
public:
    PFMatrixReadPrompter(Actor* p = 0) : PrompterBase<PFMatrixReadPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class PFMatrixWritePrompter : public PrompterBase<PFMatrixWritePrompter> {
    Q_OBJECT
public:
    PFMatrixWritePrompter(Actor* p = 0) : PrompterBase<PFMatrixWritePrompter>(p) {}
protected:
    QString composeRichDoc();
};


class PFMatrixReader : public BaseWorker {
    Q_OBJECT
public:
    static const QString ACTOR_ID;
    PFMatrixReader(Actor* a) : BaseWorker(a), output(NULL) {}
    virtual void init() ;
    virtual bool isReady() {return !urls.isEmpty();}
    virtual Task* tick() ;
    virtual bool isDone() {return urls.isEmpty();}
    virtual void cleanup() {}
    private slots:
        void sl_taskFinished();

protected:
    CommunicationChannel *output;
    QStringList urls;
    QList<Task*> tasks;
    DataTypePtr mtype;
}; 

class PFMatrixWriter : public BaseWorker {
    Q_OBJECT
public:
    static const QString ACTOR_ID;
    PFMatrixWriter(Actor* a) : BaseWorker(a), input(NULL), done(false), fileMode(SaveDoc_Overwrite) {}
    virtual void init() ;
    virtual bool isReady() {return input && input->hasMessage();}
    virtual Task* tick() ;
    virtual bool isDone() {return !input || input->isEnded();}
    virtual void cleanup() {}

protected:
    CommunicationChannel *input;
    QString url;
    QMap<QString,int> counter;
    bool done;
    uint fileMode;
}; 

} // Workflow namespace
} // U2 namespace

#endif
