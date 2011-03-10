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

static QScriptValue debugOut(QScriptContext *ctx, QScriptEngine *)  {
    scriptLog.details(ctx->argument(0).toString());
    return 0;
}

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

    TaskStateInfo tsi;
    QScriptValue foo = engine->globalObject();

    foo.setProperty("subsequence",engine->newFunction(getSubsequence));
    foo.setProperty("complement", engine->newFunction(complement));
    foo.setProperty("size", engine->newFunction(sequenceSize));
    foo.setProperty("translate", engine->newFunction(translate));
    foo.setProperty("charAt", engine->newFunction(charAt));
    foo.setProperty("alphabetType", engine->newFunction(alphabetType));
    foo.setProperty("getName", engine->newFunction(sequenceName));
    foo.setProperty("printToLog",engine->newFunction(debugOut));
    foo.setProperty("isAmino", engine->newFunction(isAmino));
    foo.setProperty("getMinimumQuality", engine->newFunction(getMinimumQuality));
    foo.setProperty("hasQuality",engine->newFunction(hasQuality));
    foo.setProperty("sequenceFromText", engine->newFunction(sequenceFromText));

    foo.setProperty("createAlignment", engine->newFunction(createAlignment));
    foo.setProperty("sequenceFromAlignment",engine->newFunction(getSequenceFromAlignment));
    foo.setProperty("addToAlignment",engine->newFunction(addToAlignment));
    foo.setProperty("findInAlignment",engine->newFunction(findInAlignment));
    foo.setProperty("removeFromAlignment",engine->newFunction(removeFromAlignment));
    foo.setProperty("rowNum",engine->newFunction(rowNum));
    foo.setProperty("columnNum",engine->newFunction(columnNum));
    foo.setProperty("alignmentAlphabetType",engine->newFunction(alignmentAlphabetType));

    foo.setProperty("annotatedRegions", engine->newFunction(getAnnotationRegion));
    foo.setProperty("addQualifier", engine->newFunction(addQualifier));
    foo.setProperty("getLocation", engine->newFunction(getLocation));
    foo.setProperty("filterByQualifier", engine->newFunction(filterByQualifier));

    QScriptValue scriptResultValue = ScriptTask::runScript(engine, scriptVars, script->getScriptText(), stateInfo);
    result = scriptResultValue.toVariant();
    if(engine->hasUncaughtException()) {
        stateInfo.setError(tr("Error in line ") + QString::number(engine->uncaughtExceptionLineNumber()) + ":" + result.toString().split(":").last());
    }

    if(engine->globalObject().property("list").toBool()) {
        isList = true;
    }

    if( stateInfo.cancelFlag ) {
        if( !stateInfo.hasErrors() ) {
            stateInfo.setError("Script task canceled");
        }
    }

}

QString ScriptPromter::composeRichDoc() {
    return target->getProto()->getDocumentation();
}

static Descriptor getSlotDescOfDatatype(const DataTypePtr & dt) {
    QString dtId = dt->getId();
    if(dtId == BaseTypes::DNA_SEQUENCE_TYPE()->getId()) {
        return BaseSlots::DNA_SEQUENCE_SLOT();
    }
    if(dtId == BaseTypes::ANNOTATION_TABLE_TYPE()->getId()) {
        return BaseSlots::ANNOTATION_TABLE_SLOT();
    }
    if(dtId == BaseTypes::MULTIPLE_ALIGNMENT_TYPE()->getId()) {
        return BaseSlots::MULTIPLE_ALIGNMENT_SLOT();
    }
    if(dtId == BaseTypes::STRING_TYPE()->getId()) {
        return BaseSlots::TEXT_SLOT();
    }
    assert(false);
    return *dt;
}

bool ScriptWorkerFactory::init(QList<DataTypePtr > input, QList<DataTypePtr > output, QList<Attribute *> attrs, const QString& name, const QString &description) {
    QList<PortDescriptor*> portDescs; 
    QList<Attribute*> attribs = attrs;

    QMap<Descriptor, DataTypePtr> map;
    foreach(const DataTypePtr & tptr, input) {
        if(tptr == DataTypePtr()) {
            coreLog.error(ScriptWorker::tr("For input port was set empty data type"));
            return false;
        }
        map[getSlotDescOfDatatype(tptr)] = tptr;
    }

    DataTypePtr inSet( new MapDataType(Descriptor(INPUT_PORT_TYPE + name), map) );
    DataTypeRegistry * dr = WorkflowEnv::getDataTypeRegistry();
    assert(dr);
    dr->registerEntry( inSet );

    map.clear();
    foreach(const DataTypePtr & tptr, output) {
        if(tptr == DataTypePtr()) {
            coreLog.error(ScriptWorker::tr("For output port was set empty data type"));
            return false;
        }
        map[getSlotDescOfDatatype(tptr)] = tptr;
    }
    
    DataTypePtr outSet( new MapDataType(Descriptor(OUTPUT_PORT_TYPE + name), map) );
    dr->registerEntry( outSet );

    Descriptor inDesc( IN_PORT_ID, ScriptWorker::tr("input data"), ScriptWorker::tr("input data") );
    Descriptor outDesc( OUT_PORT_ID, ScriptWorker::tr("output data"), ScriptWorker::tr("output data") );

    portDescs << new PortDescriptor( inDesc, inSet, /*input*/ true );
    portDescs << new PortDescriptor( outDesc, outSet, /*input*/false, /*multi*/true );


    Descriptor desc( ScriptWorkerFactory::ACTOR_ID + name, name, description );
    ActorPrototype * proto = new IntegralBusActorPrototype( desc, portDescs, attribs );

    proto->setEditor( new DelegateEditor(QMap<QString, PropertyDelegate*>()) );
    proto->setIconPath(":workflow_designer/images/script.png");

    proto->setPrompter( new ScriptPromter() );
    proto->setScriptFlag();
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_SCRIPT(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById( LocalDomainFactory::ID );
    localDomain->registerEntry( new ScriptWorkerFactory(ACTOR_ID + name) );
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
            assert(busPort != NULL);
            
            Actor * bindedAttrOwner = busPort->getLinkedActorById(actorId);
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
    if (t->getState() != Task::State_Finished || t->hasErrors()) {
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
