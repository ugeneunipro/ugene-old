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

#include "CufflinksSupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/L10n.h>
#include <U2Core/QVariantUtils.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "CuffdiffWorker.h"

namespace U2 {
namespace LocalWorkflow {

/*****************************
 * CuffdiffWorkerFactory
 *****************************/
const QString CuffdiffWorkerFactory::ACTOR_ID("cuffdiff");

static const QString TIME_SERIES_ANALYSIS("time-series-analysis");
static const QString UPPER_QUARTILE_NORM("upper-quartile-norm");
static const QString HITS_NORM("hits-norm");
static const QString FRAG_BIAS_CORRECT("frag-bias-correct");
static const QString MULTI_READ_CORRECT("multi-read-correct");
static const QString LIBRARY_TYPE("library-type");
static const QString MASK_FILE("mask-file");
static const QString MIN_ALIGNMENT_COUNT("min-alignment-count");
static const QString FDR("fdr");
static const QString MAX_MLE_ITERATIONS("max-mle-iterations");
static const QString EMIT_COUNT_TABLES("emit-count-tables");
static const QString EXT_TOOL_PATH("path");
static const QString TMP_DIR_PATH("tmp-dir");

static const QString OUT_PORT("output");

static const QString SPLICING_DIFF_SLOT("splicing");
static const QString PROMOTERS_DIFF_SLOT("promoters");
static const QString CDS_DIFF_SLOT("cds-diff");
static const QString CDS_EXP_SLOT("cds-exp");
static const QString CDS_FPKM_SLOT("cds-fpkm");
static const QString TSS_EXP_SLOT("tss-groups-exp");
static const QString TSS_FPKM_SLOT("tss-groups-fpkm");
static const QString GENES_EXP_SLOT("genes-exp");
static const QString GENES_FPKM_SLOT("genes-fpkm");
static const QString ISOMORFS_EXP_SLOT("isomorfs-exp");
static const QString ISOMORFS_FPKM_SLOT("isomorfs-fpkm");

void CuffdiffWorkerFactory::init()
{
    QList<PortDescriptor*> portDescriptors;
    QList<Attribute*> attributes;

    // Description of the element
    Descriptor cuffdiffDescriptor(ACTOR_ID,
        CuffdiffWorker::tr("Test for Diff. Expression with Cuffdiff"),
        "Cuffdiff takes a transcript file as input, along with two or"
        " more fragment alignments (e.g. in SAM format) for two or more samples."
        " It produces a number of output files that contain"
        " test results for changes in expression at the level of transcripts,"
        " primary transcripts, and genes. It also tracks changes in"
        " the relative abundance of transcripts sharing a common transcription"
        " start site, and in the relative abundances of the primary transcripts"
        " of each gene. Tracking the former allows one to see changes in splicing,"
        " and the latter lets one see changes in relative promoter use within"
        " a gene.");

    { // Define parameters of the element
        Descriptor timeSeriesAnalysis(TIME_SERIES_ANALYSIS,
            CuffdiffWorker::tr("Time series analysis"),
            CuffdiffWorker::tr("If set to <i>True</i>, instructs Cuffdiff to analyze"
            " the provided samples as a time series, rather than testing for differences"
            " between all pairs of samples. Samples should be provided in increasing time"
            " order."));

        Descriptor upperQuartileNorm(UPPER_QUARTILE_NORM,
            CuffdiffWorker::tr("Upper quartile norm"),
            CuffdiffWorker::tr("If set to <i>True</i>, normalizes by the upper quartile"
            " of the number of fragments mapping to individual loci instead of the total"
            " number of sequenced fragments. This can improve robustness of differential"
            " expression calls for less abundant genes and transcripts."));

        Descriptor hitsNorm(HITS_NORM,
            CuffdiffWorker::tr("Hits norm"),
            CuffdiffWorker::tr("Instructs how to count all fragments. <i>Total</i> specifies"
            " to count all fragments, including those not compatible with any reference"
            " transcript, towards the number of mapped fragments used in the FPKM denominator."
            " <i>Compatible</i> specifies to use only compatible fragments."
            " Selecting <i>Compatible</i> is generally recommended in Cuffdiff to reduce"
            " certain types of bias caused by differential amounts of ribosomal reads"
            " which can create the impression of falsely differentially expressed genes."));

        Descriptor fragBiasCorrect(FRAG_BIAS_CORRECT,
            CuffdiffWorker::tr("Frag bias correct"),
            CuffdiffWorker::tr("Providing the sequences your reads were mapped to instructs"
            " Cuffdiff to run bias detection and correction algorithm which can significantly"
            " improve accuracy of transcript abundance estimates."));

        Descriptor multiReadCorrect(MULTI_READ_CORRECT,
            CuffdiffWorker::tr("Multi read correct"),
            CuffdiffWorker::tr("Do an initial estimation procedure to more accurately weight"
            " reads mapping to multiple locations in the genome."));

        Descriptor libraryType(LIBRARY_TYPE,
            CuffdiffWorker::tr("Library type"),
            CuffdiffWorker::tr("Specifies RNA-Seq protocol"));

        Descriptor maskFile(MASK_FILE,
            CuffdiffWorker::tr("Mask file"),
            CuffdiffWorker::tr("Ignore all reads that could have come from transcripts"
            " in this file. It is recommended to include any annotated rRNA, mitochondrial"
            " transcripts other abundant transcripts you wish to ignore in your analysis"
            " in this file. Due to variable efficiency of mRNA enrichment methods and"
            " rRNA depletion kits, masking these transcripts often improves the overall"
            " robustness of transcript abundance estimates."));

        Descriptor minAlignmentCount(MIN_ALIGNMENT_COUNT,
            CuffdiffWorker::tr("Min alignment count"),
            CuffdiffWorker::tr("The minimum number of alignments in a locus for needed"
            " to conduct significance testing on changes in that locus observed between"
            " samples. If no testing is performed, changes in the locus are deemed not"
            " significant, and the locus' observed changes don't contribute to correction"
            " for multiple testing."));

        Descriptor fdr(FDR,
            CuffdiffWorker::tr("FDR"),
            CuffdiffWorker::tr("The allowed false discovery rate used in testing"));

        Descriptor maxMleIterations(MAX_MLE_ITERATIONS,
            CuffdiffWorker::tr("Max MLE iterations"),
            CuffdiffWorker::tr("Sets the number of iterations allowed during maximum"
            " likelihood estimation of abundances"));

        Descriptor emitCountTables(EMIT_COUNT_TABLES,
            CuffdiffWorker::tr("Emit count tables"),
            CuffdiffWorker::tr("Include information about the fragment counts, fragment"
            " count variances, and fitted variance model into the report"));

        Descriptor extToolPath(EXT_TOOL_PATH,
            CuffdiffWorker::tr("Cuffdiff tool path"),
            CuffdiffWorker::tr("The path to the Cuffdiff external tool in UGENE"));

        Descriptor tmpDir(TMP_DIR_PATH,
            CuffdiffWorker::tr("Temporary directory"),
            CuffdiffWorker::tr("The directory for temporary files"));

        attributes << new Attribute(timeSeriesAnalysis, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attributes << new Attribute(upperQuartileNorm, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attributes << new Attribute(hitsNorm, BaseTypes::NUM_TYPE(), false, QVariant(1));
        attributes << new Attribute(fragBiasCorrect, BaseTypes::STRING_TYPE(), false, QVariant(""));
        attributes << new Attribute(multiReadCorrect, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attributes << new Attribute(libraryType, BaseTypes::NUM_TYPE(), false, QVariant(0));
        attributes << new Attribute(maskFile, BaseTypes::STRING_TYPE(), false, QVariant(""));
        attributes << new Attribute(minAlignmentCount, BaseTypes::NUM_TYPE(), false, QVariant(10));
        attributes << new Attribute(fdr, BaseTypes::NUM_TYPE(), false, QVariant(0.05));
        attributes << new Attribute(maxMleIterations, BaseTypes::NUM_TYPE(), false, QVariant(5000));
        attributes << new Attribute(emitCountTables, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attributes << new Attribute(extToolPath, BaseTypes::STRING_TYPE(), true, QVariant(L10N::defaultStr()));
        attributes << new Attribute(tmpDir, BaseTypes::STRING_TYPE(), true, QVariant(L10N::defaultStr()));
    }

    { // Define ports of the element
        Descriptor assemblyDesc(BasePorts::IN_ASSEMBLY_PORT_ID(),
            CuffdiffWorker::tr("Assembly"),
            CuffdiffWorker::tr("RNA-Seq reads assemblies"));
        Descriptor annsDesc(BasePorts::IN_ANNOTATIONS_PORT_ID(),
            CuffdiffWorker::tr("Annotations"),
            CuffdiffWorker::tr("Transcript annotations"));
        Descriptor outDesc(OUT_PORT,
            CuffdiffWorker::tr("Cuffdiff output"),
            CuffdiffWorker::tr("Annotations with output information: FPKM"
            " tracking, differential expression and splicing tests,"
            "differential coding and differential promoter use"));

        Descriptor splicing(SPLICING_DIFF_SLOT,
            CuffdiffWorker::tr("Splicing"),
            CuffdiffWorker::tr("Lists the amount of overloading detected among"
            " its isoforms, i.e. how much differential splicing exists between"
            " isoforms processed from a single primary transcript"));
        Descriptor promoters(PROMOTERS_DIFF_SLOT,
            CuffdiffWorker::tr("Promoters"),
            CuffdiffWorker::tr("Lists the amount of overloading detected among"
            " its primary transcripts, i.e. how much differential promoter use"
            " exists between samples"));
        Descriptor cdsDiff(CDS_DIFF_SLOT,
            CuffdiffWorker::tr("Differential coding"),
            CuffdiffWorker::tr("The amount of overloading detected among its"
            " coding sequences, i.e. how much differential CDS output exists"
            " between samples"));
        Descriptor cdsExp(CDS_EXP_SLOT,
            CuffdiffWorker::tr("Coding sequence differential FPKM"),
            CuffdiffWorker::tr("Tests differences in the summed FPKM of"
            " transcripts sharing each p_id independent of tss_id"));
        Descriptor cdsFpkm(CDS_FPKM_SLOT,
            CuffdiffWorker::tr("Coding sequence FPKMs"),
            CuffdiffWorker::tr("Tracks the summed FPKM of transcripts sharing"
            " each p_id, independent of tss_id"));
        Descriptor tssExp(TSS_EXP_SLOT,
            CuffdiffWorker::tr("Primary transcript differential FPKM"),
            CuffdiffWorker::tr("Tests differences in the summed FPKM of"
            " transcripts sharing each tss_id"));
        Descriptor tssFpkm(TSS_FPKM_SLOT,
            CuffdiffWorker::tr("Primary transcript FPKMs"),
            CuffdiffWorker::tr("Tracks the summed FPKM of transcripts sharing"
            " each tss_id"));
        Descriptor genesExp(GENES_EXP_SLOT,
            CuffdiffWorker::tr("Gene differential FPKM"),
            CuffdiffWorker::tr("Tests difference sin the summed FPKM of"
            " transcripts sharing each gene_id"));
        Descriptor genesFpkm(GENES_FPKM_SLOT,
            CuffdiffWorker::tr("Gene FPKMs"),
            CuffdiffWorker::tr("Tracks the summed FPKM of transcripts sharing"
            " each gene_id"));
        Descriptor isoExp(ISOMORFS_EXP_SLOT,
            CuffdiffWorker::tr("Transcript differential FPKM"),
            CuffdiffWorker::tr("Transcript differential FPKM"));
        Descriptor isoFpkm(ISOMORFS_FPKM_SLOT,
            CuffdiffWorker::tr("Transcript FPKMs"),
            CuffdiffWorker::tr("Contains the estimated isoform-level expression values"));

        QMap<Descriptor, DataTypePtr> assemblyTypeMap;
        assemblyTypeMap[BaseSlots::ASSEMBLY_SLOT()] = BaseTypes::ASSEMBLY_TYPE();
        DataTypePtr assemblyType(new MapDataType(BasePorts::IN_ASSEMBLY_PORT_ID(), assemblyTypeMap));

        QMap<Descriptor, DataTypePtr> annotationsTypeMap;
        annotationsTypeMap[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
        DataTypePtr annsType(new MapDataType(BasePorts::IN_ASSEMBLY_PORT_ID(), annotationsTypeMap));

        QMap<Descriptor, DataTypePtr> outputTypeMap;
        outputTypeMap[SPLICING_DIFF_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outputTypeMap[PROMOTERS_DIFF_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outputTypeMap[CDS_DIFF_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outputTypeMap[CDS_EXP_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outputTypeMap[CDS_FPKM_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outputTypeMap[TSS_EXP_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outputTypeMap[TSS_FPKM_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outputTypeMap[GENES_EXP_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outputTypeMap[GENES_FPKM_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outputTypeMap[ISOMORFS_EXP_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outputTypeMap[ISOMORFS_FPKM_SLOT] = BaseTypes::ANNOTATION_TABLE_TYPE();
        DataTypePtr outType(new MapDataType(OUT_PORT, outputTypeMap));

        portDescriptors << new PortDescriptor(assemblyDesc, assemblyType, true, false, IntegralBusPort::BLIND_INPUT);
        portDescriptors << new PortDescriptor(annsDesc, annsType, true);
        portDescriptors << new PortDescriptor(outDesc, outType, false, true);
    }

    // Create the actor prototype
    ActorPrototype* proto = new IntegralBusActorPrototype(cuffdiffDescriptor,
        portDescriptors,
        attributes);

    // Values range of some parameters
    QMap<QString, PropertyDelegate*> delegates;

    {
        QVariantMap vm;
        vm[CuffdiffWorker::tr("Total")] = CuffdiffSettings::Total;
        vm[CuffdiffWorker::tr("Compatible")] = CuffdiffSettings::Compatible;
        delegates[HITS_NORM] = new ComboBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["Standard Illumina"] = CuffdiffSettings::StandardIllumina;
        vm["dUTP, NSR, NNSR"] = CuffdiffSettings::dUTP_NSR_NNSR;
        vm["Ligation, Standard SOLiD"] = CuffdiffSettings::Ligation_StandardSOLiD;
        delegates[LIBRARY_TYPE] = new ComboBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[MIN_ALIGNMENT_COUNT] = new SpinBoxDelegate(vm);
        delegates[MAX_MLE_ITERATIONS] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["singleStep"] = 0.05;
        delegates[FDR] = new DoubleSpinBoxDelegate(vm);
    }

    delegates[FRAG_BIAS_CORRECT] = new URLDelegate("", "", false);
    delegates[MASK_FILE] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), "", false);
    delegates[EXT_TOOL_PATH] = new URLDelegate("", "executable", false);
    delegates[TMP_DIR_PATH] = new URLDelegate("", "TmpDir", false, true);

    // Init and register the actor prototype
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new CuffdiffPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(
        BaseActorCategories::CATEGORY_RNA_SEQ(),
        proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new CuffdiffWorkerFactory());
}


/*****************************
 * CuffdiffPrompter
 *****************************/
CuffdiffPrompter::CuffdiffPrompter(Actor* parent)
    : PrompterBase<CuffdiffPrompter>(parent)
{
}


QString CuffdiffPrompter::composeRichDoc()
{
    QString result = CuffdiffWorker::tr(
        "Searches for significant changes in transcript expression,"
        " splicing and promoter use.");

    return result;
}


/*****************************
 * CuffdiffWorker
 *****************************/
CuffdiffWorker::CuffdiffWorker(Actor *actor)
: BaseWorker(actor, false), inAssembly(NULL), inTranscript(NULL), output(NULL)
{
}

void CuffdiffWorker::init() {
    QString extToolPath = actor->getParameter(EXT_TOOL_PATH)->getAttributeValue<QString>(context);
    if (QString::compare(extToolPath, "default", Qt::CaseInsensitive) != 0) {
        AppContext::getExternalToolRegistry()->getByName(CUFFDIFF_TOOL_NAME)->setPath(extToolPath);
    }

    inAssembly = ports[BasePorts::IN_ASSEMBLY_PORT_ID()];
    inTranscript = ports[BasePorts::IN_ANNOTATIONS_PORT_ID()];
    output = ports[OUT_PORT];
}

bool CuffdiffWorker::isReady() {
    if (isDone()) {
        return false;
    }
    if (inAssembly->hasMessage()) {
        return true;
    } else if (!inAssembly->isEnded()) {
        return false;
    }
    return (inTranscript->hasMessage() || inTranscript->isEnded());
}

Task * CuffdiffWorker::tick() {
    while (inAssembly->hasMessage()) {
        takeAssembly();
    }
    if (!inAssembly->isEnded()) {
        return NULL;
    }

    if (inTranscript->hasMessage()) {
        Task *t = new CuffdiffSupportTask(takeSettings());
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    } else if (inTranscript->isEnded()) {
        output->setEnded();
        setDone();
    }
    return NULL;
}

void CuffdiffWorker::sl_taskFinished() {
    CuffdiffSupportTask *t = qobject_cast<CuffdiffSupportTask*>(sender());
    if (Task::State_Finished != t->getState()) {
        return;
    }
    Message m(output->getBusType(), createMessageData(t));
    output->put(m);
}

void CuffdiffWorker::cleanup() {
    assemblies.clear();
}

CuffdiffSettings CuffdiffWorker::scanParameters() const {
    CuffdiffSettings result;
    result.timeSeriesAnalysis = param<bool>(TIME_SERIES_ANALYSIS);
    result.upperQuartileNorm = param<bool>(UPPER_QUARTILE_NORM);
    result.hitsNorm = CuffdiffSettings::HitsNorm(param<int>(HITS_NORM));
    result.fragBiasCorrect = param<QString>(FRAG_BIAS_CORRECT);
    result.multiReadCorrect = param<bool>(MULTI_READ_CORRECT);
    result.libraryType = CuffdiffSettings::Library(param<int>(LIBRARY_TYPE));
    result.maskFile = param<QString>(MASK_FILE);
    result.minAlignmentCount = param<int>(MIN_ALIGNMENT_COUNT);
    result.fdr = param<double>(FDR);
    result.maxMleIterations = param<int>(MAX_MLE_ITERATIONS);
    result.emitCountTables = param<bool>(EMIT_COUNT_TABLES);
    result.workingDir = param<QString>(TMP_DIR_PATH);
    return result;
}

CuffdiffSettings CuffdiffWorker::takeSettings() {
    CuffdiffSettings result = scanParameters();

    Message m = getMessageAndSetupScriptValues(inTranscript);
    QVariantMap data = m.getData().toMap();
    SAFE_POINT(data.contains(BaseSlots::ANNOTATION_TABLE_SLOT().getId()),
        "No annotations in a message", result);
    QVariant annsVar = data[BaseSlots::ANNOTATION_TABLE_SLOT().getId()];

    result.assemblies = assemblies;
    result.transcript = annsVar.value<QList<SharedAnnotationData> >();
    result.storage = context->getDataStorage();

    return result;
}

void CuffdiffWorker::takeAssembly() {
    Message m = getMessageAndSetupScriptValues(inAssembly);
    QVariantMap data = m.getData().toMap();
    SAFE_POINT(data.contains(BaseSlots::ASSEMBLY_SLOT().getId()),
        "No assembly in a message", );
    SharedDbiDataHandler id = data[BaseSlots::ASSEMBLY_SLOT().getId()].value<SharedDbiDataHandler>();
    assemblies << id;
}

static void insert(QVariantMap &map, const QString &slotId,
    const QList<SharedAnnotationData> &data) {
    map[slotId] = qVariantFromValue<QList<SharedAnnotationData> >(data);
}

QVariantMap CuffdiffWorker::createMessageData(CuffdiffSupportTask *task) const {
    QVariantMap result;
    CuffdiffResult cr = task->takeResult();

    insert(result, SPLICING_DIFF_SLOT, cr.splicing);
    insert(result, PROMOTERS_DIFF_SLOT, cr.promoters);
    insert(result, CDS_DIFF_SLOT, cr.cdsDiff);
    insert(result, CDS_EXP_SLOT, cr.cdsExp);
    insert(result, CDS_FPKM_SLOT, cr.cdsFpkm);
    insert(result, TSS_EXP_SLOT, cr.tssExp);
    insert(result, TSS_FPKM_SLOT, cr.tssFpkm);
    insert(result, GENES_EXP_SLOT, cr.genesExp);
    insert(result, GENES_FPKM_SLOT, cr.genesFpkm);
    insert(result, ISOMORFS_EXP_SLOT, cr.isomorfsExp);
    insert(result, ISOMORFS_FPKM_SLOT, cr.isomorfsFpkm);
    return result;
}

} // namespace LocalWorkflow
} // namespace U2
