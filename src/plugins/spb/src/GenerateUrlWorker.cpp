#include <U2Core/FailTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "SpbPlugin.h"

#include "GenerateUrlWorker.h"

namespace SPB {

const QString GenerateUrlWorkerFactory::ACTOR_ID("generate-url");

static const QString DIR_ATTR_ID("direcory");
static const QString SUFFIX_ATTR_ID("suffix");

GenerateUrlWorker::GenerateUrlWorker(Actor *a)
: BaseWorker(a), inPort(NULL), outPort(NULL)
{
}

void GenerateUrlWorker::init() {
    inPort = ports.value(BasePorts::IN_TEXT_PORT_ID());
    outPort = ports.value(BasePorts::OUT_TEXT_PORT_ID());
}

Task * GenerateUrlWorker::tick() {
    while (inPort->hasMessage()) {
        U2OpStatusImpl os;
        QString inUrl = getInUrl(os);
        CHECK_OP(os, new FailTask(os.getError()));

        sendUrl(generateUrl(inUrl));
    }
    if (inPort->isEnded()) {
        setDone();
        outPort->setEnded();
    }
    return NULL;
}

QString GenerateUrlWorker::getInUrl(U2OpStatus &os) {
    Message m = getMessageAndSetupScriptValues(inPort);
    QVariantMap data = m.getData().toMap();
    if (!data.contains(BaseSlots::URL_SLOT().getId())) {
        os.setError("No input url");
        return "";
    }

    return data[BaseSlots::URL_SLOT().getId()].toString();
}

QString GenerateUrlWorker::generateUrl(const QString &inUrl) {
    QString dirPath = actor->getParameter(DIR_ATTR_ID)->getAttributeValue<QString>(context);
    QString suffix = actor->getParameter(SUFFIX_ATTR_ID)->getAttributeValue<QString>(context);

    if (dirPath.isEmpty() && suffix.isEmpty()) {
        return inUrl;
    }

    GUrl inGUrl(inUrl);
    QString result;
    if (!dirPath.isEmpty()) {
        QDir dir(dirPath);
        dir.mkpath(dirPath);
        result = dir.absolutePath();
    } else {
        result = inGUrl.dirPath();
    }
    if (!result.endsWith("/")) {
        result += "/";
    }

    if (!suffix.isEmpty()) {
        result += inGUrl.baseFileName() + suffix +
                "." + inGUrl.completeFileSuffix();
    } else {
        result += inGUrl.fileName();
    }

    return result;
}

void GenerateUrlWorker::sendUrl(const QString &url) {
    QVariantMap data;
    data[BaseSlots::URL_SLOT().getId()] = url;
    outPort->put(Message(outPort->getBusType(), data));
}

void GenerateUrlWorker::cleanup() {

}

void GenerateUrlWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> typeMap;
        typeMap[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        DataTypePtr type(new MapDataType("url.type", typeMap));
        portDescs << new PortDescriptor(BasePorts::IN_TEXT_PORT_ID(),
            type, true /*input*/);
        portDescs << new PortDescriptor(BasePorts::OUT_TEXT_PORT_ID(),
            type, false /*input*/, true /*multy*/);
    }

    QList<Attribute*> attrs;
    {
        Descriptor dirD(DIR_ATTR_ID,
            GenerateUrlWorker::tr("Directory"),
            GenerateUrlWorker::tr("Directory"));
        Descriptor suffixD(SUFFIX_ATTR_ID,
            GenerateUrlWorker::tr("Suffix"),
            GenerateUrlWorker::tr("Suffix"));

        attrs << new Attribute(dirD, BaseTypes::STRING_TYPE());
        attrs << new Attribute(suffixD, BaseTypes::STRING_TYPE());
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        delegates[DIR_ATTR_ID] = new URLDelegate(QString(), QString(), false, true);
    }

    Descriptor protoD(ACTOR_ID,
        GenerateUrlWorker::tr("Generate URL"),
        GenerateUrlWorker::tr("Generate URL."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoD, portDescs, attrs);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new GenerateUrlPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(Constraints::WORKFLOW_CATEGORY, proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new GenerateUrlWorkerFactory());
}

Worker * GenerateUrlWorkerFactory::createWorker(Actor *a) {
    return new GenerateUrlWorker(a);
}

QString GenerateUrlPrompter::composeRichDoc() {
    return "";
}

} // SPB
