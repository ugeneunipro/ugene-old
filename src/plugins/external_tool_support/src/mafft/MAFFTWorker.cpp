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

#include "MAFFTWorker.h"
#include "TaskLocalStorage.h"
#include "MAFFTSupport.h"

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/Log.h>
#include <U2Core/FailTask.h>

namespace U2 {
namespace LocalWorkflow {

/****************************
 * MAFFTWorkerFactory
 ****************************/
const QString MAFFTWorkerFactory::ACTOR_ID("mafft");
const QString GAP_OPEN_PENALTY("gap-open-penalty");
const QString GAP_EXT_PENALTY("gap-ext-penalty");
const QString NUM_ITER("iterations-max-num");
const QString EXT_TOOL_PATH("path");
const QString TMP_DIR_PATH("temp-dir");

void MAFFTWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    Descriptor ind(BasePorts::IN_MSA_PORT_ID(), MAFFTWorker::tr("Input MSA"), 
        MAFFTWorker::tr("Multiple sequence alignment to be processed."));
    Descriptor oud(BasePorts::OUT_MSA_PORT_ID(), MAFFTWorker::tr("Multiple sequence alignment"), MAFFTWorker::tr("Result of alignment."));
    
    QMap<Descriptor, DataTypePtr> inM;
    inM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    p << new PortDescriptor(ind, DataTypePtr(new MapDataType("mafft.in.msa", inM)), true /*input*/);
    QMap<Descriptor, DataTypePtr> outM;
    outM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("mafft.out.msa", outM)), false /*input*/, true /*multi*/);
    
    Descriptor gop(GAP_OPEN_PENALTY, MAFFTWorker::tr("Gap Open Penalty"),
                   MAFFTWorker::tr("Gap Open Penalty"));
    Descriptor gep(GAP_EXT_PENALTY, MAFFTWorker::tr("Offset"),
                   MAFFTWorker::tr("Works like gap extension penalty"));
    Descriptor tgp(NUM_ITER, MAFFTWorker::tr("Max Iteration"),
                   MAFFTWorker::tr("Maximum number of iterative refinement"));
    Descriptor etp(EXT_TOOL_PATH, MAFFTWorker::tr("Tool Path"),
                   MAFFTWorker::tr("External tool path"));
    Descriptor tdp(TMP_DIR_PATH, MAFFTWorker::tr("Temporary directory"),
                   MAFFTWorker::tr("Directory for temporary files"));

    a << new Attribute(gop, BaseTypes::NUM_TYPE(), false, QVariant(1.53));
    a << new Attribute(gep, BaseTypes::NUM_TYPE(), false, QVariant(0.00));
    a << new Attribute(tgp, BaseTypes::NUM_TYPE(), false, QVariant(0));
    a << new Attribute(etp, BaseTypes::STRING_TYPE(), true, QVariant("default"));
    a << new Attribute(tdp, BaseTypes::STRING_TYPE(), true, QVariant("default"));

    Descriptor desc(ACTOR_ID, MAFFTWorker::tr("Align with MAFFT"),
        MAFFTWorker::tr("MAFFT is a multiple sequence alignment program for unix-like operating systems. "
                        "<p><dfn>It offers a range of multiple alignment methods, "
                        "L-INS-i (accurate; for alignment of &lt;&#126;200 sequences), "
                        "FFT-NS-2 (fast; for alignment of &lt;&#126;10,000 sequences), etc. </dfn></p>"));
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap m; m["minimum"] = double(.00); m["maximum"] = double(100.00); m["decimals"] = 2;
        delegates[GAP_OPEN_PENALTY] = new DoubleSpinBoxDelegate(m);
    }
    {
        QVariantMap m; m["minimum"] = double(.00); m["maximum"] = double(10.00); m["decimals"] = 2;
        delegates[GAP_EXT_PENALTY] = new DoubleSpinBoxDelegate(m);
    }
    {
        QVariantMap m; m["minimum"] = int(0); m["maximum"] = int(1000);
        delegates[NUM_ITER] = new SpinBoxDelegate(m);
    }
    delegates[EXT_TOOL_PATH] = new URLDelegate("", "executable", false);
    delegates[TMP_DIR_PATH] = new URLDelegate("", "TmpDir", false, true);

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new MAFFTPrompter());
    proto->setIconPath(":external_tool_support/images/cmdline.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ALIGNMENT(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new MAFFTWorkerFactory());
}

/****************************
* MAFFTPrompter
****************************/
MAFFTPrompter::MAFFTPrompter(Actor* p) : PrompterBase<MAFFTPrompter>(p) {
}
QString MAFFTPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_MSA_PORT_ID()));
    Actor* producer = input->getProducer(BasePorts::IN_MSA_PORT_ID());
    QString producerName = producer ? tr(" from %1").arg(producer->getLabel()) : "";

    QString doc = tr("Aligns each MSA supplied <u>%1</u> with <u>\"MAFFT\"</u>.")
        .arg(producerName);

    return doc;
}
/****************************
* MAFFTWorker
****************************/
MAFFTWorker::MAFFTWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {
}

void MAFFTWorker::init() {
    input = ports.value(BasePorts::IN_MSA_PORT_ID());
    output = ports.value(BasePorts::OUT_MSA_PORT_ID());
}

bool MAFFTWorker::isReady() {
    return (input && input->hasMessage());
}

Task* MAFFTWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(input);
    cfg.gapOpenPenalty=actor->getParameter(GAP_OPEN_PENALTY)->getAttributeValue<float>();
    cfg.gapExtenstionPenalty=actor->getParameter(GAP_EXT_PENALTY)->getAttributeValue<float>();
    cfg.maxNumberIterRefinement=actor->getParameter(NUM_ITER)->getAttributeValue<int>();
    QString path=actor->getParameter(EXT_TOOL_PATH)->getAttributeValue<QString>();
    if(QString::compare(path, "default", Qt::CaseInsensitive) != 0){
        AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->setPath(path);
    }
    path=actor->getParameter(TMP_DIR_PATH)->getAttributeValue<QString>();
    if(QString::compare(path, "default", Qt::CaseInsensitive) != 0){
        AppContext::getAppSettings()->getUserAppsSettings()->setUserTemporaryDirPath(path);
    }
    
    MAlignment msa = inputMessage.getData().toMap().value(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()).value<MAlignment>();
    if( msa.isEmpty() ) {
        return new FailTask(tr("An empty MSA has been supplied to MAFFT."));
    }
    Task* t = new MAFFTSupportTask(new MAlignmentObject(msa), cfg);
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    return t;
}

void MAFFTWorker::sl_taskFinished() {
    MAFFTSupportTask* t = qobject_cast<MAFFTSupportTask*>(sender());
    if (t->getState() != Task::State_Finished) return;
    QVariant v = qVariantFromValue<MAlignment>(t->resultMA);
    output->put(Message(BaseTypes::MULTIPLE_ALIGNMENT_TYPE(), v));
    if (input->isEnded()) {
        output->setEnded();
    }
    algoLog.info(tr("Aligned %1 with MAFFT").arg(t->resultMA.getName()));
}

bool MAFFTWorker::isDone() {
    return !input || input->isEnded();
}

void MAFFTWorker::cleanup() {
}

} //namespace LocalWorkflow
} //namespace U2
