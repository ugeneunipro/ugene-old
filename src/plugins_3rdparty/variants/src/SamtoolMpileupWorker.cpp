/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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
#include "SamtoolMpileupWorker.h"

#include "AssemblySamtoolsMpileup.h"

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
#include <QtGui/QApplication>


namespace U2 {

namespace LocalWorkflow {


const QString CallVariantsWorkerFactory::ACTOR_ID("call_variants"); 

//mpileup
const QString ILLUMINA13("illumina13-encoding");
const QString USE_ORPHAN("use_orphan");
const QString DISABLE_BAQ("disable_baq");
const QString CAPQ_THRES("capq_thres");
const QString MAX_DEPTH("max_depth");
const QString EXT_BAQ("ext_baq");
const QString BED("bed");
const QString REG("reg");
const QString MIN_MQ("min_mq");
const QString MIN_BASEQ("min_baseq");
const QString EXTQ("extQ");
const QString TANDEMQ("tandemQ");
const QString NO_INDEL("no_indel");
const QString MAX_INDEL_DEPTH("max_indel_depth");
const QString OPENQ("openQ");
const QString PL_LIST("pl_list");

//bcf view
const QString KEEPALT("keepalt");
const QString FIX_PL("fix_pl");
const QString NO_GENO("no_geno");
const QString ACGT_ONLY("acgt_only");
const QString BCF_BED("bcf_bed");
const QString QCALL("qcall");
const QString SAMPLES("samples");
const QString MIN_SMPL_FRAC("min_smpl_frac");
const QString CALL_GT("call_gt");
const QString INDEL_FRAC("indel_frac");
const QString PREF("pref");
const QString PTYPE("ptype");
const QString THETA("theta");
const QString CCALL("ccall");
const QString N1("n1");
const QString N_PERM("n_perm");
const QString MIN_PERM_P("min_perm_p");

void CallVariantsWorkerFactory::init() {
    //port descriptor
    QList<PortDescriptor*> p; 
    {
        QMap<Descriptor, DataTypePtr> refSeqMap;
        refSeqMap[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        refSeqMap[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        DataTypePtr inSet(new MapDataType("ref.sequence", refSeqMap));
        Descriptor id(BasePorts::IN_SEQ_PORT_ID(), CallVariantsWorker::tr("Input sequences"), 
            CallVariantsWorker::tr("A nucleotide reference sequence."));
        p << new PortDescriptor(id, inSet, true);

        QMap<Descriptor, DataTypePtr> assMap;
        assMap[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        assMap[BaseSlots::ASSEMBLY_SLOT()] = BaseTypes::ASSEMBLY_TYPE();
        DataTypePtr inAssemblySet(new MapDataType("assembly", assMap));
        Descriptor idA(BasePorts::IN_ASSEMBLY_PORT_ID(), CallVariantsWorker::tr("Input assembly"), 
            CallVariantsWorker::tr("Position sorted alignment file"));
        p << new PortDescriptor(idA, inAssemblySet, true);

        QMap<Descriptor, DataTypePtr> varMap;
        varMap[BaseSlots::VARIATION_TRACK_SLOT()] = BaseTypes::VARIATION_TRACK_TYPE();
        DataTypePtr outVariants(new MapDataType("variants", varMap));
        Descriptor idV(BasePorts::OUT_VARIATION_TRACK_PORT_ID(), CallVariantsWorker::tr("Output variations"), 
            CallVariantsWorker::tr("Output tracks with SNPs and short INDELs"));
        p << new PortDescriptor(idV, outVariants, false, true);

    }

    Descriptor desc(ACTOR_ID, CallVariantsWorker::tr("Call Variants with SAMTools"), 
        CallVariantsWorker::tr("Call SNPs and INDELS with samtools mpileup and bcftools")
        );


    //attributes
    QList<Attribute*> attributes;

    Descriptor illumina13Encoding(ILLUMINA13,
        CallVariantsWorker::tr("Illumina-1.3+ encoding"),
        CallVariantsWorker::tr("Assume the quality is in the Illumina 1.3+ encoding. (-6)"));

    Descriptor useOrphan(USE_ORPHAN,
        CallVariantsWorker::tr("Count anomalous read pairs"),
        CallVariantsWorker::tr("Do not skip anomalous read pairs in variant calling.(-A)"));

    Descriptor disableBaq(DISABLE_BAQ,
        CallVariantsWorker::tr("Disable BAQ computation"),
        CallVariantsWorker::tr("Disable probabilistic realignment for the computation of base "
        "alignment quality (BAQ). BAQ is the Phred-scaled probability of a read base being misaligned. "
        "Applying this option greatly helps to reduce false SNPs caused by misalignments. (-B)"));

    Descriptor capqThres(CAPQ_THRES,
        CallVariantsWorker::tr("Mapping quality downgrading coefficient"),
        CallVariantsWorker::tr("Coefficient for downgrading mapping quality for reads containing excessive mismatches. "
        "Given a read with a phred-scaled probability q of being generated from the mapped position, the new mapping quality "
        "is about sqrt((INT-q)/INT)*INT. A zero value disables this functionality; if enabled, the recommended value for BWA is 50. (-C)"));

    Descriptor maxDepth(MAX_DEPTH,
        CallVariantsWorker::tr("Max number of reads per input BAM"),
        CallVariantsWorker::tr("At a position, read maximally the number of reads per input BAM. (-d)"));

    Descriptor extBaq(EXT_BAQ,
        CallVariantsWorker::tr("Extended BAQ computation"),
        CallVariantsWorker::tr("Extended BAQ computation. This option helps sensitivity especially for MNPs,"
        " but may hurt specificity a little bit. (-E)"));

    Descriptor bed(BED,
        CallVariantsWorker::tr("BED or position list file"),
        CallVariantsWorker::tr("BED or position list file containing a list of regions or sites where"
        " pileup or BCF should be generated. (-l)"));

    Descriptor reg(REG,
        CallVariantsWorker::tr("Pileup region"),
        CallVariantsWorker::tr("Only generate pileup in region STR. (-r)"));

    Descriptor minMq(MIN_MQ,
        CallVariantsWorker::tr("Minimum mapping quality"),
        CallVariantsWorker::tr("Minimum mapping quality for an alignment to be used. (-q)"));

    Descriptor minBaseq(MIN_BASEQ,
        CallVariantsWorker::tr("Minimum base quality"),
        CallVariantsWorker::tr("Minimum base quality for a base to be considered. (-Q)"));

    Descriptor extq(EXTQ,
        CallVariantsWorker::tr("Gap extension error"),
        CallVariantsWorker::tr("Phred-scaled gap extension sequencing error probability. Reducing INT leads to longer indels. (-e)"));
 
    Descriptor tandemq(TANDEMQ,
        CallVariantsWorker::tr("Homopolymer errors coefficient"),
        CallVariantsWorker::tr("Coefficient for modeling homopolymer errors. Given an l-long homopolymer run, the sequencing error of an indel of size s is modeled as INT*s/l. (-h)"));

    Descriptor noIndel(NO_INDEL,
        CallVariantsWorker::tr("No INDELs"),
        CallVariantsWorker::tr("Do not perform INDEL calling. (-I)"));

    Descriptor maxIndelDepth(MAX_INDEL_DEPTH,
        CallVariantsWorker::tr("Max INDEL depth"),
        CallVariantsWorker::tr("Skip INDEL calling if the average per-sample depth is above INT. (-L)"));

    Descriptor openq(OPENQ,
        CallVariantsWorker::tr("Gap open error"),
        CallVariantsWorker::tr("Phred-scaled gap open sequencing error probability. Reducing INT leads to more indel calls. (-o)"));

    Descriptor plList(PL_LIST,
        CallVariantsWorker::tr("List of platforms for indels"),
        CallVariantsWorker::tr("Comma dilimited list of platforms (determined by @RG-PL) from which indel candidates are obtained."
        "It is recommended to collect indel candidates from sequencing technologies that have low indel error rate such as ILLUMINA. (-P)"));

    //bcf view
    Descriptor keepalt(KEEPALT,
        CallVariantsWorker::tr("Retain all possible alternate"),
        CallVariantsWorker::tr("Retain all possible alternate alleles at variant sites. By default, the view command discards unlikely alleles. (-A)"));

    Descriptor fixPl(FIX_PL,
        CallVariantsWorker::tr("Indicate PL"),
        CallVariantsWorker::tr("Indicate PL is generated by r921 or before (ordering is different). (-F)"));

    Descriptor noGeo(NO_GENO,
        CallVariantsWorker::tr("No genotype information."),
        CallVariantsWorker::tr("Suppress all individual genotype information. (-G)"));

    Descriptor acgtO(ACGT_ONLY,
        CallVariantsWorker::tr("A/C/G/T only"),
        CallVariantsWorker::tr("Skip sites where the REF field is not A/C/G/T (-N)"));

    Descriptor bcfBed(BCF_BED,
        CallVariantsWorker::tr("List of sites"),
        CallVariantsWorker::tr("List of sites at which information are outputted (-l)"));

    Descriptor qcall(QCALL,
        CallVariantsWorker::tr("QCALL likelihood"),
        CallVariantsWorker::tr("Output the QCALL likelihood format. (-Q)"));

    Descriptor attrSamples(SAMPLES,
        CallVariantsWorker::tr("List of samples"),
        CallVariantsWorker::tr("List of samples to use. The first column in the input gives"
        " the sample names and the second gives the ploidy, which can only be 1 or 2. When "
        "the 2nd column is absent, the sample ploidy is assumed to be 2. In the output, the ordering of samples "
        "will be identical to the one in FILE.  (-s)"));

    Descriptor minSmpl(MIN_SMPL_FRAC,
        CallVariantsWorker::tr("Min samples fraction"),
        CallVariantsWorker::tr("skip loci where the fraction of samples covered by reads is below FLOAT. (-d)"));

    Descriptor callGt(CALL_GT,
        CallVariantsWorker::tr("Per-sample genotypes"),
        CallVariantsWorker::tr("Call per-sample genotypes at variant sites. (-g)"));
    
    Descriptor indelFrac(INDEL_FRAC,
        CallVariantsWorker::tr("INDEL-to-SNP Ratio"),
        CallVariantsWorker::tr("Ratio of INDEL-to-SNP mutation rate. (-i)"));

    Descriptor pref(PREF,
        CallVariantsWorker::tr("Gap open error"),
        CallVariantsWorker::tr("Phred-scaled gap open sequencing error probability. Reducing INT leads to more indel calls. (-o)"));

    Descriptor ptype(PTYPE,
        CallVariantsWorker::tr("Max P(ref|D)"),
        CallVariantsWorker::tr("A site is considered to be a variant if P(ref|D)<FLOAT. (-p)"));

    Descriptor theta(THETA,
        CallVariantsWorker::tr("Muttion rate"),
        CallVariantsWorker::tr("Scaled muttion rate for variant calling. (-t)"));

    Descriptor ccall(CCALL,
        CallVariantsWorker::tr("Pair/trio calling"),
        CallVariantsWorker::tr("Enable pair/trio calling. For trio calling, option -s is usually needed to be"
        " applied to configure the trio members and their ordering. In the file supplied to the option -s, the "
        "first sample must be the child, the second the father and the third the mother. The valid values of STR "
        "are ‘pair’, ‘trioauto’, ‘trioxd’ and ‘trioxs’, where ‘pair’ calls differences between two input samples, and ‘trioxd’ (‘trioxs’)"
        "specifies that the input is from the X chromosome non-PAR regions and the child is a female (male). (-T)"));

    Descriptor n1(N1,
        CallVariantsWorker::tr("N group-1 samples"),
        CallVariantsWorker::tr("Number of group-1 samples. This option is used for dividing the samples "
        "into two groups for contrast SNP calling or association test. When this option is in use, the following"
        "VCF INFO will be outputted: PC2, PCHI2 and QCHI2. (-1)"));

    Descriptor n_perm(N_PERM,
        CallVariantsWorker::tr("N permutations"),
        CallVariantsWorker::tr("Number of permutations for association test (effective only with -1). (-U)"));

    Descriptor min_perm_p(MIN_PERM_P,
        CallVariantsWorker::tr("Max P(chi^2)"),
        CallVariantsWorker::tr("Only perform permutations for P(chi^2)<FLOAT (N permutations) . (-X)"));

    attributes << new Attribute(illumina13Encoding, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(useOrphan, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(disableBaq, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(capqThres, BaseTypes::NUM_TYPE(), false, QVariant(0));
    attributes << new Attribute(maxDepth, BaseTypes::NUM_TYPE(), false, QVariant(250));
    attributes << new Attribute(extBaq, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(bed, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(reg, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(minMq, BaseTypes::NUM_TYPE(), false, QVariant(0));
    attributes << new Attribute(minBaseq, BaseTypes::NUM_TYPE(), false, QVariant(13));
    attributes << new Attribute(extq, BaseTypes::NUM_TYPE(), false, QVariant(20));
    attributes << new Attribute(tandemq, BaseTypes::NUM_TYPE(), false, QVariant(100));
    attributes << new Attribute(noIndel, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(maxIndelDepth, BaseTypes::NUM_TYPE(), false, QVariant(250));
    attributes << new Attribute(openq, BaseTypes::NUM_TYPE(), false, QVariant(40));
    attributes << new Attribute(plList, BaseTypes::STRING_TYPE(), false, QVariant(""));


    //bcf vew
    attributes << new Attribute(keepalt, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(fixPl, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(noGeo, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(acgtO, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(bcfBed, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(qcall, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(attrSamples, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(minSmpl, BaseTypes::NUM_TYPE(), false, QVariant(0));
    attributes << new Attribute(callGt, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(indelFrac, BaseTypes::NUM_TYPE(), false, QVariant(0.15));
    attributes << new Attribute(pref, BaseTypes::NUM_TYPE(), false, QVariant(0.5));
    attributes << new Attribute(ptype, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(theta, BaseTypes::NUM_TYPE(), false, QVariant(0.001));
    attributes << new Attribute(ccall, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(n1, BaseTypes::NUM_TYPE(), false, QVariant(0));
    attributes << new Attribute(n_perm, BaseTypes::NUM_TYPE(), false, QVariant(0));
    attributes << new Attribute(min_perm_p, BaseTypes::NUM_TYPE(), false, QVariant(0.1));
 
    //prototype
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, attributes);
    proto->addSlotRelation(BasePorts::IN_SEQ_PORT_ID(), BaseSlots::DNA_SEQUENCE_SLOT().getId(),
        BasePorts::OUT_VARIATION_TRACK_PORT_ID(), BaseSlots::VARIATION_TRACK_SLOT().getId());
    proto->addSlotRelation(BasePorts::IN_ASSEMBLY_PORT_ID(), BaseSlots::ASSEMBLY_SLOT().getId(),
        BasePorts::OUT_VARIATION_TRACK_PORT_ID(), BaseSlots::VARIATION_TRACK_SLOT().getId());



    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[CAPQ_THRES] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[MAX_DEPTH] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[MIN_MQ] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[MIN_BASEQ] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[EXTQ] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[TANDEMQ] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[MAX_INDEL_DEPTH] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[OPENQ] = new SpinBoxDelegate(vm);
    }

    delegates[BED] = new URLDelegate("", "", false, true);

    //bcf view
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["maximum"] = 1;
        vm["singleStep"] = 0.1;
        delegates[MIN_SMPL_FRAC] = new DoubleSpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["maximum"] = 1;
        vm["singleStep"] = 0.1;
        delegates[INDEL_FRAC] = new DoubleSpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["maximum"] = 1;
        vm["singleStep"] = 0.1;
        delegates[PREF] = new DoubleSpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm[""] = "";
        vm["full"] = "full";
        vm["cond2"] = "cond2";
        vm["flat"] = "flat";
        delegates[PTYPE] = new ComboBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["maximum"] = 1;
        vm["singleStep"] = 0.1;
        delegates[THETA] = new DoubleSpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm[""] = "";
        vm["pair"] = "pair";
        vm["trioauto"] = "trioauto";
        vm["trioxd"] = "trioxd";
        vm["trioxs"] = "trioxs";
        delegates[CCALL] = new ComboBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[N1] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[N_PERM] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["maximum"] = 1;
        vm["singleStep"] = 0.1;
        delegates[MIN_PERM_P] = new DoubleSpinBoxDelegate(vm);
    }

    delegates[BCF_BED] = new URLDelegate("", "", false, true);
    delegates[SAMPLES] = new URLDelegate("", "", false, true);


    // Init and register the actor prototype
    proto->setEditor(new DelegateEditor(delegates));

    proto->setPrompter(new CallVariantsPrompter());
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CALL_VARIATIONS(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new CallVariantsWorkerFactory());
}


QString CallVariantsPrompter::composeRichDoc() {
    Actor* assemblyProducer = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_ASSEMBLY_PORT_ID()))->getProducer(BaseSlots::ASSEMBLY_SLOT().getId());
    Actor* seqProducer = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()))->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());

    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString seqName = tr("For reference sequence from <u>%1</u>,").arg(seqProducer ? seqProducer->getLabel() : unsetStr);
    QString assemblyName = tr("with assembly data provided by <u>%1</u>,").arg(assemblyProducer ? assemblyProducer->getLabel() : unsetStr);

    QString doc = tr("%1 call variants %2.")
        .arg(seqName)
        .arg(assemblyName);

    return doc;
}


CallVariantsWorker::CallVariantsWorker(Actor* a) : BaseWorker(a, false), refSeqPort(NULL), assemblyPort(NULL), output(NULL) {
}

void CallVariantsWorker::init() {
    refSeqPort = ports.value(BasePorts::IN_SEQ_PORT_ID());
    assemblyPort = ports.value(BasePorts::IN_ASSEMBLY_PORT_ID());
    output = ports.value(BasePorts::OUT_VARIATION_TRACK_PORT_ID());

    output->addComplement(refSeqPort);
    refSeqPort->addComplement(output);
}

bool CallVariantsWorker::isReady(){
    if (isDone()) {
        return false;
    }
    bool seqEnded = refSeqPort->isEnded();
    bool assEnded = assemblyPort->isEnded();
    int seqHasMes = refSeqPort->hasMessage();
    int assHasMes = assemblyPort->hasMessage();
    return  (assHasMes || assEnded && (seqEnded || seqHasMes));
    
}

Task* CallVariantsWorker::tick() {
    //check params

    //get all assemblies
    while(assemblyPort->hasMessage()){
        Message inputMessage = getMessageAndSetupScriptValues(assemblyPort);
        if (!inputMessage.isEmpty()) {
            QString assemblyUrl;
            assemblyUrl = inputMessage.getData().toMap().value(BaseSlots::URL_SLOT().getId()).value<QString>();
            assemblyUrls.append(assemblyUrl);
        }
    }
    if (!assemblyPort->isEnded()) {
        return NULL;
    }

    //get sequence
    if (cache.isEmpty() && refSeqPort->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(refSeqPort);
        if (inputMessage.isEmpty() || assemblyUrls.isEmpty()) {
            output->transit();
            return NULL;
        }
        CallVariantsTaskSettings settings;
        settings.refSeqUrl = inputMessage.getData().toMap().value(BaseSlots::URL_SLOT().getId()).value<QString>();
        settings.assemblyUrls = assemblyUrls;

        settings.illumina13 = actor->getParameter(ILLUMINA13)->getAttributeValue<bool>(context);
        settings.use_orphan = actor->getParameter(USE_ORPHAN)->getAttributeValue<bool>(context);
        settings.disable_baq = actor->getParameter(DISABLE_BAQ)->getAttributeValue<bool>(context);
        settings.capq_thres = actor->getParameter(CAPQ_THRES)->getAttributeValue<int>(context);
        settings.max_depth = actor->getParameter(MAX_DEPTH)->getAttributeValue<int>(context);
        settings.ext_baq = actor->getParameter(EXT_BAQ)->getAttributeValue<bool>(context);
        settings.bed = actor->getParameter(BED)->getAttributeValue<QString>(context).toAscii();
        settings.reg = actor->getParameter(REG)->getAttributeValue<QString>(context).toAscii();
        settings.min_mq = actor->getParameter(MIN_MQ)->getAttributeValue<int>(context);
        settings.min_baseq = actor->getParameter(MIN_BASEQ)->getAttributeValue<int>(context);
        settings.extq = actor->getParameter(EXTQ)->getAttributeValue<int>(context);
        settings.tandemq = actor->getParameter(TANDEMQ)->getAttributeValue<int>(context);
        settings.no_indel = actor->getParameter(NO_INDEL)->getAttributeValue<bool>(context);
        settings.max_indel_depth = actor->getParameter(MAX_INDEL_DEPTH)->getAttributeValue<int>(context);
        settings.openq = actor->getParameter(OPENQ)->getAttributeValue<int>(context);
        settings.pl_list = actor->getParameter(PL_LIST)->getAttributeValue<QString>(context).toAscii();

        //bcf view
        settings.keepalt = actor->getParameter(KEEPALT)->getAttributeValue<bool>(context);
        settings.fix_pl = actor->getParameter(FIX_PL)->getAttributeValue<bool>(context);
        settings.no_geno = actor->getParameter(NO_GENO)->getAttributeValue<bool>(context);
        settings.acgt_only = actor->getParameter(ACGT_ONLY)->getAttributeValue<bool>(context);
        settings.bcf_bed = actor->getParameter(BCF_BED)->getAttributeValue<QString>(context).toAscii();
        settings.qcall = actor->getParameter(QCALL)->getAttributeValue<bool>(context);
        settings.samples = actor->getParameter(SAMPLES)->getAttributeValue<QString>(context).toAscii();
        settings.min_smpl_frac = actor->getParameter(MIN_SMPL_FRAC)->getAttributeValue<float>(context);
        settings.call_gt = actor->getParameter(CALL_GT)->getAttributeValue<bool>(context);
        settings.indel_frac = actor->getParameter(INDEL_FRAC)->getAttributeValue<float>(context);
        settings.pref = actor->getParameter(PREF)->getAttributeValue<float>(context);
        settings.ptype = actor->getParameter(PTYPE)->getAttributeValue<QString>(context).toAscii();
        settings.theta = actor->getParameter(THETA)->getAttributeValue<float>(context);
        settings.ccall = actor->getParameter(CCALL)->getAttributeValue<QString>(context).toAscii();
        settings.n1 = actor->getParameter(N1)->getAttributeValue<int>(context);
        settings.n_perm = actor->getParameter(N_PERM)->getAttributeValue<int>(context);
        settings.min_perm_p = actor->getParameter(MIN_PERM_P)->getAttributeValue<float>(context);

        Task* t = new CallVariantsTask(settings, context->getDataStorage());
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));

        return t;
    } 

    //put variant tracks
    while (!cache.isEmpty()) {
        output->put(cache.takeFirst());
    }

    if (refSeqPort->isEnded()) {
        setDone();
        output->setEnded();
    }

    return NULL;
}

void CallVariantsWorker::sl_taskFinished() {
    CallVariantsTask* t = qobject_cast<CallVariantsTask*>(sender());
    if (t->getState() != Task::State_Finished) return;

    DataTypePtr mtype = output->getBusType();
    const QList<QVariantMap>& res = t->getResults();
    foreach(const QVariantMap &m, res) {
        cache.append(Message(mtype, m));
    }
    t->clearResults();
}

void CallVariantsWorker::cleanup() {
}

} //namespace LocalWorkflow

} // namespace
