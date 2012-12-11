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

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "MuscleTask.h"

#include "ProfileToProfileWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString ProfileToProfileWorkerFactory::ACTOR_ID("align-profile-to-profile");

static const QString IN_PROFILES_PORT_ID("in-profiles");

static const QString MASTER_PROFILE_SLOT_ID("master-msa");
static const QString SECOND_PROFILE_SLOT_ID("second-msa");

/************************************************************************/
/* Worker */
/************************************************************************/
ProfileToProfileWorker::ProfileToProfileWorker(Actor *a)
: BaseWorker(a), inPort(NULL), outPort(NULL)
{

}

void ProfileToProfileWorker::init() {
    inPort = ports[IN_PROFILES_PORT_ID];
    outPort = ports[BasePorts::OUT_MSA_PORT_ID()];
}

Task * ProfileToProfileWorker::tick() {
    if (inPort->hasMessage()) {
        Message m = getMessageAndSetupScriptValues(inPort);
        QVariantMap data = m.getData().toMap();
        MAlignment masterMsa = data.value(MASTER_PROFILE_SLOT_ID).value<MAlignment>();
        MAlignment secondMsa = data.value(SECOND_PROFILE_SLOT_ID).value<MAlignment>();

        Task *t = new ProfileToProfileTask(masterMsa, secondMsa);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    } else if (inPort->isEnded()) {
        setDone();
        outPort->setEnded();
    }
    return NULL;
}

void ProfileToProfileWorker::cleanup() {
    foreach (MAlignmentObject *obj, objects) {
        delete obj;
    }
    objects.clear();
}

void ProfileToProfileWorker::sl_taskFinished() {
    ProfileToProfileTask *t = dynamic_cast<ProfileToProfileTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    QVariantMap data;
    data[BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()] = qVariantFromValue<MAlignment>(t->getResult());
    outPort->put(Message(outPort->getBusType(), data));
}

/************************************************************************/
/* Task */
/************************************************************************/
ProfileToProfileTask::ProfileToProfileTask(const MAlignment &_masterMsa, MAlignment &_secondMsa)
: Task("Align profile to profile with MUSCLE", TaskFlag_NoRun), masterMsa(_masterMsa), secondMsa(_secondMsa),
seqIdx(0), subtaskCount(0)
{

}

ProfileToProfileTask::~ProfileToProfileTask() {

}

void ProfileToProfileTask::prepare() {
    int maxThreads = 1;//AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
    setMaxParallelSubtasks(maxThreads);

    U2OpStatus2Log os;
    foreach (const MAlignmentRow &row, masterMsa.getRows()) {
        result.addRow(row, os);
    }

    QList<Task*> tasks = createAlignTasks();
    foreach (Task *t, tasks) {
        addSubTask(t);
    }
}

QList<Task*> ProfileToProfileTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> tasks;
    if (subTask->hasError()) {
        return tasks;
    }

    appendResult(subTask);
    tasks << createAlignTasks();

    return tasks;
}

const MAlignment & ProfileToProfileTask::getResult() {
    U2AlphabetUtils::assignAlphabet(result);
    return result;
}

void ProfileToProfileTask::appendResult(Task *task) {
    subtaskCount--;
    MuscleTask *t = dynamic_cast<MuscleTask*>(task);
    SAFE_POINT(NULL != t, "NULL Muscle task!",);

    const QList<MAlignmentRow> &newRows = t->resultMA.getRows();
    if (newRows.size() == masterMsa.getRows().size() + 1) {
        U2OpStatus2Log os;
        result.addRow(newRows.last(), os);
    }
}

QList<Task*> ProfileToProfileTask::createAlignTasks() {
    QList<Task*> tasks;
    while (canCreateTask()) {
        U2OpStatus2Log os;
        MuscleTaskSettings cfg;
        cfg.op = MuscleTaskOp_ProfileToProfile;
        cfg.profile.addRow(secondMsa.getRow(seqIdx), os);
        cfg.profile.setAlphabet(secondMsa.getAlphabet());

        tasks << new MuscleTask(masterMsa, cfg);

        subtaskCount++;
        seqIdx++;
    }

    return tasks;
}

bool ProfileToProfileTask::canCreateTask() const {
    return (seqIdx < secondMsa.getRows().size())
        && (subtaskCount < getMaxParallelSubtasks());
}

/************************************************************************/
/* Factory */
/************************************************************************/
void ProfileToProfileWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        Descriptor masterProfileD(MASTER_PROFILE_SLOT_ID,
            ProfileToProfileWorker::tr("Master profile"),
            ProfileToProfileWorker::tr("The main alignment which will be aligned on."));
        Descriptor secondProfileD(SECOND_PROFILE_SLOT_ID,
            ProfileToProfileWorker::tr("Second profile"),
            ProfileToProfileWorker::tr("Alignment which will be aligned to the master alignment."));

        QMap<Descriptor, DataTypePtr> inMap;
        inMap[masterProfileD] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        inMap[secondProfileD] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        portDescs << new PortDescriptor(IN_PROFILES_PORT_ID, DataTypePtr(new MapDataType("in.profiles", inMap)), true);

        QMap<Descriptor, DataTypePtr> outMap;
        outMap[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        portDescs << new PortDescriptor(BasePorts::OUT_MSA_PORT_ID(), DataTypePtr(new MapDataType("out.msa", outMap)), false, true);
    }

    Descriptor protoD(ACTOR_ID,
        ProfileToProfileWorker::tr("Align Profile to Profile With MUSCLE"),
        ProfileToProfileWorker::tr("Aligns second profile to master profile with MUSCLE aligner."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoD, portDescs, QList<Attribute*>());
    proto->setEditor(new DelegateEditor(QMap<QString, PropertyDelegate*>()));
    proto->setPrompter(new ProfileToProfilePrompter());
    proto->setIconPath(":umuscle/images/muscle_16.png");

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ALIGNMENT(), proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new ProfileToProfileWorkerFactory());
}

Worker * ProfileToProfileWorkerFactory::createWorker(Actor *a) {
    return new ProfileToProfileWorker(a);
}

/************************************************************************/
/* Prompter */
/************************************************************************/
QString ProfileToProfilePrompter::composeRichDoc() {
    return "Aligns second profile to master profile with MUSCLE aligner";
}

} // LocalWorkflow
} // U2
