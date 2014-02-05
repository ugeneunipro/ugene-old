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

#include "DASAnnotationWorker.h"

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/BaseAttributes.h>

#include <U2Core/DNASequence.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/FailTask.h>
#include <U2Core/DASSource.h>
#include <U2Core/UniprotBlastTask.h>
#include <QtGui/QApplication>


namespace U2 {
namespace LocalWorkflow {

/*****************************
 * DASAnnotationWorkerFactory
 *****************************/

const QString DASAnnotationWorkerFactory::ACTOR_ID("dasannotation-search");

static const QString IDS_NUMBER("idsnumber");
static const QString IDENTITY("identity");
static const QString DB("db");
static const QString FEATURE_SOURCES("fsources");
static const QString THRESHOLD("threshold");
static const QString MATRIX("matrix");
static const QString FILTERING("filtering");
static const QString GAPPED("gapped");
static const QString MAX_RES("maxres");

void DASAnnotationWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;

    {
        Descriptor id(BasePorts::IN_SEQ_PORT_ID(), DASAnnotationWorker::tr("Input sequences"), 
            DASAnnotationWorker::tr("Protein sequences for annotation. Nucleotide sequences are skipped."));
        Descriptor od(BasePorts::OUT_ANNOTATIONS_PORT_ID(), DASAnnotationWorker::tr("DAS annotations"), 
            DASAnnotationWorker::tr("A set of annotations loaded from DAS sources using IDs of similar sequences."));
        
        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        p << new PortDescriptor(id, DataTypePtr(new MapDataType("dasannotation.sequence", inM)), true /*input*/);
        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
        p << new PortDescriptor(od, DataTypePtr(new MapDataType("dasannotation.annotations", outM)), false /*input*/, true /*multi*/);
    }
    Descriptor idsn(IDS_NUMBER, DASAnnotationWorker::tr("Max result IDs"), DASAnnotationWorker::tr("Use first IDs of similar sequences to load annotations."));
    Descriptor db(DB, DASAnnotationWorker::tr("Database"), DASAnnotationWorker::tr("Database against which the search is performed: UniProtKB or clusters of sequences with 100%, 90% or 50% identity."));
    Descriptor identity(IDENTITY, DASAnnotationWorker::tr("Min identity"), DASAnnotationWorker::tr("Minimum identity of a BLAST result and an input sequence."));
    Descriptor f_sources(FEATURE_SOURCES, DASAnnotationWorker::tr("Feature sources"), DASAnnotationWorker::tr("The DAS sources to read features from."));
    Descriptor threshold(THRESHOLD, DASAnnotationWorker::tr("Threshold"), DASAnnotationWorker::tr("The expectation value (E) threshold is a statistical measure of the number of expected matches in a random database. The lower the e-value, the more likely the match is to be significant."));
    Descriptor smatrix(MATRIX, DASAnnotationWorker::tr("Matrix"), DASAnnotationWorker::tr("The matrix assigns a probability score for each position in an alignment."));
    Descriptor filtering(FILTERING, DASAnnotationWorker::tr("Filtering"), DASAnnotationWorker::tr("Low-complexity regions (e.g. stretches of cysteine in Q03751, or hydrophobic regions in membrane proteins) tend to produce"
        "spurious, insignificant matches with sequences in the database which have the same kind of low-complexity regions, but are unrelated biologically. If 'Filter low complexity regions' is selected, the query sequence "
        "will be run through the program SEG, and all amino acids in low-complexity regions will be replaced by X's."));
    Descriptor gapped(GAPPED, DASAnnotationWorker::tr("Gapped"), DASAnnotationWorker::tr("This will allow gaps to be introduced in the sequences when the comparison is done."));
    Descriptor hits(MAX_RES, DASAnnotationWorker::tr("Hits"), DASAnnotationWorker::tr("Limits the number of returned alignments."));

    a << new Attribute(idsn, BaseTypes::NUM_TYPE(), false, QVariant(5));
    a << new Attribute(db, BaseTypes::STRING_TYPE(), false, QVariant("uniprotkb"));
    a << new Attribute(identity, BaseTypes::NUM_TYPE(), false, QVariant(90));
    a << new Attribute(threshold, BaseTypes::STRING_TYPE(), false, QVariant("10"));
    a << new Attribute(smatrix, BaseTypes::STRING_TYPE(), false, QVariant(""));
    a << new Attribute(filtering, BaseTypes::STRING_TYPE(), false, QVariant("false"));
    a << new Attribute(gapped, BaseTypes::STRING_TYPE(), false, QVariant("true"));
    a << new Attribute(hits, BaseTypes::STRING_TYPE(), false, QVariant("250"));


    Descriptor desc(ACTOR_ID, DASAnnotationWorker::tr("Annotate with DAS"), 
        DASAnnotationWorker::tr("Finds similar protein sequence using remote BLAST." 
        "<p>Using IDs of sequences found loads annotation for DAS sources."
        "<p>Nucleotide sequences are skipped if any supplied to input."
        "</p>")
        );
    QMap<QString, PropertyDelegate*> delegates;    
    
    {
        QVariantMap idsMap; idsMap["minimum"] = QVariant(1); idsMap["maximum"] = QVariant(INT_MAX); 
        delegates[IDS_NUMBER] = new SpinBoxDelegate(idsMap);
    }
    {
        QVariantMap varMap;
        varMap["UniProtKB"] = "uniprotkb";
        varMap["UniProtKB...Archaea"] = "uniprotkb_archaea";
        varMap["UniProtKB...Bacteria"] = "uniprotkb_bacteria";
        varMap["UniProtKB...Eucaryota"] = "uniprotkb_eukaryota";
        varMap["UniProtKB...Arthropoda"] = "uniprotkb_arthropoda";
        varMap["UniProtKB...Fungi"] = "uniprotkb_fungi";
        varMap["UniProtKB...Human"] = "uniprotkb_human";
        varMap["UniProtKB...Mammals"] = "uniprotkb_mammals";
        varMap["UniProtKB...Nematoda"] = "uniprotkb_nematoda";
        varMap["UniProtKB...Plants"] = "uniprotkb_plants";
        varMap["UniProtKB...Rodents"] = "uniprotkb_rodents";
        varMap["UniProtKB...Vertebrates"] = "uniprotkb_vertebrates";
        varMap["UniProtKB...Viruses"] = "uniprotkb_viruses";
        varMap["UniProtKB...PDB"] = "uniprotkb_pdb";
        varMap["UniProtKB...Complete microbial proteoms"] = "uniprotkb_complete_microbial_proteomes";
        varMap["UniProtKB/Swiss-Prot"] =  "uniprotkb_swissprot";
        varMap["UniRef100"] = "UniRef100";
        varMap["UniRef90"] = "UniRef90";
        varMap["UniRef50"] = "UniRef50";
        varMap["UniParc"] = "uniparc";
        delegates[DB] = new ComboBoxDelegate(varMap);
    }

    {
        QVariantMap valMap; valMap["minimum"] = QVariant(0); valMap["maximum"] = QVariant(100); 
        valMap["suffix"] = QVariant("%");
        delegates[IDENTITY] = new SpinBoxDelegate(valMap);
    }
    {
        DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
        SAFE_POINT (dasRegistry, DASAnnotationWorker::tr("No DAS registry"), );

        const QList<DASSource>& featureSources = dasRegistry->getFeatureSources();
        SAFE_POINT (featureSources.size() != 0, DASAnnotationWorker::tr("No DAS feature sources"), );


        QString defaultFeatures = "";
        foreach(const DASSource& fs, featureSources){
            if (fs.getReferenceType() == DASProteinSequence){
                if (defaultFeatures!=""){
                    defaultFeatures.append(",");
                }
                defaultFeatures.append(fs.getName());
            }
        }

        a << new Attribute(f_sources, BaseTypes::STRING_TYPE(), false, defaultFeatures);

        {
            QVariantMap values;

            foreach(const DASSource& fs, featureSources){
                if (fs.getReferenceType() == DASProteinSequence){
                    values[fs.getName()] = true;
                }
            }
            delegates[FEATURE_SOURCES] = new ComboBoxWithChecksDelegate(values);
        }
        {
            QVariantMap varMap;
            varMap["0.0001"] = "0.0001";
            varMap["0.001"] = "0.001";
            varMap["0.01"] = "0.01";
            varMap["0.1"] = "0.1";
            varMap["1"] = "1";
            varMap["10"] = "10";
            varMap["100"] = "100";
            varMap["1000"] = "1000";
            delegates[THRESHOLD] = new ComboBoxDelegate(varMap);
        }
        {
            QVariantMap varMap;
            varMap["Auto"] = "";
            varMap["BLOSUM-45"] = "blosum45";
            varMap["BLOSUM-62"] = "blosum62";
            varMap["BLOSUM-80"] = "blosum80";
            varMap["PAM-70"] = "pam70";
            varMap["PAM-30"] = "pam30";
            delegates[MATRIX] = new ComboBoxDelegate(varMap);

        }
        {
            QVariantMap varMap;
            varMap["None"] = "false";
            varMap["Filter low complexity regions"] = "true";
            varMap["Mask lookup table only"] = "mask";
            delegates[FILTERING] = new ComboBoxDelegate(varMap);
        }
        {
            QVariantMap varMap;
            varMap["true"] = "true";
            varMap["false"] = "false";
            delegates[GAPPED] = new ComboBoxDelegate(varMap);
        }
        {
            QVariantMap varMap;
            varMap["50"] = "50";
            varMap["100"] = "100";
            varMap["250"] = "250";
            varMap["500"] = "500";
            varMap["750"] = "750";
            varMap["1000"] = "1000";
            delegates[MAX_RES] = new ComboBoxDelegate(varMap);
        }
    }
    
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setPrompter(new DASAnnotationPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_BASIC(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new DASAnnotationWorkerFactory());
}

/*****************************
 * DASAnnotationPrompter
 *****************************/
QString DASAnnotationPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    Actor* producer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);
    QString firstIds = getParameter(IDS_NUMBER).toString();

    QString doc = tr("For each protein sequence %1, finds IDs of similar sequences using remote BLAST."
        "<br>Loads annotations for DAS sources using the first %2 ID(s).").arg(producerName).arg(firstIds);
    
    return doc;
}

/*****************************
 * DASAnnotationWorker
 *****************************/
DASAnnotationWorker::DASAnnotationWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {
}

void DASAnnotationWorker::init() {
    input = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
}

Task* DASAnnotationWorker::tick() {
    if (input->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(input);
        if (inputMessage.isEmpty()) {
            output->put(Message::getEmptyMapMessage());
        }

        DASAnnotationsSettings cfg;

        UniprotBlastSettings blastSettings;
        blastSettings.insert(UniprotBlastSettings::DATABASE, actor->getParameter(DB)->getAttributeValue<QString>(context));
        blastSettings.insert(UniprotBlastSettings::THRESHOLD, actor->getParameter(THRESHOLD)->getAttributeValue<QString>(context));
        blastSettings.insert(UniprotBlastSettings::MATRIX, actor->getParameter(MATRIX)->getAttributeValue<QString>(context));
        blastSettings.insert(UniprotBlastSettings::FILTERING, actor->getParameter(FILTERING)->getAttributeValue<QString>(context));
        blastSettings.insert(UniprotBlastSettings::GAPPED, actor->getParameter(GAPPED)->getAttributeValue<QString>(context));
        blastSettings.insert(UniprotBlastSettings::HITS, actor->getParameter(MAX_RES)->getAttributeValue<QString>(context));
        
        cfg.blastSettings = blastSettings;
        cfg.identityThreshold = actor->getParameter(IDENTITY)->getAttributeValue<int>(context);
        cfg.maxResults = actor->getParameter(IDS_NUMBER)->getAttributeValue<int>(context);

        DASSource refSource;
        QList<DASSource> featureSources;
        DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
        if (!dasRegistry){
            return new FailTask(tr("Internal error: no DAS Registry."));
        }else{
            QStringList featureids = actor->getParameter(FEATURE_SOURCES)->getAttributeValue<QString>(context).split(",", QString::SkipEmptyParts);
            foreach(const QString& fId, featureids){
                if (!fId.isEmpty()){
                    const DASSource& fs = dasRegistry->findByName(fId);
                    if (fs.isValid()){
                        featureSources.append(fs);
                    }
                }
            }
        }
        cfg.featureSources = featureSources;

        SharedDbiDataHandler seqId = inputMessage.getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<SharedDbiDataHandler>();
        std::auto_ptr<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));

        if (NULL == seqObj.get()) {
            return NULL;
        }

        cfg.sequence = seqObj->getWholeSequence().constSequence();

        qint64 seqLen = cfg.sequence.length();
        if (seqLen < MIN_SEQ_LENGTH || seqLen >= MAX_SEQ_LENGTH){
            algoLog.error(tr("Sequence %1 is %2 amino acids long. Only sequences from %3 to %4 amino acids are processed.")
                .arg(seqObj->getSequenceName())
                .arg(seqLen)
                .arg(MIN_SEQ_LENGTH)
                .arg(MAX_SEQ_LENGTH));

            output->put(Message::getEmptyMapMessage());
            return NULL;
        }
        
        const DNAAlphabet* alphabet = seqObj->getAlphabet();
        if (alphabet && alphabet->getType() == DNAAlphabet_AMINO) {
            
            Task* t = new UniprotBlastAndLoadDASAnnotations(cfg);
            connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
            return t;
        }else{
            output->put(Message::getEmptyMapMessage());
            return NULL;
        }
        QString err = tr("Bad sequence supplied to DASAnnotationWorker: %1").arg(seqObj->getSequenceName());

        return new FailTask(err);
    } else if (input->isEnded()) {
        output->setEnded();
        setDone();
    }
    return NULL;
}

void DASAnnotationWorker::sl_taskFinished( ) {
     UniprotBlastAndLoadDASAnnotations *t = qobject_cast<UniprotBlastAndLoadDASAnnotations *>( sender( ) );
     if ( !t->isFinished( ) || t->isCanceled( ) ) {
         return;
     }
     if ( NULL != output ) {
         const QList<AnnotationData> results = t->prepareResults( );
         const SharedDbiDataHandler tableId = context->getDataStorage( )->putAnnotationTable( results );
         output->put( Message( BaseTypes::ANNOTATION_TABLE_TYPE( ),
             qVariantFromValue<SharedDbiDataHandler>( tableId ) ) );

         const QStringList ids = t->getAccessionNumbers( );
         const int countOfAccessions = ids.size( );
         const QString idsString = ( 0 == countOfAccessions ) ? tr( " IDs: " ).arg( idsString ) : "";
         algoLog.trace( tr( "Number of similar sequences: %1." ).arg( countOfAccessions ) + idsString );
     }
}

void DASAnnotationWorker::cleanup( ) {

}

} //namespace LocalWorkflow
} //namespace U2
