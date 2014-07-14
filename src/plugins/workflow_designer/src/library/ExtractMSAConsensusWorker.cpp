/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>
#include <U2Algorithm/MSAConsensusUtils.h>
#include <U2Algorithm/BuiltInConsensusAlgorithms.h>

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/FailTask.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include <U2View/AssemblyModel.h>
#include <U2View/ExportConsensusTask.h>

#include "ExtractMSAConsensusWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString ExtractMSAConsensusWorkerFactory::ACTOR_ID("extract-msa-consensus");

namespace {
    const QString ALGO_ATTR_ID("algorithm");
    const QString THRESHOLD_ATTR_ID("threshold");
    const QString GAPS_ATTR_ID("keep-gaps");
}

ExtractMSAConsensusWorker::ExtractMSAConsensusWorker(Actor *actor)
: BaseWorker(actor)
{

}

void ExtractMSAConsensusWorker::init() {

}

Task * ExtractMSAConsensusWorker::tick() {
    if (hasMsa()) {
        U2OpStatusImpl os;
        MAlignment msa = takeMsa(os);
        CHECK_OP(os, new FailTask(os.getError()));

        return createTask(msa);
    } else {
        finish();
        return NULL;
    }
}

void ExtractMSAConsensusWorker::sl_taskFinished() {
    ExtractMSAConsensusTaskHelper *t = dynamic_cast<ExtractMSAConsensusTaskHelper*>(sender());
    CHECK(NULL != t, );
    CHECK(t->isFinished() && !t->hasError(), );
    CHECK(!t->isCanceled(), );

    sendResult(context->getDataStorage()->getDataHandler(t->getResult()));
}

void ExtractMSAConsensusWorker::cleanup() {

}

bool ExtractMSAConsensusWorker::hasMsa() const {
    const IntegralBus *port = ports[BasePorts::IN_MSA_PORT_ID()];
    SAFE_POINT(NULL != port, "NULL msa port", false);
    return port->hasMessage();
}

MAlignment ExtractMSAConsensusWorker::takeMsa(U2OpStatus &os) {
    const Message m = getMessageAndSetupScriptValues(ports[BasePorts::IN_MSA_PORT_ID()]);
    const QVariantMap data = m.getData().toMap();
    if (!data.contains(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId())) {
        os.setError(tr("Empty msa slot"));
        return MAlignment();
    }
    const SharedDbiDataHandler dbiId = data[BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()].value<SharedDbiDataHandler>();
    const MAlignmentObject *obj = StorageUtils::getMsaObject(context->getDataStorage(), dbiId);
    if (NULL == obj) {
        os.setError(tr("Error with msa object"));
        return MAlignment();
    }
    return obj->getMAlignment();
}

Task * ExtractMSAConsensusWorker::createTask(const MAlignment &msa) {
    const QString algoId = getValue<QString>(ALGO_ATTR_ID);
    const int threshold = getValue<int>(THRESHOLD_ATTR_ID);
    const bool keepGaps = getValue<bool>(GAPS_ATTR_ID);
    Task *t = new ExtractMSAConsensusTaskHelper(algoId, threshold, keepGaps, msa, context->getDataStorage()->getDbiRef());
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    return t;
}

void ExtractMSAConsensusWorker::finish() {
    IntegralBus *inPort = ports[BasePorts::IN_MSA_PORT_ID()];
    SAFE_POINT(NULL != inPort, "NULL msa port", );
    SAFE_POINT(inPort->isEnded(), "The msa is not ended", );
    IntegralBus *outPort = ports[BasePorts::OUT_SEQ_PORT_ID()];
    SAFE_POINT(NULL != outPort, "NULL sequence port", );

    outPort->setEnded();
    setDone();
}

void ExtractMSAConsensusWorker::sendResult(const SharedDbiDataHandler &seqId) {
    QVariantMap data;
    data[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(seqId);
    IntegralBus *outPort = ports[BasePorts::OUT_SEQ_PORT_ID()];
    SAFE_POINT(NULL != outPort, "NULL sequence port", );

    outPort->put(Message(outPort->getBusType(), data));
}

///////////////////////////////////////////////////////////////////////
//ExtractMSAConsensusTaskHelper
ExtractMSAConsensusTaskHelper::ExtractMSAConsensusTaskHelper(const QString &algoId, int threshold, bool keepGaps, const MAlignment &msa, const U2DbiRef &targetDbi)
: Task(tr("Extract consensus"), TaskFlags_NR_FOSCOE),
  algoId(algoId),
  threshold(threshold),
  keepGaps(keepGaps),
  msa(msa),
  targetDbi(targetDbi)
{

}

QString ExtractMSAConsensusTaskHelper::getResultName () const {
    QString res;
    res = msa.getName();
    res+="_consensus";
    return res;
}

void ExtractMSAConsensusTaskHelper::prepare() {
    QSharedPointer<MSAConsensusAlgorithm> algo (createAlgorithm());

    QByteArray cons;
    MSAConsensusUtils::updateConsensus(msa, cons, algo.data());
    if(!keepGaps){
        cons.replace("-", "");
    }

    U2SequenceImporter seqImporter;
    seqImporter.startSequence(targetDbi, U2ObjectDbi::ROOT_FOLDER, getResultName(), false, stateInfo);
    seqImporter.addBlock(cons.data(), cons.length(), stateInfo);
    resultSequence = seqImporter.finalizeSequence(stateInfo);
}

U2EntityRef ExtractMSAConsensusTaskHelper::getResult() const {
    const U2EntityRef ref(targetDbi, resultSequence.id);
    return ref;
}

MSAConsensusAlgorithm * ExtractMSAConsensusTaskHelper::createAlgorithm() {
    MSAConsensusAlgorithmRegistry *reg = AppContext::getMSAConsensusAlgorithmRegistry();
    SAFE_POINT_EXT(NULL != reg, setError("NULL registry"), NULL);

    MSAConsensusAlgorithmFactory *f = reg->getAlgorithmFactory(algoId);
    if (NULL == f) {
        setError(tr("Unknown consensus algorithm: ") + algoId);
        return NULL;
    }
    MSAConsensusAlgorithm* alg = f->createAlgorithm(msa);
    SAFE_POINT_EXT(NULL != alg, setError("NULL algorithm"), NULL);
    alg->setThreshold(threshold);

    return alg;
}

///////////////////////////////////////////////////////////////////////
//ExtractMSAConsensusWorkerFactory
ExtractMSAConsensusWorkerFactory::ExtractMSAConsensusWorkerFactory()
: DomainFactory(ACTOR_ID)
{

}

Worker * ExtractMSAConsensusWorkerFactory::createWorker(Actor *actor) {
    return new ExtractMSAConsensusWorker(actor);
}

void ExtractMSAConsensusWorkerFactory::init() {
    MSAConsensusAlgorithmRegistry *reg = AppContext::getMSAConsensusAlgorithmRegistry();
    SAFE_POINT(NULL != reg, "NULL registry", );

    const Descriptor desc(ACTOR_ID,
        QObject::tr("Extract Consensus from Alignment"),
        QObject::tr("Extract the consensus sequence from the incoming multiple sequence alignment."));

    QList<PortDescriptor*> ports;
    {
        QMap<Descriptor, DataTypePtr> inData;
        inData[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        DataTypePtr inType(new MapDataType(BasePorts::IN_MSA_PORT_ID(), inData));
        ports << new PortDescriptor(BasePorts::IN_MSA_PORT_ID(), inType, true);

        QMap<Descriptor, DataTypePtr> outData;
        outData[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        DataTypePtr outType(new MapDataType(BasePorts::OUT_SEQ_PORT_ID(), outData));
        ports << new PortDescriptor(BasePorts::OUT_SEQ_PORT_ID(), outType, false, true);
    }

    QList<Attribute*> attrs;
    {
        const Descriptor algoDesc(ALGO_ATTR_ID,
            QObject::tr("Algorithm"),
            QObject::tr("The algorithm of consensus extracting."));
        const Descriptor thresholdDesc(THRESHOLD_ATTR_ID,
            QObject::tr("Threshold"),
            QObject::tr("The threshold of the algorithm."));
        const Descriptor gapsDesc(GAPS_ATTR_ID,
            QObject::tr("Keep gaps"),
            QObject::tr("Set this parameter if the result consensus must keep the gaps."));

        attrs << new Attribute(algoDesc, BaseTypes::STRING_TYPE(), true, BuiltInConsensusAlgorithms::DEFAULT_ALGO);
        attrs << new Attribute(thresholdDesc, BaseTypes::NUM_TYPE(), true, 100);
        attrs << new Attribute(gapsDesc, BaseTypes::BOOL_TYPE(), true, true);
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap algos;
        foreach (const QString algoId, reg->getAlgorithmIds()) {
            MSAConsensusAlgorithmFactory *f = reg->getAlgorithmFactory(algoId);
            algos[f->getName()] = algoId;
        }
        delegates[ALGO_ATTR_ID] = new ComboBoxDelegate(algos);

        QVariantMap m; m["minimum"] = 0; m["maximum"] = 100;
        delegates[THRESHOLD_ATTR_ID] = new SpinBoxDelegate(m);
    }

    ActorPrototype *proto = new IntegralBusActorPrototype(desc, ports, attrs);
    proto->setPrompter(new ExtractMSAConsensusWorkerPrompter());
    proto->setEditor(new DelegateEditor(delegates));

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ALIGNMENT(), proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new ExtractMSAConsensusWorkerFactory());
}

///////////////////////////////////////////////////////////////////////
//ExtractMSAConsensusWorkerPrompter
ExtractMSAConsensusWorkerPrompter::ExtractMSAConsensusWorkerPrompter(Actor *actor)
: PrompterBase<ExtractMSAConsensusWorkerPrompter>(actor)
{

}

QString ExtractMSAConsensusWorkerPrompter::composeRichDoc() {
    QString algorithm = getParameter(ALGO_ATTR_ID).toString();
    QString link = getHyperlink(ALGO_ATTR_ID, algorithm);
    return tr("Extracts the consensus sequence from the incoming alignment(s) using the %1 algorithm.").arg(link);
}

} // LocalWorkflow
} // U2
