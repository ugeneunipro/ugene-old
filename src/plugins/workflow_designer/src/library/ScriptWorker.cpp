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

#include "ScriptWorker.h"

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/IntegralBusType.h>
#include <U2Lang/ScriptLibrary.h>
#include <U2Lang/IncludedProtoFactory.h>

#include <U2Core/DNASequence.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/FailTask.h>


namespace U2 {
namespace LocalWorkflow {

const QString ScriptWorkerFactory::ACTOR_ID("Script-");

const static QString INPUT_PORT_TYPE("input-for-");
const static QString OUTPUT_PORT_TYPE("output-for-");

static const QString IN_PORT_ID("in");
static const QString OUT_PORT_ID("out");

void ScriptWorkerTask::run() {
    //QScriptEngine engine;
    QMap<QString, QScriptValue> scriptVars;
    foreach( const Descriptor & key, script->getScriptVars().uniqueKeys() ) {
        assert(!key.getId().isEmpty());
        if(!(script->getScriptVars().value(key)).isNull()) {
            scriptVars[key.getId()] = engine->newVariant(script->getScriptVars().value(key));
        }
        else {
            scriptVars[key.getId()] = engine->newVariant(engine->globalObject().property(key.getId().toAscii().data()).toVariant());
        }
    }

    WorkflowScriptLibrary::initEngine(engine);
    QScriptValue scriptResultValue = ScriptTask::runScript(engine, scriptVars, script->getScriptText(), stateInfo);
    if(engine->hasUncaughtException()) {
		scriptResultValue = engine->uncaughtException();
        QString message = scriptResultValue.toString();
        stateInfo.setError(tr("Error in line ") + QString::number(engine->uncaughtExceptionLineNumber()) + ":" + message.split(":").last());
    }
	result = scriptResultValue.toVariant();

    if(engine->globalObject().property("list").toBool()) {
        isList = true;
    }

    if( stateInfo.cancelFlag ) {
        if( !stateInfo.hasError() ) {
            stateInfo.setError("Script task canceled");
        }
    }

}

QString ScriptPromter::composeRichDoc() {
    return target->getProto()->getDocumentation();
}

bool ScriptWorkerFactory::init(QList<DataTypePtr > input, QList<DataTypePtr > output, QList<Attribute *> attrs,
                               const QString& name, const QString &description, const QString &actorFilePath) {
    ActorPrototype * proto = IncludedProtoFactory::getScriptProto(input, output, attrs, name, description, actorFilePath);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_SCRIPT(), proto);
    IncludedProtoFactory::registerScriptWorker(ACTOR_ID + name);
    return true;
}

void ScriptWorker::init() {
    input = ports.value(IN_PORT_ID);
    output = ports.value(OUT_PORT_ID);
}

bool ScriptWorker::isReady() {
    return (input && input->hasMessage());
}

bool ScriptWorker::isDone() {
    return input->isEnded();
}


void ScriptWorker::bindPortVariables() {
    foreach( IntegralBus * bus, ports.values() ) {
        assert(bus != NULL);
        if(actor->getPort(bus->getPortId())->isOutput()) { // means that it is bus for output port
            continue;
        }

        QVariantMap busData = bus->look().getData().toMap();
        foreach(const QString & slotDesc, busData.keys()) { 
            ActorId actorId = IntegralBusType::parseSlotDesc(slotDesc);
            QString attrId = IntegralBusType::parseAttributeIdFromSlotDesc(slotDesc);
            QString portId = bus->getPortId();
            IntegralBusPort * busPort = qobject_cast<IntegralBusPort*>(actor->getPort(portId));
            Q_UNUSED(busPort);
            assert(busPort != NULL);
            
            //Actor * bindedAttrOwner = busPort->getLinkedActorById(actorId);
            attrId.prepend("in_");
            if( script->hasVarWithId(attrId)) {
                script->setVarValueWithId(attrId, busData.value(slotDesc));
            }
        }
    }
}

void ScriptWorker::bindAttributeVariables() {
    QMap<QString,Attribute*> attrs = actor->getParameters();
    QMap<QString,Attribute*>::iterator it;
    for(it = attrs.begin(); it!=attrs.end();it++) {
        Attribute* attr = it.value();
        if(script->hasVarWithId(attr->getId())) {
            script->setVarValueWithId(attr->getId(),attr->getAttributePureValue());
        }
    }
}

Task *ScriptWorker::tick() {
    //AttributeScript *script = actor->getScript();
    if( script->isEmpty() ) {
        coreLog.error(tr("no script text"));
        return new FailTask(tr("no script text"));
    }

    bindPortVariables();
    bindAttributeVariables();

    getMessageAndSetupScriptValues(input);

    Task *t = new ScriptWorkerTask(&engine, script);
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    return t;
}

void ScriptWorker::sl_taskFinished() {
    ScriptWorkerTask *t = qobject_cast<ScriptWorkerTask*>(sender());
    if (t->getState() != Task::State_Finished || t->hasError()) {
        return;
    }

    QString name = actor->getProto()->getDisplayName();
    DataTypeRegistry *dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    DataTypePtr ptr = dtr->getById(OUTPUT_PORT_TYPE + name);

    if(ptr->getAllDescriptors().size() == 1 && ptr->getAllDescriptors().first().getId() == BaseTypes::MULTIPLE_ALIGNMENT_TYPE()->getId()) {
        if (!input->isEnded()) {
            return;
        }
    }

    QVariantMap map;
    foreach(const Descriptor &desc, ptr->getAllDescriptors()) {
        QString varName = "out_" + desc.getId();
        //MAlignment ma = t->getEngine()->globalObject().property(varName.toAscii().data()).toVariant().value<MAlignment>();
        map[desc.getId()] = t->getEngine()->globalObject().property(varName.toAscii().data()).toVariant();
    }
    if(output) {
        if(t->isList) {
            QString varName = "out_" + BaseTypes::DNA_SEQUENCE_TYPE()->getId();
            QList<DNASequence> seqs = t->getEngine()->globalObject().property(varName.toAscii().data()).toVariant().value<QList<DNASequence> >();
            //QList<DNASequence> seqs = t->getResult().value<QList<DNASequence> >();
            foreach(DNASequence seq, seqs) {
                QVariantMap message;
                //message[ptr->getAllDescriptors().first().getId()] = QVariant::fromValue<DNASequence>(seq);
                map[BaseTypes::DNA_SEQUENCE_TYPE()->getId()] = QVariant::fromValue<DNASequence>(seq);
                output->put(Message(ptr,map));
            }
        }
        else {
            QVariant scriptResult = t->getResult();
            output->put(Message(ptr,map));
        }
    }

    if (input->isEnded()) {
        output->setEnded();
    }
}


} //namespace LocalWorkflow
} //namespace U2
