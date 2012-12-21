/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "CAP3Worker.h"
#include "TaskLocalStorage.h"
#include "CAP3Support.h"

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
#include <U2Core/Log.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/FailTask.h>

namespace U2 {
namespace LocalWorkflow {



 //////////////////////////////////////////////////////////////////////////
 //CAP3WorkerFactory
 //////////////////////////////////////////////////////////////////////////

const QString CAP3WorkerFactory::ACTOR_ID("CAP3");
const QString INPUT_FILE_PATH("input-path");

void CAP3WorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    Descriptor oud(BasePorts::OUT_MSA_PORT_ID(), CAP3Worker::tr("CAP3 result multiple sequence alignment"), 
        CAP3Worker::tr("The result of the CAP3 contig assembly."));
    
    QMap<Descriptor, DataTypePtr> outM;
    outM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("ace.out.msa", outM)), false /*input*/, true /*multi*/);
    
    Descriptor input(INPUT_FILE_PATH, CAP3Worker::tr("Input"),
                    CAP3Worker::tr("Path to input long DNA reads."));
    
    a << new Attribute(input, BaseTypes::STRING_TYPE(), true, QVariant("Default"));

    QString cap3Description = AppContext::getExternalToolRegistry()->getByName(CAP3_TOOL_NAME)->getDescription();
    Descriptor desc(ACTOR_ID, CAP3Worker::tr("Align with CAP3"), cap3Description);

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

    QMap<QString, PropertyDelegate*> delegates;
    delegates[INPUT_FILE_PATH] = new URLDelegate("", "inputPath", false);
    
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new CAP3Prompter());
    proto->setIconPath(":external_tool_support/images/clustalx.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ASSEMBLY(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new CAP3WorkerFactory());
}

//////////////////////////////////////////////////////////////////////////
// CAP3Prompter
//////////////////////////////////////////////////////////////////////////

CAP3Prompter::CAP3Prompter(Actor* p) : PrompterBase<CAP3Prompter>(p) {
}

QString CAP3Prompter::composeRichDoc() {
    QString inputPath = target->getParameter(INPUT_FILE_PATH)->getAttributeValueWithoutScript<QString>();
    QString doc = tr("Aligns long DNA reads from <u>%1</u> with \"<u>CAP3</u>\".")
        .arg(inputPath);

    return doc;
}
//////////////////////////////////////////////////////////////////////////
// CAP3Worker
//////////////////////////////////////////////////////////////////////////

CAP3Worker::CAP3Worker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {
}

void CAP3Worker::init() {
    input = ports.value(BasePorts::IN_MSA_PORT_ID());
    output = ports.value(BasePorts::OUT_MSA_PORT_ID());
}

Task* CAP3Worker::tick() {
    if (input->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(input);
        if (inputMessage.isEmpty()) {
            output->transit();
            return NULL;
        }
        cfg.inputFiles.append( actor->getParameter(INPUT_FILE_PATH)->getAttributeValue<QString>(context) );

        Task* t = new CAP3SupportTask(cfg);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    } else if (input->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void CAP3Worker::sl_taskFinished() {
    CAP3SupportTask* t = qobject_cast<CAP3SupportTask*>(sender());
    if (t->getState() != Task::State_Finished) return;

    MAlignment al = t->getResultAlignment()->getMAlignment();
    SAFE_POINT(NULL != output, "NULL output!", );
    SharedDbiDataHandler msaId = context->getDataStorage()->putAlignment(al);
    QVariantMap msgData;
    msgData[BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(msaId);
    output->put(Message(BaseTypes::MULTIPLE_ALIGNMENT_TYPE(), msgData));
    algoLog.info(tr("Aligned %1 with CAP3").arg(al.getName()));
}

void CAP3Worker::cleanup() {
}

} //namespace LocalWorkflow
} //namespace U2
