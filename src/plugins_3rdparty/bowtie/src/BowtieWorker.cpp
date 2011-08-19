/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "BowtieWorker.h"
#include "BowtiePlugin.h"

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Core/Log.h>
#include <U2Gui/DialogUtils.h>
#include <U2Core/DNAAlphabet.h>

/* TRANSLATOR U2::LocalWorkflow::BowtieWorker */

namespace U2 {
namespace LocalWorkflow {

static const QString EBWT_PORT("in-ebwt");
static const QString EBWT_OUT_PORT_ID("out-ebwt");

const QString BowtieWorkerFactory::ACTOR_ID("bowtie");
const QString BowtieBuildWorkerFactory::ACTOR_ID("bowtie-build-index");
const QString BowtieIndexReaderWorkerFactory::ACTOR_ID("bowtie-read-index");

static const QString REFSEQ_URL_ATTR("url-reference");
static const QString EBWT_URL_ATTR("url-ebwt");
static const QString N_MODE_MISMATCHES_ATTR("mismatches-num");
static const QString V_MODE_MISMATCHES_ATTR("report-with-mismatches");
static const QString MAQERR_ATTR("maq-err");
static const QString SEEDLEN_ATTR("seed-length");
static const QString NOMAQROUND_ATTR("no-maq-rounding");
static const QString NOFW_ATTR("no-forward");
static const QString NORC_ATTR("no-reverse-complemented");
static const QString MAXBTS_ATTR("max-backtracks");
static const QString TRYHARD_ATTR("try-hard");
static const QString CHUNKMBS_ATTR("chunk-mbs");
static const QString SEED_ATTR("seed");
static const QString BEST_ATTR("best");
static const QString ALL_ATTR("all");

/************************************************************************/
/* BowtieCommunicationChanelReader                                      */
/************************************************************************/

BowtieCommunicationChanelReader::BowtieCommunicationChanelReader(CommunicationChannel* reads) {
	assert(reads != NULL);
	this->reads = reads;
}

bool BowtieCommunicationChanelReader::isEnd() {
	return !reads->hasMessage() || reads->isEnded();
}

DNASequence *BowtieCommunicationChanelReader::read() {
    return new DNASequence(reads->get().getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<DNASequence>());
}

/************************************************************************/
/* BowtieMAlignmentWriter                                               */
/************************************************************************/
BowtieMAlignmentWriter::BowtieMAlignmentWriter() {
	//do nothing
}

void BowtieMAlignmentWriter::close() { 
	//TODO: add some heuristic alphabet selection. 
	result.setAlphabet(AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT()));
}

MAlignment& BowtieMAlignmentWriter::getResult() {
	return result;
}

void BowtieMAlignmentWriter::write(const DNASequence& seq, int offset) {
	MAlignmentRow row;
	row.setName(seq.getName());
	row.setSequence(seq.seq, offset);
	row.setQuality(seq.quality);
	result.addRow(row);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

static const Descriptor EBWT_SLOT("ebwt-index", BowtieWorker::tr("EBWT index"), BowtieWorker::tr("EBWT Index for bowtie"));

void BowtieWorkerFactory::init() {
	QList<PortDescriptor*> p; QList<Attribute*> a;
    Descriptor readsd(BasePorts::IN_SEQ_PORT_ID(), BowtieWorker::tr("Short read sequences"), 
        BowtieWorker::tr("Short reads to be aligned."));
	Descriptor ebwtd(EBWT_PORT, BowtieWorker::tr("EBWT index"), BowtieWorker::tr("EBWT index of reference sequence."));
	Descriptor oud(BasePorts::OUT_MSA_PORT_ID(), BowtieWorker::tr("Short reads alignment"), BowtieWorker::tr("Result of alignment."));
	
	QMap<Descriptor, DataTypePtr> inSeqM;
    inSeqM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
    p << new PortDescriptor(readsd, DataTypePtr(new MapDataType("bowtie.in.sequence", inSeqM)), true /*input*/);
	QMap<Descriptor, DataTypePtr> inEbwtM;
    inEbwtM[EBWT_SLOT] = BowtiePlugin::EBWT_INDEX_TYPE();
    p << new PortDescriptor(ebwtd, DataTypePtr(new MapDataType("bowtie.in.ebwt", inEbwtM)), true /*input*/, false /*multi*/, IntegralBusPort::BLIND_INPUT);
	QMap<Descriptor, DataTypePtr> outM;
    outM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("bowtie.out.out.ma", outM)), false /*input*/, false /*multi*/);
	
	Descriptor desc(ACTOR_ID, BowtieWorker::tr("Bowtie aligner"), 
		BowtieWorker::tr("An ultrafast memory-efficient short read aligner, http://bowtie-bio.sourceforge.net"));
	Descriptor n_mismatches(N_MODE_MISMATCHES_ATTR, BowtieWorker::tr("-n alignment mode"), 
		BowtieWorker::tr("<html><body><p><b>-n</b></p>Alignments may have no more than N mismatches (where N is a number 0-3, set with -n) in the first L bases  \
                         (where L is a number 5 or greater, set with -l) on the high-quality (left) end of the read. The first L bases are called \
                         the \"seed\".</p></body></html>"));
	Descriptor v_mismatches(V_MODE_MISMATCHES_ATTR, BowtieWorker::tr("-v alignment mode"), 
		BowtieWorker::tr("<html><body><p><b>-v</b></p>\"-1\" - use default value. Report alignments with at most <int> mismatches. -e and -l options are ignored \
                         and quality values have no effect on what alignments are valid. -v is mutually exclusive with -n.</p></body></html>"));
	Descriptor maqerr(MAQERR_ATTR, BowtieWorker::tr("Maximum permitted total of quality values"), 
		BowtieWorker::tr("<html><body><p><b>-e/--maqerr</b></p>Maximum permitted total of quality values at all mismatched read positions \
                         throughout the entire alignment, not just in the \"seed\". The default is 70.</p></body></html>"));
	Descriptor seedlen(SEEDLEN_ATTR, BowtieWorker::tr("Seed length"), 
		BowtieWorker::tr("<html><body><p><b>-l/--seedlen</b></p>The \"seed length\"; i.e., the number of bases on the high-quality end of \
                         the read to which the -n ceiling applies. The lowest permitted setting is 5 and the default is 28. bowtie is faster for \
                         larger values of -l.</p></body></html>"));
	Descriptor nomaqround(NOMAQROUND_ATTR, BowtieWorker::tr("Skip Maq quality rounding"), 
		BowtieWorker::tr("<html><body><p><b>--nomaqround</b></p>Maq accepts quality values in the Phred quality scale, but internally rounds \
                         values to the nearest 10, with a maximum of 30. By default, bowtie also rounds this way. --nomaqround prevents this rounding \
                         in bowtie.</p></body></html>"));
	Descriptor nofw(NOFW_ATTR, BowtieWorker::tr("Do not align against the forward reference strand"), 
		BowtieWorker::tr("<html><body><p><b>--nofw</b></p>If --nofw is specified, bowtie will not attempt to align against the forward reference \
                         strand.</p></body></html>"));
	Descriptor norc(NORC_ATTR, BowtieWorker::tr("Do not align against the reverse-complement reference strand"), 
		BowtieWorker::tr("<html><body><p><b>--norc</b></p>If --norc is specified, bowtie will not attempt to align against the reverse-complement \
                         reference strand.</p></body></html>"));
	Descriptor maxbts(MAXBTS_ATTR, BowtieWorker::tr("Maximum number of backtracks permitted"), 
		BowtieWorker::tr("<html><body><p><b>--maxbts</b></p>The maximum number of backtracks permitted when aligning a read in -n 2 or -n 3 mode \
                         (default: 125 without --best, 800 with --best). A \"backtrack\" is the introduction of a speculative substitution into the alignment. \
                         Without this limit, the default parameters will sometimes require that bowtie try 100s or 1,000s of backtracks to align a read, \
                         especially if the read has many low-quality bases and/or has no valid alignments, slowing bowtie down significantly. However, this \
                         limit may cause some valid alignments to be missed. Higher limits yield greater sensitivity at the expensive of longer running \
                         times.</p></body></html>"));
	Descriptor tryhard(TRYHARD_ATTR, BowtieWorker::tr("Tryhard"), 
		BowtieWorker::tr("<html><body><p><b>-y/--tryhard</b></p>Try as hard as possible to find valid alignments when they exist, including paired-end \
                         alignments.</p></body></html>"));
	Descriptor chunkmbs(CHUNKMBS_ATTR, BowtieWorker::tr("Chunk Mbs"), 
		BowtieWorker::tr("<html><body><p><b>--chunkmbs</b></p>The number of megabytes of memory a given thread is given to store path descriptors \
                         in --best mode.</p></body></html>"));
	Descriptor seed(SEED_ATTR, BowtieWorker::tr("Pseudo random seed number"), 
		BowtieWorker::tr("<html><body><p><b>--seed</b></p>Use <int> as the seed for pseudo-random number generator. \"-1\" - use pseudo random</p></body></html>"));
    Descriptor best(BEST_ATTR, BowtieWorker::tr("Align reads with \"best\" option"), 
        BowtieWorker::tr("<html><body><p><b>--best</b></p>Make Bowtie guarantee that reported singleton alignments are \"best\" in terms of stratum (i.e. number of mismatches, or \
                         mismatches in the seed in the case of -n mode) and in terms of the quality values at the mismatched position(s). bowtie is somewhat \
                         slower when --best is specified.</p></body></html>"));
    Descriptor all(ALL_ATTR, BowtieWorker::tr("Report about all valid alignments"), 
        BowtieWorker::tr("<html><body><p><b>-a/--all</b></p>Report all valid alignments per read or pair.</p></body></html>"));

	a << new Attribute(n_mismatches, BaseTypes::NUM_TYPE(), false /*not required*/, 2);
	a << new Attribute(v_mismatches, BaseTypes::NUM_TYPE(), false /*not required*/, -1);
	a << new Attribute(maqerr, BaseTypes::NUM_TYPE(), false /*not required*/, 70);
	a << new Attribute(seedlen, BaseTypes::NUM_TYPE(), false /*not required*/, 28);
	a << new Attribute(nomaqround, BaseTypes::BOOL_TYPE(), false /*not required*/, false);
	a << new Attribute(nofw, BaseTypes::BOOL_TYPE(), false /*not required*/, false);
	a << new Attribute(norc, BaseTypes::BOOL_TYPE(), false /*not required*/, false);
	a << new Attribute(maxbts, BaseTypes::NUM_TYPE(), false /*not required*/, -1);
	a << new Attribute(tryhard, BaseTypes::BOOL_TYPE(), false /*not required*/, false);
	a << new Attribute(chunkmbs, BaseTypes::NUM_TYPE(), false /*not required*/, 64);
	a << new Attribute(seed, BaseTypes::NUM_TYPE(), false /*not required*/, -1);
    a << new Attribute(best, BaseTypes::BOOL_TYPE(), false /*not required*/, false);
    a << new Attribute(all, BaseTypes::BOOL_TYPE(), false /*not required*/, false);

	ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

	QMap<QString, PropertyDelegate*> delegates;    

	//delegates[REFSEQ_URL_ATTR] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), true);
	{
		QVariantMap _n; _n["minimum"] = 0; _n["maximum"] = 3;
		delegates[N_MODE_MISMATCHES_ATTR] = new SpinBoxDelegate(_n);
		QVariantMap _v; _v["minimum"] = -1; _v["maximum"] = 3;
		delegates[V_MODE_MISMATCHES_ATTR] = new SpinBoxDelegate(_v);
		QVariantMap _l; _l["minimum"] = 5;
		delegates[SEEDLEN_ATTR] = new SpinBoxDelegate(_l);
		QVariantMap _e; _e["minimum"] = 1;
		delegates[MAQERR_ATTR] = new SpinBoxDelegate(_e);
		QVariantMap _maxbts; _maxbts["minimum"] = -1;
		delegates[MAXBTS_ATTR] = new SpinBoxDelegate(_maxbts);
		QVariantMap _chunkmbs; _chunkmbs["minimum"] = 1;
		delegates[CHUNKMBS_ATTR] = new SpinBoxDelegate(_chunkmbs);
		QVariantMap _seed; _seed["minimum"] = -1;
		delegates[SEED_ATTR] = new SpinBoxDelegate(_seed);
	}
	proto->setEditor(new DelegateEditor(delegates));
	proto->setPrompter(new BowtiePrompter());
	proto->setIconPath(":core/images/align.png");
	WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ASSEMBLY(), proto);

	DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
	localDomain->registerEntry(new BowtieWorkerFactory());
}

QString BowtiePrompter::composeRichDoc() {
    Actor* readsProducer = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()))->getProducer(BasePorts::IN_SEQ_PORT_ID());
	Actor* ebwtProducer = qobject_cast<IntegralBusPort*>(target->getPort(EBWT_PORT))->getProducer(EBWT_PORT);
	
	QString readsName = readsProducer ? tr(" from <u>%1</u>").arg(readsProducer->getLabel()) : "";
	QString ebwtName = ebwtProducer ? tr(" from <u>%1</u>").arg(ebwtProducer->getLabel()) : "";

	QString doc = tr("Align short reads %1 to the reference genome %2 and send it to output.")
		.arg(readsName).arg(ebwtName);

	return doc;
}

void BowtieWorker::init() {
	reader = NULL;
	writer = NULL;
    reads = ports.value(BasePorts::IN_SEQ_PORT_ID());
	ebwt = ports.value(EBWT_PORT);
	output = ports.value(BasePorts::OUT_MSA_PORT_ID());
	settings.prebuiltIndex = true;
	settings.setCustomValue(BowtieTask::OPTION_N_MISMATCHES, actor->getParameter(N_MODE_MISMATCHES_ATTR)->getAttributeValue<int>());
	settings.setCustomValue(BowtieTask::OPTION_V_MISMATCHES, actor->getParameter(V_MODE_MISMATCHES_ATTR)->getAttributeValue<int>());
	settings.setCustomValue(BowtieTask::OPTION_MAQERR, actor->getParameter(MAQERR_ATTR)->getAttributeValue<int>());
	settings.setCustomValue(BowtieTask::OPTION_SEED_LEN, actor->getParameter(SEEDLEN_ATTR)->getAttributeValue<int>());
	settings.setCustomValue(BowtieTask::OPTION_NOMAQROUND, actor->getParameter(NOMAQROUND_ATTR)->getAttributeValue<bool>());
	settings.setCustomValue(BowtieTask::OPTION_NOFW, actor->getParameter(NOFW_ATTR)->getAttributeValue<bool>());
	settings.setCustomValue(BowtieTask::OPTION_NORC, actor->getParameter(NORC_ATTR)->getAttributeValue<bool>());
	settings.setCustomValue(BowtieTask::OPTION_MAXBTS, actor->getParameter(MAXBTS_ATTR)->getAttributeValue<int>());
	settings.setCustomValue(BowtieTask::OPTION_TRYHARD, actor->getParameter(TRYHARD_ATTR)->getAttributeValue<bool>());
	settings.setCustomValue(BowtieTask::OPTION_CHUNKMBS, actor->getParameter(CHUNKMBS_ATTR)->getAttributeValue<int>());
	settings.setCustomValue(BowtieTask::OPTION_SEED, actor->getParameter(SEED_ATTR)->getAttributeValue<int>());
    settings.setCustomValue(BowtieTask::OPTION_BEST, actor->getParameter(BEST_ATTR)->getAttributeValue<bool>());
    settings.setCustomValue(BowtieTask::OPTION_ALL, actor->getParameter(ALL_ATTR)->getAttributeValue<bool>());
}

bool BowtieWorker::isReady() {
	return (reads && reads->hasMessage() && ebwt && ebwt->hasMessage());
}

Task* BowtieWorker::tick() {
	if (reads->hasMessage())  {  
		if( reads->isEnded()) {
			algoLog.error(BowtieWorker::tr("Short reads list is empty."));
			return NULL;
		}

		reader = new BowtieCommunicationChanelReader(reads);
		writer = new BowtieMAlignmentWriter();
		
        settings.refSeqUrl = GUrl(ebwt->get().getData().toMap().value(EBWT_SLOT.getId()).value<QString>());

		settings.setCustomValue(BowtieTask::OPTION_READS_READER, QVariant::fromValue(BowtieReadsReaderContainer(reader)));
		settings.setCustomValue(BowtieTask::OPTION_READS_WRITER, QVariant::fromValue(BowtieReadsWriterContainer(writer)));

		if( settings.refSeqUrl.isEmpty()) {
			algoLog.trace(BowtieWorker::tr("Reference sequence URL is empty")); 
			return NULL;
		}

		Task* t = new BowtieTask(settings);
		connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
		return t;
	}
	return NULL;
}

void BowtieWorker::cleanup() {
	delete reader;
	delete writer;
	writer = NULL;
	reader = NULL;
}

void BowtieWorker::sl_taskFinished() {
	BowtieTask* t = qobject_cast<BowtieTask*>(sender());
	if (t->getState() != Task::State_Finished) {
		return;
	}

	QVariant v = qVariantFromValue<MAlignment>((dynamic_cast<BowtieMAlignmentWriter*>(writer))->getResult());
	output->put(Message(BaseTypes::MULTIPLE_ALIGNMENT_TYPE(), v));
	if (reads->isEnded()) {
		output->setEnded();
	}
		
	algoLog.trace(tr("Bowtie alignment finished. Result name is %1").arg(settings.resultFileName.baseFileName()));
}

bool BowtieWorker::isDone() {
	return !reads || reads->isEnded();
}

/************************************************************************/
/* Bowtie build worker                                                  */
/************************************************************************/

void BowtieBuildWorkerFactory::init() {
	QList<PortDescriptor*> p; QList<Attribute*> a;
	Descriptor oud(EBWT_OUT_PORT_ID, BowtieBuildWorker::tr("EBWT index"), BowtieBuildWorker::tr("Result ebwt index."));
	
    QMap<Descriptor, DataTypePtr> outM;
    outM[EBWT_SLOT] = BowtiePlugin::EBWT_INDEX_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("bowtie.build.index.out", outM)), false /*input*/, true /*multi*/);
	
    Descriptor refseq(REFSEQ_URL_ATTR, BowtieBuildWorker::tr("Reference"), 
		BowtieBuildWorker::tr("Reference sequence url. The short reads will be aligned to this reference genome."));
    Descriptor desc(ACTOR_ID, BowtieBuildWorker::tr("Bowtie index builder"),
		BowtieWorker::tr("Bowtie-build builds a Bowtie index from a set of DNA sequences. bowtie-build outputs a set of 6 files with suffixes .1.ebwt, .2.ebwt, .3.ebwt, .4.ebwt, .rev.1.ebwt, and .rev.2.ebwt. These files together constitute the index: they are all that is needed to align reads to that reference. The original sequence files are no longer used by Bowtie once the index is built."));
	Descriptor ebwt(EBWT_URL_ATTR, BowtieBuildWorker::tr("EBWT"), 
		BowtieBuildWorker::tr("Output index url."));

	a << new Attribute(refseq, BaseTypes::STRING_TYPE(), true /*required*/, QString());
	a << new Attribute(ebwt, BaseTypes::STRING_TYPE(), true /*required*/, QString());

	ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

	QMap<QString, PropertyDelegate*> delegates;    

	delegates[REFSEQ_URL_ATTR] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), true);
	delegates[EBWT_URL_ATTR] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), false);

	proto->setEditor(new DelegateEditor(delegates));
	proto->setPrompter(new BowtieBuildPrompter());
	proto->setIconPath(":core/images/align.png");
	WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ASSEMBLY(), proto);

	DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
	localDomain->registerEntry(new BowtieBuildWorkerFactory());
}

QString BowtieBuildPrompter::composeRichDoc() {
	QString refSeqUrl = getParameter(REFSEQ_URL_ATTR).toString();
	QString refSeq = (refSeqUrl.isEmpty() ? "" : QString("<u>%1</u>").arg(GUrl(refSeqUrl).fileName()) );

	QString doc = tr("Build ebwt index from %1 and send it url to output.").arg(refSeq);

	return doc;
}

void BowtieBuildWorker::init() {
	output = ports.value(EBWT_OUT_PORT_ID);
	refSeqUrl = actor->getParameter(REFSEQ_URL_ATTR)->getAttributeValue<QString>();
	ebwtUrl = actor->getParameter(EBWT_URL_ATTR)->getAttributeValue<QString>();
}

bool BowtieBuildWorker::isReady() {
	return !isDone();
}

Task* BowtieBuildWorker::tick() {

	if( refSeqUrl.isEmpty()) {
		algoLog.trace(BowtieBuildWorker::tr("Reference sequence URL is empty")); 
		return NULL;
	}
	if( ebwtUrl.isEmpty()) {
		algoLog.trace(BowtieBuildWorker::tr("Reference sequence URL is empty")); 
		return NULL;
	}

	Task* t = new BowtieBuildTask(refSeqUrl.getURLString(), ebwtUrl.getURLString());
	connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
	return t;
}

void BowtieBuildWorker::sl_taskFinished() {
	BowtieBuildTask* t = qobject_cast<BowtieBuildTask*>(sender());
	if (t->getState() != Task::State_Finished) {
		return;
	}

	done = true;

	QVariant v = qVariantFromValue<QString>(t->getEbwtPath());
	output->put(Message(BowtiePlugin::EBWT_INDEX_TYPE(), v));
	output->setEnded();
	algoLog.trace(tr("Bowtie index building finished. Result name is %1").arg(t->getEbwtPath()));
}

bool BowtieBuildWorker::isDone() {
	return done;
}

/************************************************************************/
/* Bowtie index reader                                                  */
/************************************************************************/

void BowtieIndexReaderWorkerFactory::init() {
	QList<PortDescriptor*> p; QList<Attribute*> a;
	Descriptor oud(EBWT_OUT_PORT_ID, BowtieIndexReaderWorker::tr("EBWT index"), BowtieIndexReaderWorker::tr("Result of alignment."));
	
    QMap<Descriptor, DataTypePtr> outM;
    outM[EBWT_SLOT] = BowtiePlugin::EBWT_INDEX_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("bowtie.index.reader.out", outM)), false /*input*/, true /*multi*/);
	
    Descriptor desc(ACTOR_ID, BowtieIndexReaderWorker::tr("Bowtie index reader"), 
		BowtieWorker::tr("Read a set of 6 files with suffixes .1.ebwt, .2.ebwt, .3.ebwt, .4.ebwt, .rev.1.ebwt, and .rev.2.ebwt. These files together constitute the index: they are all that is needed to align reads to that reference."));
	Descriptor ebwt(EBWT_URL_ATTR, BowtieIndexReaderWorker::tr("EBWT"), 
		BowtieIndexReaderWorker::tr("Select one of index files"));

	a << new Attribute(ebwt, BaseTypes::STRING_TYPE(), true /*required*/, QString());

	ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

	QMap<QString, PropertyDelegate*> delegates;    

	delegates[EBWT_URL_ATTR] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), false, false, false);

	proto->setEditor(new DelegateEditor(delegates));
	proto->setPrompter(new BowtieIndexReaderPrompter());
	proto->setIconPath(":core/images/align.png");
	WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ASSEMBLY(), proto);

	DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
	localDomain->registerEntry(new BowtieIndexReaderWorkerFactory());
}

QString BowtieIndexReaderPrompter::composeRichDoc() {
	QString ebwtUrl = getParameter(EBWT_URL_ATTR).toString();
	QString ebwt = (ebwtUrl.isEmpty() ? "" : QString("<u>%1</u>").arg(GUrl(ebwtUrl).fileName()) );

	QString doc = tr("Read ebwt index from %1 and send it url to output.").arg(ebwt);

	return doc;
}

void BowtieIndexReaderWorker::init() {
	output = ports.value(EBWT_OUT_PORT_ID);
	ebwtUrl = actor->getParameter(EBWT_URL_ATTR)->getAttributeValue<QString>();
}

bool BowtieIndexReaderWorker::isReady() {
	return !isDone();
}

Task* BowtieIndexReaderWorker::tick() {

	if( ebwtUrl.isEmpty()) {
		algoLog.trace(BowtieIndexReaderWorker::tr("Reference sequence URL is empty")); 
		return NULL;
	}
	Task* t = new Task("Bowtie index reader", TaskFlags_NR_FOSCOE);
	connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
	return t;
}

void BowtieIndexReaderWorker::sl_taskFinished() {
	QVariant v = qVariantFromValue<QString>(ebwtUrl.getURLString());
	output->put(Message(BowtiePlugin::EBWT_INDEX_TYPE(), v));
	output->setEnded();
	done = true;
	algoLog.trace(tr("Reading bowtie index finished. Result name is %1").arg(ebwtUrl.getURLString()));
}

bool BowtieIndexReaderWorker::isDone() {
	return done;
}

} //namespace LocalWorkflow
} //namespace U2
