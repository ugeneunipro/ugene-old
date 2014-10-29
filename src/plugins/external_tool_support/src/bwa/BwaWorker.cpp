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

#include <U2Algorithm/DnaAssemblyTask.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "BwaSupport.h"
#include "BwaWorker.h"
#include "BwaTask.h"

namespace U2 {
namespace LocalWorkflow {

const QString BWAWorkerFactory::ACTOR_ID("align-reads-with-bwa");

static const QString USE_MISS_PROB = "use-miss-prob";
static const QString MAX_DIFF = "max-diff";
static const QString MISSING_PROB = "missing-prob";
static const QString SEED_LENGTH = "seed-length";
static const QString INDEX_ALG = "index-alg";
static const QString BEST_HITS = "best-hits";
static const QString SCALED_GAP = "scaled-gap";
static const QString LONG_DELETIONS = "long-deletions";
static const QString NON_ITERATIVE = "non-iterative";
static const QString ENABLE_LONG_GAPS = "enable-long-gaps";
static const QString MAX_GAP = "max-gap";
static const QString GAP_EXTENSIONS = "gap-extensions";
static const QString INDEL_OFFSET = "indel-offset";
static const QString MAX_QUEUE_ENTRIES = "max-queue";
static const QString BARCODE_LENGTH = "barcode-length";
static const QString NUM_THREADS = "num-threads";
static const QString MAX_SEED_DIFF = "max-seed";
static const QString MISMATCH_PENALTY = "mistmatch-penalty";
static const QString GAP_OPEN_PENALTY = "gap-open-penalty";
static const QString GAP_EXTENSION_PENALTY = "gap-ext-penalty";
static const QString QUALITY_THRESHOLD = "quality-threshold";


static const QString BASE_BWA_SUBDIR("bwa");
static const QString BASE_BWA_OUTFILE("out.sam");


/************************************************************************/
/* Worker */
/************************************************************************/
BWAWorker::BWAWorker(Actor *p)
: BaseShortReadsAlignerWorker(p, BwaTask::ALGORITHM_BWA_ALN)
{
}

QVariantMap BWAWorker::getCustomParameters() const {
    QMap<QString, QVariant> customSettings;

    if(getValue<bool>(USE_MISS_PROB)) {
        customSettings.insert(BwaTask::OPTION_N, getValue<double>(MISSING_PROB));
    } else {
        customSettings.insert(BwaTask::OPTION_N, getValue<int>(MAX_DIFF));
    }
    customSettings.insert(BwaTask::OPTION_SEED_LENGTH, getValue<int>(SEED_LENGTH));
    customSettings.insert(BwaTask::OPTION_INDEX_ALGORITHM, getValue<QString>(INDEX_ALG));
    customSettings.insert(BwaTask::OPTION_BEST_HITS, getValue<int>(BEST_HITS));
    customSettings.insert(BwaTask::OPTION_MAX_GAP_OPENS, getValue<int>(MAX_GAP));
    customSettings.insert(BwaTask::OPTION_LONG_SCALED_GAP_PENALTY_FOR_LONG_DELETIONS, getValue<bool>(SCALED_GAP));
    customSettings.insert(BwaTask::OPTION_MAX_LONG_DELETION_EXTENSIONS, getValue<int>(LONG_DELETIONS));
    customSettings.insert(BwaTask::OPTION_NON_ITERATIVE_MODE, getValue<bool>(NON_ITERATIVE));
    if(getValue<bool>(ENABLE_LONG_GAPS)) {
        customSettings.insert(BwaTask::OPTION_MAX_GAP_EXTENSIONS, getValue<int>(GAP_EXTENSIONS));
    }

    customSettings.insert(BwaTask::OPTION_INDEL_OFFSET, getValue<int>(INDEL_OFFSET));
    customSettings.insert(BwaTask::OPTION_MAX_QUEUE_ENTRIES, getValue<int>(MAX_QUEUE_ENTRIES));
    customSettings.insert(BwaTask::OPTION_BARCODE_LENGTH, getValue<int>(BARCODE_LENGTH));
    customSettings.insert(BwaTask::OPTION_THREADS, getValue<int>(NUM_THREADS));
    customSettings.insert(BwaTask::OPTION_MAX_SEED_DIFFERENCES, getValue<int>(MAX_SEED_DIFF));
    customSettings.insert(BwaTask::OPTION_MISMATCH_PENALTY, getValue<int>(MISMATCH_PENALTY));
    customSettings.insert(BwaTask::OPTION_GAP_OPEN_PENALTY, getValue<int>(GAP_OPEN_PENALTY));
    customSettings.insert(BwaTask::OPTION_GAP_EXTENSION_PENALTY, getValue<int>(GAP_EXTENSION_PENALTY));
    customSettings.insert(BwaTask::OPTION_QUALITY_THRESHOLD, getValue<int>(QUALITY_THRESHOLD));

    return customSettings;
}

QString BWAWorker::getDefaultFileName() const {
    return BASE_BWA_OUTFILE;
}

QString BWAWorker::getBaseSubdir() const {
    return BASE_BWA_SUBDIR;
}

DnaAssemblyToReferenceTask* BWAWorker::getTask(const DnaAssemblyToRefTaskSettings settings) const {
    return new BwaTask(settings);
}

void BWAWorker::setGenomeIndex(DnaAssemblyToRefTaskSettings& settings) {
    settings.prebuiltIndex = true;
    settings.indexFileName = getValue<QString>(REFERENCE_GENOME);
    settings.refSeqUrl = GUrl(settings.indexFileName).baseFileName();
}

/************************************************************************/
/* Factory */
/************************************************************************/

void BWAWorkerFactory::init() {

    QList<Attribute*> attrs;
    QMap<QString, PropertyDelegate*> delegates;
    addCommonAttributes(attrs, delegates);
    {
         Descriptor useMissProb(USE_MISS_PROB ,
             BWAWorker::tr("Use missing prob"),
             BWAWorker::tr("Use missing prob instead maximum edit distance."));

         Descriptor maxDiff(MAX_DIFF ,
             BWAWorker::tr("Max #diff"),
             BWAWorker::tr("Max #diff (-n)"));

         Descriptor missProb(MISSING_PROB,
             BWAWorker::tr("Missing prob"),
             BWAWorker::tr("Missing prob (-n)"));

         Descriptor seedLength(SEED_LENGTH,
             BWAWorker::tr("Seed length"),
             BWAWorker::tr("Seed length (-l)."));

         Descriptor maxGap(MAX_GAP,
             BWAWorker::tr("Max gap opens"),
             BWAWorker::tr("Max gap opens (-o)."));

         Descriptor indexAlg(INDEX_ALG,
             BWAWorker::tr("Index algorithm"),
             BWAWorker::tr("Index algorithm (-a)."));

         Descriptor bestHits(BEST_HITS,
             BWAWorker::tr("Best hits"),
             BWAWorker::tr("Best hits (-R)."));

         Descriptor longGapPenalty(SCALED_GAP,
             BWAWorker::tr("Long-scaled gap penalty for long deletions"),
             BWAWorker::tr("Long-scaled gap penalty for long deletions (-L)"));

         Descriptor nonIterative(NON_ITERATIVE,
             BWAWorker::tr("Non iterative mode"),
             BWAWorker::tr("Non iterative mode (-N)."));

         Descriptor enableLongGaps(ENABLE_LONG_GAPS,
             BWAWorker::tr("Enable long gaps"),
             BWAWorker::tr("Enable long gaps"));

         Descriptor maxGapExtensions(GAP_EXTENSIONS,
             BWAWorker::tr("Max gap extensions"),
             BWAWorker::tr("Max gap extensions (-e)"));

         Descriptor indelOffset(INDEL_OFFSET,
             BWAWorker::tr("Indel offset"),
             BWAWorker::tr("Indel offset (-i)"));

         Descriptor maxLongDeletions(LONG_DELETIONS, 
             BWAWorker::tr("Max long deletions extensions"),
             BWAWorker::tr("Max long deletions extensions(-d)"));

         Descriptor maxQueue(MAX_QUEUE_ENTRIES,
             BWAWorker::tr("Max queue entries"),
             BWAWorker::tr("Max queue entries (-m)"));

         Descriptor barcodeLength(BARCODE_LENGTH,
             BWAWorker::tr("Barcode length"),
             BWAWorker::tr("Barcode length (-B)"));

         Descriptor numThreads(NUM_THREADS,
             BWAWorker::tr("Threads"),
             BWAWorker::tr("Threads (-t)"));

         Descriptor maxSeedDiff(MAX_SEED_DIFF,
             BWAWorker::tr("Max seed differencies"),
             BWAWorker::tr("Max seed differencies (-k)"));

         Descriptor mismatchPenalty(MISMATCH_PENALTY,
             BWAWorker::tr("Mismatch penalty"),
             BWAWorker::tr("Mismatch penalty (-M)"));

         Descriptor gapOpen(GAP_OPEN_PENALTY,
             BWAWorker::tr("Gap open penalty"),
             BWAWorker::tr("Gap open penalty (-O)."));

         Descriptor gapExt(GAP_EXTENSION_PENALTY,
             BWAWorker::tr("Gap extension penalty"),
             BWAWorker::tr("Gap extension penalty; a gap of size k cost (-E)."));

         Descriptor quolityThreshold(QUALITY_THRESHOLD,
             BWAWorker::tr("Quolity threshold"),
             BWAWorker::tr("Quolity threshold (-q)."));

        attrs << new Attribute(useMissProb, BaseTypes::BOOL_TYPE(), false, QVariant(true));

        Attribute* maxDiffAttr = new Attribute(maxDiff, BaseTypes::NUM_TYPE(), false, QVariant(0));
        maxDiffAttr->addRelation(new VisibilityRelation(USE_MISS_PROB, QVariant(false)));
        attrs << maxDiffAttr;

        Attribute* missProbAttr = new Attribute(missProb, BaseTypes::NUM_TYPE(), false, QVariant(0.04));
        missProbAttr->addRelation(new VisibilityRelation(USE_MISS_PROB, QVariant(true)));
        attrs << missProbAttr;

        attrs << new Attribute(seedLength, BaseTypes::NUM_TYPE(), false, QVariant(32));
        attrs << new Attribute(maxGap, BaseTypes::NUM_TYPE(), false, QVariant(1));
        attrs << new Attribute(indexAlg, BaseTypes::STRING_TYPE(), false, QVariant("is"));
        attrs << new Attribute(bestHits, BaseTypes::NUM_TYPE(), false, QVariant(30));
        attrs << new Attribute(longGapPenalty, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(nonIterative, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(enableLongGaps, BaseTypes::BOOL_TYPE(), false, QVariant(true));

        Attribute* maxGapExtensionsAttr = new Attribute(maxGapExtensions, BaseTypes::NUM_TYPE(), false, QVariant(0));
        maxGapExtensionsAttr->addRelation(new VisibilityRelation(ENABLE_LONG_GAPS, QVariant(true)));
        attrs << maxGapExtensionsAttr;

        attrs << new Attribute(indelOffset, BaseTypes::NUM_TYPE(), false, QVariant(5));
        attrs << new Attribute(maxLongDeletions, BaseTypes::NUM_TYPE(), false, QVariant(10));
        attrs << new Attribute(barcodeLength, BaseTypes::NUM_TYPE(), false, QVariant(0));
        attrs << new Attribute(maxQueue, BaseTypes::NUM_TYPE(), false, QVariant(2000000));
        attrs << new Attribute(numThreads, BaseTypes::NUM_TYPE(), false, QVariant(4));
        attrs << new Attribute(maxSeedDiff, BaseTypes::NUM_TYPE(), false, QVariant(2));
        attrs << new Attribute(mismatchPenalty, BaseTypes::NUM_TYPE(), false, QVariant(3));
        attrs << new Attribute(gapOpen, BaseTypes::NUM_TYPE(), false, QVariant(11));
        attrs << new Attribute(gapExt, BaseTypes::NUM_TYPE(), false, QVariant(4));
        attrs << new Attribute(quolityThreshold, BaseTypes::NUM_TYPE(), false, QVariant(0));
     }

     {
         QVariantMap spinMap; spinMap["minimum"] = QVariant(0); spinMap["maximum"] = QVariant(INT_MAX);
         delegates[MISSING_PROB]  = new DoubleSpinBoxDelegate(spinMap);
         delegates[MAX_DIFF]  = new SpinBoxDelegate(spinMap);
         delegates[SEED_LENGTH] = new SpinBoxDelegate(spinMap);
         delegates[MAX_GAP] = new SpinBoxDelegate(spinMap);
         delegates[BEST_HITS] = new SpinBoxDelegate(spinMap);
         delegates[GAP_EXTENSIONS] = new DoubleSpinBoxDelegate(spinMap);
         delegates[INDEL_OFFSET] = new SpinBoxDelegate(spinMap);
         delegates[LONG_DELETIONS] = new SpinBoxDelegate(spinMap);
         delegates[MAX_QUEUE_ENTRIES] = new SpinBoxDelegate(spinMap);
         delegates[BARCODE_LENGTH] = new DoubleSpinBoxDelegate(spinMap);
         delegates[NUM_THREADS] = new SpinBoxDelegate(spinMap);
         delegates[MAX_SEED_DIFF] = new SpinBoxDelegate(spinMap);
         delegates[MISMATCH_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[GAP_OPEN_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[GAP_EXTENSION_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[QUALITY_THRESHOLD] = new SpinBoxDelegate(spinMap);

         QVariantMap vm;
         vm["bwtsw"] = "bwtsw";
         vm["div"] = "div";
         vm["is"] = "is";
         delegates[INDEX_ALG] = new ComboBoxDelegate(vm);
    }

    Descriptor protoDesc(BWAWorkerFactory::ACTOR_ID,
        BWAWorker::tr("Align Reads with BWA"),
        BWAWorker::tr("Performs alignment of short reads with BWA."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, getPortDescriptors(), attrs);
    proto->setPrompter(new ShortReadsAlignerPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPortValidator(IN_PORT_DESCR, new ShortReadsAlignerSlotsValidator());
    proto->addExternalTool(ET_BWA);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_NGS_ALIGN_SHORT_READS(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new BWAWorkerFactory());
}

Worker *BWAWorkerFactory::createWorker(Actor *a) {
    return new BWAWorker(a);
}

} // LocalWorkflow
} // U2

