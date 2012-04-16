#include "AminoTranslationWorker.h"

#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Designer/DelegateEditors.h>

#include <U2Core/U2SequenceUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/FailTask.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNAAlphabet.h>

const QString OFFSET_DELIMITER(",");

namespace U2{
namespace LocalWorkflow{

const QString AminoTranslationWorkerFactory::ACTOR_ID("sequence-translation");

static const QString NAME_ATTR("result-name");

void TranslateSequence2AminoTask::run(){
    assert(configs.seqObj != NULL);

    U2SequenceObject* seqObj = configs.seqObj.data();
    qint64 seqLen = seqObj->getSequenceLength();
    QVector<U2Region> regionsDirect = configs.regionsDirect;
    QVector<U2Region> regionsComplementary = configs.regionsComplementary;
    QString resultName = configs.resultName;    

    for(QVector<U2Region>::iterator i = regionsDirect.begin(); i != regionsDirect.end();){
        if(i->startPos >= seqLen){
            i =  regionsDirect.erase(i);
        }
        else{
            ++i;
        }
    }

    for(QVector<U2Region>::iterator i = regionsComplementary.begin(); i != regionsComplementary.end();){
        if(i->startPos >= seqLen){
            i =  regionsComplementary.erase(i);
        }
        else{
            ++i;
        }
    }

    DNATranslation* complTT = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(seqObj->getAlphabet());
    SAFE_POINT(complTT != NULL || regionsComplementary.isEmpty(), QString("Can't support complement mode"),);
    DNATranslation* aminoTT = AppContext::getDNATranslationRegistry()->lookupTranslation(seqObj->getAlphabet(), DNATranslationType_NUCL_2_AMINO).first();
    SAFE_POINT(aminoTT != NULL, QString("Can't produce amino translation"),);

    static const int NUM_DB_READ = 3 * 1024;

    int currentSeq = 0;

    QByteArray seq;
    QByteArray translatedSeq;
    // direct
    foreach(U2Region directRegion, regionsDirect){
        seq.clear();
        translatedSeq.clear();

        TmpDbiHandle dbiHandle(WORKFLOW_SESSION_TMP_DBI_ALIAS, stateInfo);
        U2DbiRef dbRef = dbiHandle.getDbiRef();
        CHECK_OP(stateInfo, );

        U2SequenceImporter importer;
        importer.startSequence(dbRef, resultName + QString("_%1").arg(currentSeq), false, stateInfo);
        
        int blockCounter = 0;
        qint64 end = directRegion.startPos +  directRegion.length / 3 * 3;
        for(qint64 i = directRegion.startPos; i < end ; i+=3, blockCounter += 3){
            if( (blockCounter % NUM_DB_READ) == 0){
                importer.addBlock(translatedSeq.constData(), translatedSeq.size(), stateInfo);
                seq.clear();
                translatedSeq.clear();
                seq = seqObj->getSequenceData(U2Region(i, qMin(static_cast<qint64>(NUM_DB_READ), end  - i)));
                blockCounter = 0;
            }
            translatedSeq.append(aminoTT->translate3to1(seq[blockCounter], seq[blockCounter + 1], seq[blockCounter + 2]));
        }

        importer.addBlock(translatedSeq.constData(), translatedSeq.size(), stateInfo);
        U2Sequence u2Seq = importer.finalizeSequence(stateInfo);
          
        CHECK_OP(stateInfo, );
        results << new U2SequenceObject(u2Seq.visualName, U2EntityRef(dbRef, u2Seq.id));
        currentSeq++;
    }
    
    // complementary
    foreach(U2Region complementaryRegion, regionsComplementary){
        translatedSeq.clear();

        TmpDbiHandle dbiHandle(WORKFLOW_SESSION_TMP_DBI_ALIAS, stateInfo);
        U2DbiRef dbRef = dbiHandle.getDbiRef();
        CHECK_OP(stateInfo, );

        U2SequenceImporter importer;
        importer.startSequence(dbRef, resultName + QString("_%1").arg(currentSeq), false, stateInfo);

        QByteArray complementarySeq;
        char* complSeq;

        int blockCounter = 0;
        qint64 end = complementaryRegion.endPos() - 1  - complementaryRegion.length / 3 * 3;
        
        for(qint64 i = complementaryRegion.endPos() - 1 ; i > end ;i-=3, blockCounter += 3){
            if( (blockCounter % NUM_DB_READ) == 0 ){
                importer.addBlock(translatedSeq.constData(), translatedSeq.size(), stateInfo);
                complementarySeq.clear();
                seq.clear();
                translatedSeq.clear();
                seq = seqObj->getSequenceData(U2Region(qMax(end + 1, static_cast<qint64>(i - NUM_DB_READ + 1)), qMin(static_cast<qint64>(NUM_DB_READ), i - end)));
                complementarySeq.reserve(seq.size());
                complSeq = complementarySeq.data();
                TextUtils::translate(complTT->getOne2OneMapper(), seq, seq.size(), complSeq);
                TextUtils::reverse(complSeq, seq.size());
                blockCounter = 0;

            }
            translatedSeq.append(aminoTT->translate3to1(complSeq[blockCounter], complSeq[blockCounter + 1], complSeq[blockCounter + 2]));
        }

        importer.addBlock(translatedSeq.constData(), translatedSeq.size(), stateInfo);
        U2Sequence u2Seq = importer.finalizeSequence(stateInfo);

        CHECK_OP(stateInfo, );
        results << new U2SequenceObject(u2Seq.visualName, U2EntityRef(dbRef, u2Seq.id));
        currentSeq++;
    }
}


void AminoTranslationWorkerFactory::init(){

    QMap<Descriptor, DataTypePtr> m;
    m[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
    DataTypePtr inSet(new MapDataType(Descriptor("regioned.sequence"), m));
    DataTypeRegistry* dr = WorkflowEnv::getDataTypeRegistry();
    assert(dr);
    dr->registerEntry(inSet);

    QList<PortDescriptor*> p;
    {
        Descriptor ind(BasePorts::IN_SEQ_PORT_ID(),
            AminoTranslationWorker::tr("Input Data"),
            AminoTranslationWorker::tr("An input sequence to translate in amino."));

        Descriptor oud(BasePorts::OUT_SEQ_PORT_ID(),
            AminoTranslationWorker::tr("Amino sequence"),
            AminoTranslationWorker::tr("Sequence has been translated."));

        p << new PortDescriptor(ind, inSet, true);
        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        outM[BaseSlots::TEXT_SLOT()] = BaseTypes::STRING_TYPE();

        p << new PortDescriptor(oud, DataTypePtr(new MapDataType("translate.sequence", outM)), false, true);
    }
    QList<Attribute*> a;
    a << new Attribute(BaseAttributes::STRAND_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false, BaseAttributes::STRAND_BOTH());
   
    Descriptor nd(NAME_ATTR, AminoTranslationWorker::tr("Sequence name"), AminoTranslationWorker::tr("Name of the result amino translation"));
    a << new Attribute(nd, BaseTypes::STRING_TYPE(), true, QVariant("Amino translation"));

    a << new Attribute(BaseAttributes::AMINO_TRANSLATION_OFFSET_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false, BaseAttributes::AMINO_TRANSLATION_OFFSET_ALL());

    Descriptor desc(ACTOR_ID,
        AminoTranslationWorker::tr("Amino translation"),
        AminoTranslationWorker::tr("Translate sequence into it's amino translation"
        " . Output a sequence."));

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

    QMap<QString, PropertyDelegate*> delegates;    
    delegates[BaseAttributes::STRAND_ATTRIBUTE().getId()] = new ComboBoxDelegate(BaseAttributes::STRAND_ATTRIBUTE_VALUES_MAP());
    delegates[BaseAttributes::AMINO_TRANSLATION_OFFSET_ATTRIBUTE().getId()] = new ComboBoxDelegate(BaseAttributes::AMINO_TRANSLATION_OFFSET_ATTRIBUTE_VALUES_MAP());
    
    proto->setEditor(new DelegateEditor(delegates));

    proto->setIconPath( ":core/images/show_trans.png" );
    proto->setPrompter(new AminoTranslationPrompter());
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_BASIC(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new AminoTranslationWorkerFactory());
}

static SequenceTranslationStrand getStrand(const QString & s) {
    QString str = s.toLower();
    if(BaseAttributes::STRAND_BOTH().startsWith(str)) {
        return SequenceTranslationStrand_Both;
    } else if(BaseAttributes::STRAND_DIRECT().startsWith(str)) {
        return SequenceTranslationStrand_Direct;
    } else if(BaseAttributes::STRAND_COMPLEMENTARY().startsWith(str)) {
        return SequenceTranslationStrand_Complement;
    } else {
        bool ok = false;
        int num = str.toInt(&ok);
        if(ok && num >= 0) {
            return SequenceTranslationStrand(num);
        } else {
            return SequenceTranslationStrand_Both;
        }
    }
}


QString AminoTranslationPrompter::composeRichDoc(){
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    Actor* producer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);

    SequenceTranslationStrand strand = getStrand(getParameter(BaseAttributes::STRAND_ATTRIBUTE().getId()).value<QString>());
    
    QString strandName;
    switch (strand) {
        case SequenceTranslationStrand_Both:
            strandName = AminoTranslationWorker::tr("both strands");
            break;
        case SequenceTranslationStrand_Direct:
            strandName = AminoTranslationWorker::tr("direct strand");
            break;
        case SequenceTranslationStrand_Complement:
            strandName = AminoTranslationWorker::tr("complement strand");
            break;
        default:
            assert(false);
    }
    QString offset = getParameter(BaseAttributes::AMINO_TRANSLATION_OFFSET_ATTRIBUTE().getId()).value<QString>();
    QString resultName = getParameter(NAME_ATTR).toString();

    resultName = getHyperlink(NAME_ATTR, resultName);
    offset = getHyperlink(BaseAttributes::AMINO_TRANSLATION_OFFSET_ATTRIBUTE().getId(), offset);
    strandName = getHyperlink(BaseAttributes::STRAND_ATTRIBUTE().getId(), strandName);
    QString doc = QString("Translated sequence with base name %1 to amino in %2 with %3 offset").
                    arg(resultName).
                    arg(strandName).
                    arg(offset);
    return doc;
}

AminoTranslationWorker::AminoTranslationWorker(Actor* a): BaseWorker(a), input(NULL), output(NULL) {}

void AminoTranslationWorker::init(){
    input = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_SEQ_PORT_ID());
}

Task* AminoTranslationWorker::tick(){
    if (input->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(input);
        
        if (inputMessage.isEmpty()) {
            output->put(Message::getEmptyMapMessage());
        }

        U2DataId seqId = inputMessage.getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<U2DataId>();
        QSharedPointer<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));


        if (NULL == seqObj.data()) {
            return NULL;
        }

        if(seqObj->getAlphabet() == NULL  ){
            return NULL;
        }

        if(!seqObj->getAlphabet()->isNucleic()){
            return new FailTask(tr("Alphabet is not nucleic"));
        }


        QStringList offsets = actor->getParameter(BaseAttributes::AMINO_TRANSLATION_OFFSET_ATTRIBUTE().getId())->getAttributeValue<QString>(context).split(OFFSET_DELIMITER, QString::SkipEmptyParts);
        QString resultName = actor->getParameter(NAME_ATTR)->getAttributeValue<QString>(context);
        if(resultName.isEmpty()){
            algoLog.error(tr("Amino translation: result name is not defined, used default one instead"));
            resultName = "Sequence";
        }

        SequenceTranslationStrand strand = getStrand(actor->getParameter(BaseAttributes::STRAND_ATTRIBUTE().getId())->getAttributeValue<QString>(context));

        QVector<U2Region> regionsDirect;
        QVector<U2Region> regionsComplementaries;

        foreach(const QString& offset, offsets){
            bool ok;            
            int currentOffset = offset.toInt(&ok);
                        
            switch (strand) {
        case SequenceTranslationStrand_Both:
            regionsDirect.append(U2Region(currentOffset, seqObj->getSequenceLength() - currentOffset));
            regionsComplementaries.append(U2Region(0, seqObj->getSequenceLength() - currentOffset));
            break;
        case SequenceTranslationStrand_Direct:
            regionsDirect.append(U2Region(currentOffset, seqObj->getSequenceLength() - currentOffset));
            break;
        case SequenceTranslationStrand_Complement:
            regionsComplementaries.append(U2Region(0, seqObj->getSequenceLength() - currentOffset));
            break;
        default:
            assert(false);
            }
        }
      
        AminoTranslationSettings  config;

        config.regionsDirect = regionsDirect;
        config.regionsComplementary = regionsComplementaries;
        config.resultName = resultName;
        config.seqObj = seqObj;

        TranslateSequence2AminoTask* transTask = new TranslateSequence2AminoTask(config);

        connect(transTask ,SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return transTask;
    }   
    else if (input->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void AminoTranslationWorker::cleanup(){

}

void AminoTranslationWorker::sl_taskFinished(){
    TranslateSequence2AminoTask* translate2AminoTask = qobject_cast<TranslateSequence2AminoTask*>(sender());
    if (translate2AminoTask->getState() != Task::State_Finished) {
        return;
    }
    
    if(output){
        QVariantMap channelContext = output->getContext();

        QList<U2SequenceObject*> seqObjs = translate2AminoTask->popResults();
        foreach(U2SequenceObject* seqObj, seqObjs ){
            QVariantMap msgData;
            U2DataId seqId = context->getDataStorage()->putSequence(seqObj->getWholeSequence());
            msgData[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = seqId;
            output->put(Message(BaseTypes::DNA_SEQUENCE_TYPE(), msgData));
            output->setContext(channelContext);
        }
    }
}

}
}
