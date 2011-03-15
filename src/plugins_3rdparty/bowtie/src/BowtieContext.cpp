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

#include "BowtieTask.h"
#include "BowtieContext.h"
#include "BowtieConstants.h"

#include <U2Core/Log.h>

#include "ebwt.h"

BowtieContext* BowtieContext::getContext()
{
	return static_cast<BowtieContext*>(U2::TLSUtils::current(BOWTIE_CONTEXT_ID));
}

BowtieContext::Search* BowtieContext::getSearchContext()
{
	return &((static_cast<BowtieContext*>(U2::TLSUtils::current(BOWTIE_CONTEXT_ID)))->search);
}

BowtieContext::Build* BowtieContext::getBuildContext()
{
	return &((static_cast<BowtieContext*>(U2::TLSUtils::current(BOWTIE_CONTEXT_ID)))->build);
}

bool BowtieContext::isCanceled()
{
	return ((static_cast<BowtieContext*>(U2::TLSUtils::current(BOWTIE_CONTEXT_ID)))->ti.cancelFlag);
}

void BowtieContext::runWorker( int* id, void *(*start_routine) (void *), void *arg )
{
	*id = jobList.size();
	BowtieWorkerArg bowtieArg = {*id, start_routine, arg};
	jobList.append(bowtieArg);
	workerList[*id]->start.release();
}

void BowtieContext::joinWorker( int id )
{
	workerList[id]->stop.acquire();
}

BowtieContext::BowtieContext( U2::TaskStateInfo& stateInfo, int nThreads )
: U2::TLSContext(BOWTIE_CONTEXT_ID), ti(stateInfo), hasError(false)
{
	search.nthreads = nThreads;
	for(int i=1;i<nThreads;i++) {
		workerList.append(new Worker());
	}
}

BowtieContext::~BowtieContext()
{
	for(int i=1;i<search.nthreads;i++) {
		delete workerList[i];
	}
}

void BowtieContext::verbose( const std::string& s ) {
    U2::algoLog.trace(s.c_str());
}

void BowtieContext::Search::resetOptions() {
	mates1.clear();
	mates2.clear();
	mates12.clear();
	adjustedEbwtFileBase	= "";
	verbose					= 0;
	startVerbose			= 0;
	quiet					= false;
	sanityCheck				= 0;  // enable expensive sanity checks
	format					= FASTQ; // default read format is FASTQ
	origString				= ""; // reference text, or filename(s)
	seed					= 0; // srandom() seed
	timing					= 0; // whether to report basic timing data
	allHits					= false; // for multihits, report just one
	rangeMode				= false; // report BWT ranges instead of ref locs
	showVersion				= 0; // just print version and quit?
	ipause					= 0; // pause before matching?
	qUpto					= 0xffffffff; // max # of queries to read
	trim5					= 0; // amount to trim from 5' end
	trim3					= 0; // amount to trim from 3' end
	reportOpps				= 0; // whether to report # of other mappings
	offRate					= -1; // keep default offRate
	isaRate					= -1; // keep default isaRate
	mismatches				= 0; // allow 0 mismatches by default
	patDumpfile				= NULL; // filename to dump patterns to
	solexaQuals				= false; // quality strings are solexa quals, not phred, and subtract 64 (not 33)
	phred64Quals			= false; // quality chars are phred, but must subtract 64 (not 33)
	integerQuals			= false; // quality strings are space-separated strings of integers, not ASCII
	maqLike					= 1;   // do maq-like searching
	seedLen					= 28;  // seed length (changed in Maq 0.6.4 from 24)
	seedMms					= 2;   // # mismatches allowed in seed (maq's -n)
	qualThresh				= 70;  // max qual-weighted hamming dist (maq's -e)
	maxBtsBetter			= 125; // max # backtracks allowed in half-and-half mode
	maxBts					= 800; // max # backtracks allowed in half-and-half mode
	//nthreads				= 1;     // number of pthreads operating concurrently
	outType					= OUTPUT_FULL;  // style of output
	randReadsNoSync			= false; // true -> generate reads from per-thread random source
	numRandomReads			= 50000000; // # random reads (see Random*PatternSource in pat.h)
	lenRandomReads			= 35;    // len of random reads (see Random*PatternSource in pat.h)
	noRefNames				= false; // true -> print reference indexes; not names
	dumpAlBase				= "";    // basename of same-format files to dump aligned reads to
	dumpUnalBase			= "";    // basename of same-format files to dump unaligned reads to
	dumpMaxBase				= "";    // basename of same-format files to dump reads with more than -m valid alignments to
	khits					= 1;     // number of hits per read; >1 is much slower
	mhits					= 0xffffffff; // don't report any hits if there are > mhits
	better					= false; // true -> guarantee alignments from best possible stratum
	strata					= false; // true -> don't stop at stratum boundaries
	refOut					= false; // if true, alignments go to per-ref files
	partitionSz				= 0;     // output a partitioning key in first field
	noMaqRound				= false; // true -> don't round quals to nearest 10 like maq
	useSpinlock				= true;  // false -> don't use of spinlocks even if they're #defines
	fileParallel			= false; // separate threads read separate input files in parallel
	useShmem				= false; // use shared memory to hold the index
	useMm					= false; // use memory-mapped files to hold the index
	mmSweep					= false; // sweep through memory-mapped files immediately after mapping
	stateful				= false; // use stateful aligners
	prefetchWidth			= 1;     // number of reads to process in parallel w/ --stateful
	minInsert				= 0;     // minimum insert size (Maq = 0, SOAP = 400)
	maxInsert				= 250;   // maximum insert size (Maq = 250, SOAP = 600)
	mate1fw					= true;  // -1 mate aligns in fw orientation on fw strand
	mate2fw					= false; // -2 mate aligns in rc orientation on fw strand
	mateFwSet				= false; // true -> user set mate1fw/mate2fw with --ff/--fr/--rf
	mixedThresh				= 4;     // threshold for when to switch to paired-end mixed mode (see aligner.h)
	mixedAttemptLim			= 100;   // number of attempts to make in "mixed mode" before giving up on orientation
	dontReconcileMates		= true;  // suppress pairwise all-versus-all way of resolving mates
	cacheLimit				= 5;     // ranges w/ size > limit will be cached
	cacheSize				= 0;     // # words per range cache
	offBase					= 0;     // offsets are 0-based by default, but configurable
	tryHard					= false; // set very high maxBts, mixedAttemptLim
	skipReads				= 0;     // # reads/read pairs to skip
	nofw					= false; // don't align fw orientation of read
	norc					= false; // don't align rc orientation of read
	strandFix				= true;  // attempt to fix strand bias
	randomizeQuals			= false; // randomize quality values
	stats					= false; // print performance stats
	chunkPoolMegabytes		= 64;    // max MB to dedicate to best-first search frames per thread
	chunkSz					= 256;   // size of single chunk disbursed by ChunkPool (in KB)
	chunkVerbose			= false; // have chunk allocator output status messages?
	recal					= false;
	recalMaxCycle			= 64;
	recalMaxQual			= 40;
	recalQualShift			= 2;
	useV1					= true;
	reportSe				= false;
	refMapFile				= NULL;  // file containing a map from index coordinates to another coordinate system
	annotMapFile			= NULL;  // file containing a map from reference coordinates to annotations
	fastaContLen			= 0;
	fastaContFreq			= 0;
	hadoopOut				= false; // print Hadoop status and summary messages
	fuzzy					= false; // reads will have alternate basecalls w/ qualities
	fullRef					= false; // print entire reference name instead of just up to 1st space
	samNoHead				= false; // don't print any header lines in SAM output
	samNoSQ					= false; // don't print @SQ header lines
	color					= false; // don't align in colorspace by default
	colorExEnds				= true;  // true -> nucleotides on either end of decoded cspace alignment should be excluded
	rgs						= "";    // SAM outputs for @RG header line
	snpPhred				= 30;    // probability of SNP, for scoring colorspace alignments
	suppressOuts.clear();            // output fields to suppress
	sampleMax				= false;
	defaultMapq				= 255;
	colorSeq				= false; // true -> show colorspace alignments as colors, not decoded bases
	colorQual				= false; // true -> show colorspace qualities as original quals, not decoded quals
	printCost				= false; // true -> print cost and stratum
	showSeed				= false; // true -> print per-read pseudo-random seed
	qualities.clear();
	qualities1.clear();
	qualities2.clear();
	sortAlignment = false;
	MUTEX_INIT(gLock);
}

void BowtieContext::Build::resetOptions() {
	clDCs_calced = false;
	verbose      = true;  // be talkative (default)
	sanityCheck  = 0;     // do slow sanity checks
	format       = FASTA; // input sequence format
	bmax         = 0xffffffff; // max blockwise SA bucket size
	bmaxMultSqrt = 0xffffffff; // same, as multplier of sqrt(n)
	bmaxDivN     = 4;          // same, as divisor of n
	dcv          = 1024;  // bwise SA difference-cover sample sz
	noDc         = 0;     // disable difference-cover sample
	entireSA     = 0;     // 1 = disable blockwise SA
	seed         = 0;     // srandom seed
	showVersion  = 0;     // just print version and quit?
	doubleEbwt   = true;  // build forward and reverse Ebwts
	//   Ebwt parameters
	lineRate     = 6;  // a "line" is 64 bytes
	linesPerSide = 1;  // 1 64-byte line on a side
	offRate      = 5;  // sample 1 out of 32 SA elts
	ftabChars    = 10; // 10 chars in initial lookup table
	bigEndian    = 0;  // little endian
	nsToAs       = false; // convert reference Ns to As prior to indexing
	autoMem      = true;  // automatically adjust memory usage parameters
	packed       = false; //
	writeRef     = true;  // write compact reference to .3.ebwt/.4.ebwt
	justRef      = false; // *just* write compact reference, don't index
	reverseType  = REF_READ_REVERSE_EACH;
	color        = false;
	secondEbwtRound = false;
}

extern void joinThread(pthread_t th) {
	BowtieContext* ctx = BowtieContext::getContext();
	ctx->joinWorker(th);
}

extern void createThread(pthread_t* th,
						 void *(*start_routine) (void *),
						 void *arg)
{
	BowtieContext* ctx = BowtieContext::getContext();
	ctx->runWorker(th, start_routine, arg);
}
