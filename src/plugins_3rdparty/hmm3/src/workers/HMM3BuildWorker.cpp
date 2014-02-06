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

#include "HMM3BuildWorker.h"
#include "HMM3IOWorker.h"
#include "build/uHMM3BuildTask.h"

#include <U2Lang/Datatype.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>

#include <U2Core/MAlignment.h>
#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/FailTask.h>

/* TRANSLATOR U2::LocalWorkflow::HMM3BuildWorker */

namespace U2 {
namespace LocalWorkflow {

/******************************
 * HMM3BuildWorkerFactory
 ******************************/
const QString HMM3BuildWorkerFactory::ACTOR("hmm3-build");
static const QString OUT_HMM3_PORT_ID("out-hmm3");

static const QString SEED_ATTR("seed");

static const QString HMM3_PROFILE_DEFAULT_NAME("hmm3_profile");

void HMM3BuildWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    {
        Descriptor id(BasePorts::IN_MSA_PORT_ID(), HMM3BuildWorker::tr("Input MSA"), 
            HMM3BuildWorker::tr("Input multiple sequence alignment for building statistical model."));
        Descriptor od(OUT_HMM3_PORT_ID, HMM3BuildWorker::tr("HMM3 profile"), HMM3BuildWorker::tr("Produced HMM3 profile"));
        
        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        p << new PortDescriptor(id, DataTypePtr(new MapDataType("hmm3.build.in", inM)), true /*input*/);
        QMap<Descriptor, DataTypePtr> outM;
        outM[HMM3Lib::HMM3_SLOT] = HMM3Lib::HMM3_PROFILE_TYPE();
        p << new PortDescriptor(od, DataTypePtr(new MapDataType("hmm3.build", outM)), false /*input*/, true /*multi*/);
    }

    Descriptor sed(SEED_ATTR, HMM3BuildWorker::tr("Random seed"), HMM3BuildWorker::tr("Random generator seed. 0 - means that one-time arbitrary seed will be used."));

    a << new Attribute(sed, BaseTypes::NUM_TYPE(), false, QVariant(0));

    Descriptor desc(HMM3BuildWorkerFactory::ACTOR, HMM3BuildWorker::tr("HMM3 Build"), HMM3BuildWorker::tr("Builds a HMM3 profile from a multiple sequence alignment."
        "<p>The HMM3 profile is a statistical model which captures position-specific information"
        " about how conserved each column of the alignment is, and which residues are likely."));
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    QMap<QString, PropertyDelegate*> delegates;    
    
    {
        QVariantMap m; m["minimum"] = 0; m["maximum"] = INT_MAX;
        delegates[SEED_ATTR] = new SpinBoxDelegate(m);
    }
    proto->setEditor(new DelegateEditor(delegates));
    proto->setIconPath( ":/hmm3/images/hmmer_16.png" );
    proto->setPrompter(new HMM3BuildPrompter());
    WorkflowEnv::getProtoRegistry()->registerProto(HMM3Lib::HMM3_CATEGORY(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new HMM3BuildWorkerFactory());
}

void HMM3BuildWorkerFactory::cleanup() {
    delete WorkflowEnv::getProtoRegistry()->unregisterProto(ACTOR);
    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    delete localDomain->unregisterEntry(ACTOR);
}

/******************************
 * HMM3BuildPrompter
 ******************************/
QString HMM3BuildPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_MSA_PORT_ID()));
    Actor* msaProducer = input->getProducer(BasePorts::IN_MSA_PORT_ID());

    QString msaName = msaProducer ? tr("For each MSA from <u>%1</u>,").arg(msaProducer->getLabel()) : "";

    QString doc = tr("%1 builds a HMM3 profile.")
        .arg(msaName);

    return doc;
}

/******************************
* HMM3BuildWorker
******************************/
HMM3BuildWorker::HMM3BuildWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL){
}

void HMM3BuildWorker::init() {
    input = ports.value(BasePorts::IN_MSA_PORT_ID());
    output = ports.value(OUT_HMM3_PORT_ID);
    setDefaultUHMM3BuildSettings(&cfg);
}

bool HMM3BuildWorker::isReady() {
    if (isDone()) {
        return false;
    }
    return input->hasMessage() || input->isEnded();
}

Task* HMM3BuildWorker::tick() {
    if (input->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(input);
        if (inputMessage.isEmpty()) {
            output->transit();
            return NULL;
        }
        cfg.seed = actor->getParameter(SEED_ATTR)->getAttributeValue<int>(context);

        QVariantMap qm = inputMessage.getData().toMap();
        SharedDbiDataHandler msaId = qm.value(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()).value<SharedDbiDataHandler>();
        std::auto_ptr<MAlignmentObject> msaObj(StorageUtils::getMsaObject(context->getDataStorage(), msaId));
        SAFE_POINT(NULL != msaObj.get(), "NULL MSA Object!", NULL);
        MAlignment msa = msaObj->getMAlignment();
        
        Task* t = new UHMM3BuildTask(cfg, msa);
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
        return t;
    } else if (input->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void HMM3BuildWorker::sl_taskFinished() {
    Task * t = qobject_cast<Task*>(sender());
    SAFE_POINT( NULL != t, "Invalid task is encountered", );
    if ( t->isCanceled( ) ) {
        return;
    }
    if( t->getState() != Task::State_Finished ) {
        return;
    }
    sl_taskFinished(t);
}

void HMM3BuildWorker::sl_taskFinished(Task* t) {
    UHMM3BuildTask* build = qobject_cast<UHMM3BuildTask*>(t);
    SAFE_POINT( NULL != t, "Invalid task is encountered", );
    if ( t->isCanceled( ) ) {
        return;
    }
    const P7_HMM* hmm = NULL;
    hmm = build->takeHMM();
    output->put(Message(HMM3Lib::HMM3_PROFILE_TYPE(), qVariantFromValue<const P7_HMM*>(hmm)));
    algoLog.info(tr("Built HMM3 profile"));
}

bool HMM3BuildWorker::isDone() {
    return BaseWorker::isDone();
}

void HMM3BuildWorker::cleanup() {
}

} //namespace LocalWorkflow
} //namespace U2
