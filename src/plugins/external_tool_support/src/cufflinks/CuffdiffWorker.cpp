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

#include "CuffdiffWorker.h"

#include <U2Core/L10n.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>


namespace U2 {
namespace LocalWorkflow {

/*****************************
 * CuffdiffWorkerFactory
 *****************************/
const QString CuffdiffWorkerFactory::ACTOR_ID("cuffdiff");

const QString TIME_SERIES_ANALYSIS("time-series-analysis");
const QString UPPER_QUARTILE_NORM("upper-quartile-norm");
const QString HITS_NORM("hits-norm");
const QString FRAG_BIAS_CORRECT("frag-bias-correct");
const QString MULTI_READ_CORRECT("multi-read-correct");
const QString LIBRARY_TYPE("library-type");
const QString MASK_FILE("mask-file");
const QString MIN_ALIGNMENT_COUNT("min-alignment-count");
const QString FDR("fdr");
const QString MAX_MLE_ITERATIONS("max-mle-iterations");
const QString EMIT_COUNT_TABLES("emit-count-tables");
const QString EXT_TOOL_PATH("path");
const QString TMP_DIR_PATH("tmp-dir");


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

    // Define parameters of the element
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

    // Create the actor prototype
    ActorPrototype* proto = new IntegralBusActorPrototype(cuffdiffDescriptor,
        portDescriptors,
        attributes);

    // Values range of some parameters
    QMap<QString, PropertyDelegate*> delegates;

    {
        QVariantMap vm;
        vm[CuffdiffWorker::tr("Total")] = 0;
        vm[CuffdiffWorker::tr("Compatible")] = 1;
        delegates[HITS_NORM] = new ComboBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["Standard Illumina"] = 0;
        vm["dUTP, NSR, NNSR"] = 1;
        vm["Ligation, Standard SOLiD"] = 2;
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
CuffdiffWorker::CuffdiffWorker(Actor* actor)
    : BaseWorker(actor),
      input(NULL),
      output(NULL)
{
}


void CuffdiffWorker::init()
{
}


Task* CuffdiffWorker::tick()
{
    return NULL;
}


void CuffdiffWorker::sl_taskFinished()
{
}


void CuffdiffWorker::cleanup()
{
}


} // namespace LocalWorkflow
} // namespace U2
