#include "StatisticWorkers.h"

#include <U2Lang/ConfigurationEditor.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/FailTask.h>

namespace U2 {
namespace LocalWorkflow {

const QString DNAStatWorkerFactory::ACTOR_ID("dna-stats");

void DNAStatWorkerFactory::init() {
    QList<PortDescriptor*> portDescs; 
    QList<Attribute*> attribs;

    //accept sequence and annotated regions as input
    QMap<Descriptor, DataTypePtr> inputMap;
    QMap<Descriptor, DataTypePtr> outputMap;
    inputMap[ BaseSlots::DNA_SEQUENCE_SLOT() ] = BaseTypes::DNA_SEQUENCE_TYPE();
    outputMap[ BaseSlots::ANNOTATION_TABLE_SLOT() ] = BaseTypes::ANNOTATION_TABLE_TYPE();

    { //Create input port descriptors
        Descriptor inDesc( BasePorts::IN_SEQ_PORT_ID(), DNAStatWorker::tr("Input sequence"), 
            DNAStatWorker::tr("Sequence for which GC-content and GC3-content will be evaluated.") );
        Descriptor outDesc( BasePorts::OUT_ANNOTATIONS_PORT_ID(), DNAStatWorker::tr("Result annotation"), 
            DNAStatWorker::tr("Resulted annotations, with GC-content and GC3-content.") );

        portDescs << new PortDescriptor( inDesc, DataTypePtr(new MapDataType("filter.anns", inputMap)), /*input*/ true );
        portDescs << new PortDescriptor( outDesc, DataTypePtr(new MapDataType("filter.anns", outputMap)), /*input*/false, /*multi*/true );
    }

    Descriptor desc( ACTOR_ID,
        DNAStatWorker::tr("DNA statistic"), 
        DNAStatWorker::tr("Evaluate statistic for DNA sequences") );
    ActorPrototype * proto = new IntegralBusActorPrototype( desc, portDescs, attribs );

    proto->setPrompter( new DNAStatWorkerPrompter() );
    proto->setEditor(new DelegateEditor(QMap<QString, PropertyDelegate*>()));
    WorkflowEnv::getProtoRegistry()->registerProto( BaseActorCategories::CATEGORY_STATISTIC(), proto );

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById( LocalDomainFactory::ID );
    localDomain->registerEntry( new DNAStatWorkerFactory() );
}

QString DNAStatWorkerPrompter::composeRichDoc() {
    return tr("Evaluate GC-Content and GC3-Content");
}


void DNAStatWorker::init() {
    input = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
}

bool DNAStatWorker::isReady() {
    return input->hasMessage();
}

Task* DNAStatWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(input);
    QVariantMap qm = inputMessage.getData().toMap();
    DNASequence dna = qVariantValue<DNASequence>( qm.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()) );

    if(!dna.alphabet->isNucleic()) {
        return new FailTask(tr("Sequence must be nucleotide"));
    }

    int gc = 0;
    for(int i = 0; i < dna.seq.size(); i++) {
        if(QRegExp("[cCgG]").exactMatch(QString(dna.seq.at(i)))){
            gc++;
        }
    }
    float gcContent = (float)gc/dna.seq.size();

    int gc3 = 0;
    for(int i = 2; i < dna.seq.size(); i+=3) {
        if(QRegExp("[cCgG]").exactMatch(QString(dna.seq.at(i)))){
            gc3++;
        }
    }
    float gc3Content = (float)gc3/dna.seq.size()*3;

    QList<SharedAnnotationData> res;
    SharedAnnotationData gcAnn(new AnnotationData());
    gcAnn->name = "statistic";
    gcAnn->qualifiers.push_back(U2Qualifier("gc-content", QString::number(gcContent*100) + "%"));
    gcAnn->location->regions << U2Region( 0, dna.seq.size() - 1);
    res << gcAnn;

    SharedAnnotationData gc3Ann(new AnnotationData());
    gc3Ann->name = "statistic";
    gc3Ann->qualifiers.push_back(U2Qualifier("gc3-content", QString::number(gc3Content*100) + "%"));
    gc3Ann->location->regions << U2Region( 0, dna.seq.size() - 1);
    res << gc3Ann;

    QVariant v = qVariantFromValue<QList<SharedAnnotationData> >(res);
    output->put( Message(BaseTypes::ANNOTATION_TABLE_TYPE(), v) );
    if (input->isEnded()) {
        output->setEnded();
    }

    return NULL;
}

bool DNAStatWorker::isDone() {
    return input->isEnded();
}


} //LocalWorkflow
} //U2