#ifndef _U2_WORKFLOW_SEQ_ACTOR_H_
#define _U2_WORKFLOW_SEQ_ACTOR_H_

#include <U2Lang/Attribute.h>
#include <U2Lang/IntegralBusModel.h>

#include <U2Lang/WorkflowUtils.h>

namespace U2 {
namespace Workflow {

class GenericMAActorProto : public IntegralBusActorPrototype {
public:
    static const QString TYPE;
    GenericMAActorProto();
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const;
};

class GenericSeqActorProto : public IntegralBusActorPrototype {
public:
    enum Mode{
        SPLIT,
        MERGE,
    };

    static const QString MODE_ATTR;
    static const QString GAP_ATTR;
    static const QString ACC_ATTR;
    static const QString TYPE;

    GenericSeqActorProto();
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const;
};

class SeqReadPrompter;
typedef PrompterBase<SeqReadPrompter> SeqReadPrompterBase;

class SeqReadPrompter : public SeqReadPrompterBase {
    Q_OBJECT
public:
    SeqReadPrompter(Actor* p = 0) : SeqReadPrompterBase(p) {}
protected:
    QString composeRichDoc();
};

}//namespace Workflow
}//namespace U2
#endif
