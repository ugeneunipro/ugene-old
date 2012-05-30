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

#include "CufflinksWorker.h"

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
 * CufflinksWorkerFactory
 *****************************/
const QString CufflinksWorkerFactory::ACTOR_ID("cufflinks");

const QString REF_ANNOTATION("ref-annotation");
const QString RABT_ANNOTATION("rabt-annotation");
const QString LIBRARY_TYPE("library-type");
const QString MASK_FILE("mask-file");
const QString MULTI_READ_CORRECT("multi-read-correct");
const QString MIN_ISOFORM_FRACTION("min-isoform-fraction");
const QString FRAG_BIAS_CORRECT("frag-bias-correct");
const QString PRE_MRNA_FRACTION("pre-mrna-fraction");
const QString EXT_TOOL_PATH("path");
const QString TMP_DIR_PATH("tmp-dir");


void CufflinksWorkerFactory::init()
{
    QList<PortDescriptor*> portDescriptors;
    QList<Attribute*> attributes;

    // Description of the element
    Descriptor cufflinksDescriptor(ACTOR_ID,
        CufflinksWorker::tr("Assemble Transcripts with Cufflinks"),
        CufflinksWorker::tr("Cufflinks accepts aligned RNA-Seq reads"
        " and assembles the alignments into a parsimonious set of"
        " transcripts. Cufflinks then estimates the relative abundances"
        " of these transcripts based on how many reads support each one,"
        " taking into account biases in library preparation protocols."));

    // Define parameters of the element
    Descriptor refAnnotation(REF_ANNOTATION,
        CufflinksWorker::tr("Reference annotation"),
        CufflinksWorker::tr("Tells Cufflinks to use the supplied reference"
        " annotation to estimate isoform expression. Cufflinks will not"
        " assemble novel transcripts and the program will ignore alignments"
        " not structurally compatible with any reference transcript"));

    Descriptor rabtAnnotation(RABT_ANNOTATION,
        CufflinksWorker::tr("RABT annotation"),
        CufflinksWorker::tr("Tells Cufflinks to use the supplied reference"
        " annotation to guide Reference Annotation Based Transcript (RABT) assembly."
        " Reference transcripts will be tiled with faux-reads to provide additional"
        " information in assembly. Output will include all reference transcripts"
        " as well as any novel genes and isoforms that are assembled."));

    Descriptor libraryType(LIBRARY_TYPE,
        CufflinksWorker::tr("Library type"),
        CufflinksWorker::tr("Specifies RNA-Seq protocol"));

    Descriptor maskFile(MASK_FILE,
        CufflinksWorker::tr("Mask files"),
        CufflinksWorker::tr("Ignore all reads that could have come from transcripts"
        " in this file. It is recommended to include any annotated rRNA, mitochondrial"
        " transcripts other abundant transcripts you wish to ignore in your analysis"
        " in this file. Due to variable efficiency of mRNA enrichment methods and rRNA"
        " depletion kits, masking these transcripts often improves the overall robustness"
        " of transcript abundance estimates."));

    Descriptor multiReadCorrect(MULTI_READ_CORRECT,
        CufflinksWorker::tr("Multi-read correct"),
        CufflinksWorker::tr("Tells Cufflinks to do an initial estimation procedure to more"
        " accurately weight reads mapping to multiple locations in the genome."));

    Descriptor minIsoformFraction(MIN_ISOFORM_FRACTION,
        CufflinksWorker::tr("Min isoform fraction"),
        CufflinksWorker::tr("After calculating isoform abundance for a gene, Cufflinks"
        " filters out transcripts that it believes are very low abundance, because"
        " isoforms expressed at extremely low levels often cannot reliably be assembled,"
        " and may even be artifacts of incompletely spliced precursors of processed transcripts."
        " This parameter is also used to filter out introns that have far fewer spliced"
        " alignments supporting them."));

    Descriptor fragBiasCorrect(FRAG_BIAS_CORRECT,
        CufflinksWorker::tr("Frag bias correct"),
        CufflinksWorker::tr("Providing Cufflinks with a multifasta file via this option"
        " instructs it to run the bias detection and correction algorithm which can"
        " significantly improve accuracy of transcript abundance estimates."));

    Descriptor preMrnaFraction(PRE_MRNA_FRACTION,
        CufflinksWorker::tr("Pre-mRNA fraction"),
        CufflinksWorker::tr("Some RNA-Seq protocols produce a significant amount of reads"
        " that originate from incompletely spliced transcripts, and these reads can"
        " confound the assembly of fully spliced mRNAs. Cufflinks uses this parameter"
        " to filter out alignments that lie within the intronic intervals implied"
        " by the spliced alignments. The minimum depth of coverage in the intronic"
        " region covered by the alignment is divided by the number of spliced reads,"
        " and if the result is lower than this parameter value, the intronic"
        " alignments are ignored."));

    Descriptor extToolPath(EXT_TOOL_PATH,
        CufflinksWorker::tr("Cufflinks tool path"),
        CufflinksWorker::tr("The path to the Cufflinks external tool in UGENE"));

    Descriptor tmpDir(TMP_DIR_PATH,
        CufflinksWorker::tr("Temporary directory"),
        CufflinksWorker::tr("The directory for temporary files"));

    attributes << new Attribute(refAnnotation, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(rabtAnnotation, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(libraryType, BaseTypes::NUM_TYPE(), false, QVariant(0));
    attributes << new Attribute(maskFile, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(multiReadCorrect, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(minIsoformFraction, BaseTypes::NUM_TYPE(), false, QVariant(0.1));
    attributes << new Attribute(fragBiasCorrect, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(preMrnaFraction, BaseTypes::NUM_TYPE(), false, QVariant(0.15));
    attributes << new Attribute(extToolPath, BaseTypes::STRING_TYPE(), true, QVariant(L10N::defaultStr()));
    attributes << new Attribute(tmpDir, BaseTypes::STRING_TYPE(), true, QVariant(L10N::defaultStr()));

    // Create the actor prototype
    ActorPrototype* proto = new IntegralBusActorPrototype(cufflinksDescriptor,
        portDescriptors,
        attributes);

    // Values range of some parameters
    QMap<QString, PropertyDelegate*> delegates;

    {
        QVariantMap vm;
        vm["Standard Illumina"] = 0;
        vm["dUTP, NSR, NNSR"] = 1;
        vm["Ligation, Standard SOLiD"] = 2;
        delegates[LIBRARY_TYPE] = new ComboBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0.0;
        vm["maximum"] = 1.0;
        vm["singleStep"] = 0.1;
        delegates[MIN_ISOFORM_FRACTION] = new DoubleSpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0.0;
        vm["maximum"] = 1.0;
        vm["singleStep"] = 0.05;
        delegates[PRE_MRNA_FRACTION] = new DoubleSpinBoxDelegate(vm);
    }

    delegates[REF_ANNOTATION] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), "", false);
    delegates[RABT_ANNOTATION] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), "", false);
    delegates[MASK_FILE] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), "", false);
    delegates[FRAG_BIAS_CORRECT] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), "", false);
    delegates[EXT_TOOL_PATH] = new URLDelegate("", "executable", false);
    delegates[TMP_DIR_PATH] = new URLDelegate("", "TmpDir", false, true);

    // Init and register the actor prototype
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new CufflinksPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(
        BaseActorCategories::CATEGORY_RNA_SEQ(),
        proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new CufflinksWorkerFactory());
}


/*****************************
 * CufflinksPrompter
 *****************************/
CufflinksPrompter::CufflinksPrompter(Actor* parent)
    : PrompterBase<CufflinksPrompter>(parent)
{
}


QString CufflinksPrompter::composeRichDoc()
{
    QString result = "Assembles transcripts and estimates their abundances.";

    return result;
}


/*****************************
 * CufflinksWorker
 *****************************/
CufflinksWorker::CufflinksWorker(Actor* actor)
    : BaseWorker(actor),
      input(NULL),
      output(NULL)
{
}


void CufflinksWorker::init()
{
}


Task* CufflinksWorker::tick()
{
    return NULL;
}


void CufflinksWorker::sl_taskFinished()
{
}


void CufflinksWorker::cleanup()
{
}


} // namespace LocalWorkflow
} // namespace U2
