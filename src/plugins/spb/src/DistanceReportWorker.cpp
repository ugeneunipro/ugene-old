#include <U2Core/U2OpStatusUtils.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "SpbPlugin.h"

#include "DistanceReportWorker.h"

namespace SPB {

const QString DistanceReportWorkerFactory::ACTOR_ID("distance-report");

static const QString IN_PROFILES_PORT_ID("in-profiles");

static const QString MAIN_MSA_SLOT_ID("main-msa");
static const QString ALIGNED_MSA_SLOT_ID("aligned-msa");

static const QString GAPS_ATTR_ID("exclude-gaps");

/************************************************************************/
/* Task */
/************************************************************************/
DistanceReportWorker::DistanceReportWorker(Actor *a)
: BaseWorker(a), inPort(NULL), outPort(NULL)
{

}

void DistanceReportWorker::init() {
    inPort = ports[IN_PROFILES_PORT_ID];
    outPort = ports[BasePorts::OUT_TEXT_PORT_ID()];
}

Task * DistanceReportWorker::tick() {
    if (inPort->hasMessage()) {
        Message m = getMessageAndSetupScriptValues(inPort);
        QVariantMap data = m.getData().toMap();
        MAlignment mainMsa = data.value(MAIN_MSA_SLOT_ID).value<MAlignment>();
        MAlignment alignedMsa = data.value(ALIGNED_MSA_SLOT_ID).value<MAlignment>();
        bool excludeGaps = actor->getParameter(GAPS_ATTR_ID)->getAttributeValue<bool>(context);

        Task *t = new DistanceReportTask(mainMsa, alignedMsa, excludeGaps);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    } else if (inPort->isEnded()) {
        setDone();
        outPort->setEnded();
    }
    return NULL;
}

void DistanceReportWorker::cleanup() {

}

void DistanceReportWorker::sl_taskFinished() {
    DistanceReportTask *t = dynamic_cast<DistanceReportTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    QVariantMap data;
    data[BaseSlots::TEXT_SLOT().getId()] = t->getResult();
    outPort->put(Message(outPort->getBusType(), data));
}

/************************************************************************/
/* Task */
/************************************************************************/
DistanceReportTask::DistanceReportTask(const MAlignment &_mainMsa, MAlignment &_alignedMsa, bool _excludeGaps)
: Task("Distance report", TaskFlag_None), mainMsa(_mainMsa), alignedMsa(_alignedMsa), excludeGaps(_excludeGaps)
{

}

void DistanceReportTask::run() {
    if (mainMsa.getNumRows() > alignedMsa.getNumRows()) {
        setError(tr("The main alignment has more sequences than the alignment for report"));
        return;
    }
    for (int i=0; i<mainMsa.getNumRows(); i++) {
        const MAlignmentRow &row = alignedMsa.getRow(i);
        result += "," + row.getName();

        if (mainMsa.getRow(i).getName() != row.getName()) {
            setError(tr("The main alignment has different main sequence names than the alignment for report"));
            return;
        }
    }
    result += "\n";

    for (int i=mainMsa.getNumRows(); i<alignedMsa.getNumRows(); i++) {
        const MAlignmentRow &row = alignedMsa.getRow(i);
        addRowInfo(row);
    }
}

void DistanceReportTask::addRowInfo(const MAlignmentRow &alignedRow) {
    result += alignedRow.getName();

    U2OpStatus2Log os;
    QByteArray alignedArray = alignedRow.toByteArray(alignedMsa.getLength(), os);

    for (int i=0; i<mainMsa.getNumRows(); i++) {
        const MAlignmentRow &row = alignedMsa.getRow(i);

        int sim = getSimilarity(row.toByteArray(alignedMsa.getLength(), os), alignedArray);
        double simD = 100 * double(sim) / alignedMsa.getLength();
        result += "," + QByteArray::number(simD);
    }
    result += "\n";
}

int DistanceReportTask::getSimilarity(const QByteArray &row1, const QByteArray &row2) {
    int sim = 0;
    for (int k = 0; k < row1.length(); k++) {
        bool similar = (row1[k] == row2[k]);

        if (!excludeGaps) {
            if (similar) sim++;
        } else {
            if (similar && row1[k] != MAlignment_GapChar) sim++;
        }
    }
    return sim;
}

const QString & DistanceReportTask::getResult() {
    return result;
}

/************************************************************************/
/* Factory */
/************************************************************************/
void DistanceReportWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        Descriptor mainMsaD(MAIN_MSA_SLOT_ID,
            DistanceReportWorker::tr("Main alignment"),
            DistanceReportWorker::tr("Main alignment."));
        Descriptor alignedMsaD(ALIGNED_MSA_SLOT_ID,
            DistanceReportWorker::tr("Alignment for report"),
            DistanceReportWorker::tr("Alignment for report."));

        QMap<Descriptor, DataTypePtr> inMap;
        inMap[mainMsaD] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        inMap[alignedMsaD] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        portDescs << new PortDescriptor(IN_PROFILES_PORT_ID, DataTypePtr(new MapDataType("in.profiles", inMap)), true);

        QMap<Descriptor, DataTypePtr> outMap;
        outMap[BaseSlots::TEXT_SLOT()] = BaseTypes::STRING_TYPE();
        portDescs << new PortDescriptor(BasePorts::OUT_TEXT_PORT_ID(), DataTypePtr(new MapDataType("out.report", outMap)), false, true);
    }

    QList<Attribute*> attrs;
    {
        Descriptor gapsD(GAPS_ATTR_ID,
            DistanceReportWorker::tr("Exclude gaps"),
            DistanceReportWorker::tr("Exclude gaps."));
        attrs << new Attribute(gapsD, BaseTypes::BOOL_TYPE(), false, true);
    }

    Descriptor protoD(ACTOR_ID,
        DistanceReportWorker::tr("Generate Distance Report"),
        DistanceReportWorker::tr("Generate Distance Report."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoD, portDescs, attrs);
    proto->setEditor(new DelegateEditor(QMap<QString, PropertyDelegate*>()));
    proto->setPrompter(new DistanceReportPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(Constraints::WORKFLOW_CATEGORY, proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new DistanceReportWorkerFactory());
}

Worker * DistanceReportWorkerFactory::createWorker(Actor *a) {
    return new DistanceReportWorker(a);
}

/************************************************************************/
/* Prompter */
/************************************************************************/
QString DistanceReportPrompter::composeRichDoc() {
    return "Creates alignment distance report.";
}

} // SPB
