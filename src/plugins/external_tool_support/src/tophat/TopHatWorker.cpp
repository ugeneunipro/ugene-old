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

#include "TopHatWorker.h"

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
 * TopHatWorkerFactory
 *****************************/
const QString TopHatWorkerFactory::ACTOR_ID("tophat");

const QString BOWTIE_INDEX_DIR("bowtie-index-dir");
const QString BOWTIE_INDEX_BASENAME("bowtie-index-basename");
const QString REF_SEQ("ref-seq");
const QString MATE_INNER_DISTANCE("mate-inner-distance");
const QString MATE_STANDARD_DEVIATION("mate-standard-deviation");
const QString LIBRARY_TYPE("library-type");
const QString NO_NOVEL_JUNCTIONS("no-novel-junctions");
const QString RAW_JUNCTIONS("raw-junctions");
const QString KNOWN_TRANSCRIPT("known-transcript");
const QString MAX_MULTIHITS("max-multihits");
const QString SEGMENT_LENGTH("segment-length");
const QString DISCORDANT_PAIR_ALIGNMENTS("discordant-pair-alignments");
const QString FUSION_SEARCH("fusion-search");
const QString TRANSCRIPTOME_ONLY("transcriptome-only");
const QString TRANSCRIPTOME_MAX_HITS("transcriptome-max-hits");
const QString PREFILTER_MULTIHITS("prefilter-multihits");
const QString MIN_ANCHOR_LENGTH("min-anchor-length");
const QString SPLICE_MISMATCHES("splice-mismatches");
const QString TRANSCRIPTOME_MISMATCHES("transcriptome-mismatches");
const QString GENOME_READ_MISMATCHES("genome-read-mismatches");
const QString READ_MISMATCHES("read-mismatches");
const QString SEGMENT_MISMATCHES("segment-mismatches");
const QString SOLEXA_1_3_QUALS("solexa-1-3-quals");
const QString BOWTIE_VERSION("bowtie-version");
const QString BOWTIE_N_MODE("bowtie-n-mode");
const QString BOWTIE_TOOL_PATH("bowtie-tool-path");
const QString EXT_TOOL_PATH("path");
const QString TMP_DIR_PATH("temp-dir");


void TopHatWorkerFactory::init()
{
    QList<PortDescriptor*> portDescriptors;
    QList<Attribute*> attributes;

    // Description of the element
    Descriptor topHatDescriptor(ACTOR_ID,
        TopHatWorker::tr("Find Splice Junctions with TopHat"),
        TopHatWorker::tr("TopHat is a fast splice junction mapper for RNA-Seq"
            " reads. It aligns RNA-Seq reads to mammalian-sized genomes"
            " using the ultra high-throughput short read aligner Bowtie,"
            " and then analyzes the mapping results to identify splice"
            " junctions between exons."));

    // Define parameters of the element
    Descriptor bowtieIndexDir(BOWTIE_INDEX_DIR,
        TopHatWorker::tr("Bowtie index directory"),
        TopHatWorker::tr("The directory with the Bowtie index for the reference sequence."
        " It is required to either input a directory and a basename of a Bowtie index,"
        " or a reference sequence."));

    Descriptor bowtieIndexBasename(BOWTIE_INDEX_BASENAME,
        TopHatWorker::tr("Bowtie index basename"),
        TopHatWorker::tr("The basename of the Bowtie index for the reference sequence."
        " It is required to either input a directory and a basename of a Bowtie index,"
        " or a reference sequence."));

    Descriptor refSeq(REF_SEQ,
        TopHatWorker::tr("Reference sequence"),
        TopHatWorker::tr("The reference sequence for short reads."
        " It is required to either input a directory and a basename of a Bowtie index,"
        " or a reference sequence. Note that the Bowtie index parameters have higher priority"
        " than this parameter, i.e. if they are specified, this parameter is ignored."));

    Descriptor mateInnerDistance(MATE_INNER_DISTANCE,
        TopHatWorker::tr("Mate inner distance"),
        TopHatWorker::tr("The expected (mean) inner distance between mate pairs"));

    Descriptor mateStandardDeviation(MATE_STANDARD_DEVIATION,
        TopHatWorker::tr("Mate standard deviation"),
        TopHatWorker::tr("The standard deviation for the distribution on inner distances between mate pairs"));

    Descriptor libraryType(LIBRARY_TYPE,
        TopHatWorker::tr("Library type"),
        TopHatWorker::tr("Specifies RNA-Seq protocol"));

    Descriptor noNovelJunctions(NO_NOVEL_JUNCTIONS,
        TopHatWorker::tr("No novel junctions"),
        TopHatWorker::tr("Only look for reads across junctions indicated in"
        " the supplied GFF or junctions file. This parameter is ignored"
        " if <i>Raw junctions</i> or <i>Known transcript file</i> is not set."));

    Descriptor rawJunctions(RAW_JUNCTIONS,
        TopHatWorker::tr("Raw junctions"),
        TopHatWorker::tr("The list of raw junctions"));

    Descriptor knownTranscript(KNOWN_TRANSCRIPT,
        TopHatWorker::tr("Known transcript file"),
        TopHatWorker::tr("A set of gene model annotations and/or known transcripts"));

    Descriptor maxMultihits(MAX_MULTIHITS,
        TopHatWorker::tr("Max multihits"),
        TopHatWorker::tr("Instructs TopHat to allow up to this many alignments to"
        " the reference for a given read, and suppresses all alignments for"
        " reads with more than this many alignments."));

    Descriptor segmentLength(SEGMENT_LENGTH,
        TopHatWorker::tr("Segment length"),
        TopHatWorker::tr("Each read is cut up into segments, each at least this long."
        " These segments are mapped independently."));

    Descriptor discordantPairAlignments(DISCORDANT_PAIR_ALIGNMENTS,
        TopHatWorker::tr("Report discordant pair alignments"),
        TopHatWorker::tr("This option will allow mate pairs to map to different"
        " chromosomes, distant places on the same chromosome, or on the same strand."));

    Descriptor fusionSearch(FUSION_SEARCH,
        TopHatWorker::tr("Fusion search"),
        TopHatWorker::tr("Turn on fusion mapping"));

    Descriptor transcriptomeOnly(TRANSCRIPTOME_ONLY,
        TopHatWorker::tr("Transcriptome only"),
        TopHatWorker::tr("Only align the reads to the transcriptome and report only"
        " those mappings as genomic mappings"));

    Descriptor transcriptomeMaxHits(TRANSCRIPTOME_MAX_HITS,
        TopHatWorker::tr("Transcriptome max hits"),
        TopHatWorker::tr("Maximum number of mappings allowed for a read, when aligned"
        " to the transcriptome (any reads found with more than this number of"
        " mappings will be discarded)"));

    Descriptor prefilterMultihits(PREFILTER_MULTIHITS,
        TopHatWorker::tr("Prefilter multihits"),
        TopHatWorker::tr("When mapping reads on the transcriptome, some repetitive or"
        " low complexity reads that would be discarded in the context of the genome"
        " may appear to align to the transcript sequences and thus may end up"
        " reported as mapped to those genes only. This option directs TopHat"
        " to first align the reads to the whole genome in order to determine"
        " and exclude such multi-mapped reads (according to the value of the"
        " <i>Max multihits</i> option)."));

    Descriptor minAnchorLength(MIN_ANCHOR_LENGTH,
        TopHatWorker::tr("Min anchor length"),
        TopHatWorker::tr("The <i>anchor length</i>. TopHat will report junctions"
        " spanned by reads with at least this many bases on each side of the"
        " junction. Note that individual spliced alignments may span a junction"
        " with fewer than this many bases on one side. However, every junction"
        " involved in spliced alignments is supported by at least one read with"
        " this many bases on each side."));

    Descriptor spliceMismatches(SPLICE_MISMATCHES,
        TopHatWorker::tr("Splice mismatches"),
        TopHatWorker::tr("The maximum number of mismatches that may appear in"
        " the <i>anchor</i> region of a spliced alignment"));

    Descriptor transcriptomeMismatches(TRANSCRIPTOME_MISMATCHES,
        TopHatWorker::tr("Transcriptome mismatches"),
        TopHatWorker::tr("The maximum number of mismatches allowed when reads"
        " are aligned to the transcriptome"));

    Descriptor genomeReadMismatches(GENOME_READ_MISMATCHES,
        TopHatWorker::tr("Genome read mismatches"),
        TopHatWorker::tr("When whole reads are first mapped on the genome,"
        " this many mismatches in each read alignment are allowed."));

    Descriptor readMismatches(READ_MISMATCHES,
        TopHatWorker::tr("Read mismatches"),
        TopHatWorker::tr("Final read alignments having more than these"
        " many mismatches are discarded."));

    Descriptor segmentMismatches(SEGMENT_MISMATCHES,
        TopHatWorker::tr("Segment mismatches"),
        TopHatWorker::tr("Read segments are mapped independently,"
        " allowing up to this many mismatches in each segment"
        " alignment."));

    Descriptor solexa13Quals(SOLEXA_1_3_QUALS,
        TopHatWorker::tr("Solexa 1.3 quals"),
        TopHatWorker::tr("As of the Illumina GA pipeline version 1.3,"
        " quality scores are encoded in Phred-scaled base-64."
        " Use this option for FASTQ files from pipeline 1.3 or later."));

    Descriptor bowtieVersion(BOWTIE_VERSION,
        TopHatWorker::tr("Bowtie version"),
        TopHatWorker::tr("Specifies which Bowtie version should be used"));

    Descriptor bowtieNMode(BOWTIE_N_MODE,
        TopHatWorker::tr("Bowtie -n mode"),
        TopHatWorker::tr("TopHat uses <i>-v</i> in Bowtie for initial"
        " read mapping (the default), but with this option, <i>-n</i>"
        " is used instead. Read segments are always mapped using"
        " <i>-v</i> option."));

    Descriptor bowtieToolPath(BOWTIE_TOOL_PATH,
        TopHatWorker::tr("Bowtie tool path"),
        TopHatWorker::tr("The path to the Bowtie external tool"));

    Descriptor extToolPath(EXT_TOOL_PATH,
        TopHatWorker::tr("TopHat tool path"),
        TopHatWorker::tr("The path to the TopHat external tool in UGENE"));

    Descriptor tmpDir(TMP_DIR_PATH,
        TopHatWorker::tr("Temporary directory"),
        TopHatWorker::tr("The directory for temporary files"));

    attributes << new Attribute(bowtieIndexDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
    attributes << new Attribute(bowtieIndexBasename, BaseTypes::STRING_TYPE(), true, QVariant(""));
    attributes << new Attribute(refSeq, BaseTypes::STRING_TYPE(), true, QVariant(""));
    attributes << new Attribute(mateInnerDistance, BaseTypes::NUM_TYPE(), false, QVariant(200));
    attributes << new Attribute(mateStandardDeviation, BaseTypes::NUM_TYPE(), false, QVariant(20));
    attributes << new Attribute(libraryType, BaseTypes::NUM_TYPE(), false, QVariant(0));
    attributes << new Attribute(noNovelJunctions, BaseTypes::BOOL_TYPE(), false, QVariant(true));
    attributes << new Attribute(rawJunctions, BaseTypes::NUM_TYPE(), false, QVariant());
    attributes << new Attribute(knownTranscript, BaseTypes::NUM_TYPE(), false, QVariant());
    attributes << new Attribute(maxMultihits, BaseTypes::NUM_TYPE(), false, QVariant(20));
    attributes << new Attribute(segmentLength, BaseTypes::NUM_TYPE(), false, QVariant(25));
    attributes << new Attribute(discordantPairAlignments, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(fusionSearch, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(transcriptomeOnly, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(transcriptomeMaxHits, BaseTypes::NUM_TYPE(), false, QVariant(60));
    attributes << new Attribute(prefilterMultihits, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(minAnchorLength, BaseTypes::NUM_TYPE(), false, QVariant(8));
    attributes << new Attribute(spliceMismatches, BaseTypes::NUM_TYPE(), false, QVariant(0));
    attributes << new Attribute(transcriptomeMismatches, BaseTypes::NUM_TYPE(), false, QVariant(1));
    attributes << new Attribute(genomeReadMismatches, BaseTypes::NUM_TYPE(), false, QVariant(2));
    attributes << new Attribute(readMismatches, BaseTypes::NUM_TYPE(), false, QVariant(2));
    attributes << new Attribute(segmentMismatches, BaseTypes::NUM_TYPE(), false, QVariant(2));
    attributes << new Attribute(solexa13Quals, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    attributes << new Attribute(bowtieVersion, BaseTypes::NUM_TYPE(), false, QVariant(1));
    attributes << new Attribute(bowtieNMode, BaseTypes::NUM_TYPE(), false, QVariant(1));
    attributes << new Attribute(bowtieToolPath, BaseTypes::STRING_TYPE(), true, QVariant(L10N::defaultStr()));
    attributes << new Attribute(extToolPath, BaseTypes::STRING_TYPE(), true, QVariant(L10N::defaultStr()));
    attributes << new Attribute(tmpDir, BaseTypes::STRING_TYPE(), true, QVariant(L10N::defaultStr()));

    // Create the actor prototype
    ActorPrototype* proto = new IntegralBusActorPrototype(topHatDescriptor,
        portDescriptors,
        attributes);

    // Values range of some parameters
    QMap<QString, PropertyDelegate*> delegates;

    {
        QVariantMap vm;
        vm[TopHatWorker::tr("Use -n mode")] = 0;
        vm[TopHatWorker::tr("Use -v mode")] = 1;
        delegates[BOWTIE_N_MODE] = new ComboBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm[TopHatWorker::tr("Bowtie1")] = 0;
        vm[TopHatWorker::tr("Bowtie2")] = 1;
        delegates[BOWTIE_VERSION] = new ComboBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = 1020;
        vm["singleStep"] = 1;
        delegates[MATE_INNER_DISTANCE] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[MATE_STANDARD_DEVIATION] = new SpinBoxDelegate(vm);
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
        delegates[MAX_MULTIHITS] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 10;
        vm["maximum"] = 512;
        vm["singleStep"] = 1;
        delegates[SEGMENT_LENGTH] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 1;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[TRANSCRIPTOME_MAX_HITS] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 3;
        vm["maximum"] = INT_MAX;
        vm["singleStep"] = 1;
        delegates[MIN_ANCHOR_LENGTH] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["maximum"] = 2;
        vm["singleStep"] = 1;
        delegates[SPLICE_MISMATCHES] = new SpinBoxDelegate(vm);
    }
    {
        QVariantMap vm;
        vm["minimum"] = 0;
        vm["maximum"] = 3;
        vm["singleStep"] = 1;
        delegates[TRANSCRIPTOME_MISMATCHES] = new SpinBoxDelegate(vm);
        delegates[GENOME_READ_MISMATCHES] = new SpinBoxDelegate(vm);
        delegates[READ_MISMATCHES] = new SpinBoxDelegate(vm);
        delegates[SEGMENT_MISMATCHES] = new SpinBoxDelegate(vm);
    }

    delegates[BOWTIE_INDEX_DIR] = new URLDelegate("", "", false, true);
    delegates[BOWTIE_TOOL_PATH] = new URLDelegate("", "executable", false);
    delegates[REF_SEQ] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), "", false);
    delegates[EXT_TOOL_PATH] = new URLDelegate("", "executable", false);
    delegates[TMP_DIR_PATH] = new URLDelegate("", "TmpDir", false, true);
    delegates[RAW_JUNCTIONS] = new URLDelegate("", "", false);
    delegates[KNOWN_TRANSCRIPT] = new URLDelegate("", "", false);

    // Init and register the actor prototype
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new TopHatPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(
        BaseActorCategories::CATEGORY_RNA_SEQ(),
        proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new TopHatWorkerFactory());
}


/*****************************
 * TopHatPrompter
 *****************************/
TopHatPrompter::TopHatPrompter(Actor* parent)
    : PrompterBase<TopHatPrompter>(parent)
{
}


QString TopHatPrompter::composeRichDoc()
{
    QString result = TopHatWorker::tr(
        "Finds splice junctions using RNA-Seq data.");

    return result;
}


/*****************************
 * TopHatWorker
 *****************************/
TopHatWorker::TopHatWorker(Actor* actor)
    : BaseWorker(actor),
      input(NULL),
      output(NULL)
{
}


void TopHatWorker::init()
{
}


Task* TopHatWorker::tick()
{
    return NULL;
}


void TopHatWorker::sl_taskFinished()
{
}


void TopHatWorker::cleanup()
{
}

} // namespace LocalWorkflow
} // namespace U2
