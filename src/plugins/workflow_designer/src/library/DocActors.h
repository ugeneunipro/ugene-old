#ifndef _U2_WORKFLOW_DOC_ACTORS_H_
#define _U2_WORKFLOW_DOC_ACTORS_H_

#include <U2Core/global.h>

#include <U2Lang/Attribute.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Core/BaseDocumentFormats.h>

namespace U2 {
namespace Workflow {

class DocActorProto : public IntegralBusActorPrototype {
public:
    DocActorProto(const DocumentFormatId& _fid, const Descriptor& desc, const QList<PortDescriptor*>& ports, 
        const QList<Attribute*>& attrs = QList<Attribute*>());
    DocActorProto(const Descriptor& desc, const GObjectType& t, const QList<PortDescriptor*>& ports, 
        const QList<Attribute*>& attrs = QList<Attribute*>());
    
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const = 0;
    bool isAcceptableDrop(const QMimeData*, QVariantMap*, const QString & urlAttrId ) const;
protected:
    QString prepareDocumentFilter();
    
protected:
    DocumentFormatId fid;
    GObjectType type;
};

class ReadDocActorProto : public DocActorProto {
public:
    ReadDocActorProto( const DocumentFormatId& fid, const Descriptor& desc, const QList<PortDescriptor*>& ports, 
        const QList<Attribute*>& attrs = QList<Attribute*>() );
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const;
};

class WriteDocActorProto : public DocActorProto {
public:
    WriteDocActorProto( const DocumentFormatId& fid, const Descriptor& desc, const QList<PortDescriptor*>& ports, 
        const QString & portId, const QList<Attribute*>& attrs = QList<Attribute*>() );
    WriteDocActorProto(const Descriptor& desc, const GObjectType & t, const QList<PortDescriptor*>& ports, 
        const QString & portId, const QList<Attribute*>& attrs = QList<Attribute*>() );

private:
    void construct();
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const;

private:
    QString outPortId;
};

class ReadDocPrompter;
typedef PrompterBase<ReadDocPrompter> ReadDocPrompterBase;

class ReadDocPrompter : public ReadDocPrompterBase {
    Q_OBJECT
public:
    ReadDocPrompter(const QString& s) : spec(s) {}
    ReadDocPrompter(Actor* p = 0) : ReadDocPrompterBase(p) {}
    virtual ActorDocument* createDescription(Actor* a) {
        ReadDocPrompter* doc = static_cast<ReadDocPrompter*>(ReadDocPrompterBase::createDescription(a));
        doc->spec = this->spec;
        return doc;
    }
protected:
    QString composeRichDoc();
    QString spec;
};

class WriteDocPrompter;
typedef PrompterBase<WriteDocPrompter> WriteDocPrompterBase;

class WriteDocPrompter : public WriteDocPrompterBase {
    Q_OBJECT
public:
    WriteDocPrompter(const QString& spec, const QString& slot) : spec(spec), slot(slot){}
    WriteDocPrompter(Actor* p = 0) : WriteDocPrompterBase(p) {}
    virtual ActorDocument* createDescription(Actor* a) {
        WriteDocPrompter* doc = static_cast<WriteDocPrompter*>(WriteDocPrompterBase::createDescription(a));
        doc->spec = this->spec;
        doc->slot = this->slot;
        return doc;
    }
protected:
    QString composeRichDoc();
    QString spec,slot;
};

class WriteGenbankPrompter : public PrompterBase<WriteGenbankPrompter> {
    Q_OBJECT
public:
    WriteGenbankPrompter(Actor* p = 0) : PrompterBase<WriteGenbankPrompter>(p){}

protected:
    QString composeRichDoc();
};

class WriteFastaPrompter : public PrompterBaseImpl {
    Q_OBJECT
public:
    WriteFastaPrompter(const QString & formatId, Actor* p = 0) : PrompterBaseImpl(p), format(formatId){}
    
    virtual QString composeRichDoc();
    virtual ActorDocument * createDescription(Actor*);
    
private:
    QString format;
};

} // Workflow namespace
} // U2 namespace

#endif
