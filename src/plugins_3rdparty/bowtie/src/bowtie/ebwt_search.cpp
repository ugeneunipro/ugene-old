#include "BowtieConstants.h"
#include "BowtieAdapter.h"
#include "BowtieIOAdapter.h"
#include "BowtieContext.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/Task.h>
#include <U2Core/Log.h>
#include <U2Core/GUrl.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Core/MAlignment.h>

#include <algorithm>
#include <QtCore/QVector>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <seqan/find.h>
#include <vector>
#include "alphabet.h"
#include "assert_helpers.h"
#include "endian_swap.h"
#include "ebwt.h"
#include "formats.h"
#include "sequence_io.h"
#include "tokenize.h"
#include "hit.h"
#include "pat.h"
#include "bitset.h"
#include "threading.h"
#include "range_cache.h"
#include "refmap.h"
#include "annot.h"
#include "aligner.h"
#include "aligner_0mm.h"
#include "aligner_1mm.h"
#include "aligner_23mm.h"
#include "aligner_seed_mm.h"
#include "aligner_metrics.h"
#include "sam.h"
#ifdef CHUD_PROFILING
#include <CHUD/CHUD.h>
#endif

using namespace std;
using namespace seqan;

void prepareSearchOptions() {
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	//ctx->resetOptions();

	bool paired = ctx->mates1.size() > 0 || ctx->mates2.size() > 0 || ctx->mates12.size() > 0;
	if(ctx->rangeMode) {
		// Tell the Ebwt loader to ignore the suffix-array portion of
		// the index.  We don't need it because the user isn't asking
		// for bowtie to report reference positions (just matrix
		// ranges).
		ctx->offRate = 32;
	}
	if(!ctx->maqLike && ctx->mismatches == 3) {
		// Much faster than normal 3-mismatch mode
		ctx->stateful = true;
	}
// 	if(ctx->mates1.size() != ctx->mates2.size()) {
// 		ostringstream err;
// 		err << "Error: " << mates1.size() << " mate files/sequences were specified with -1, but " << mates2.size() << endl
// 			<< "mate files/sequences were specified with -2.  The same number of mate files/" << endl
// 			<< "sequences must be specified with -1 and -2." << endl;
// 		throw BowtieException(err.str());
// 	}
// 	if(ctx->qualities.size() && ctx->format != FASTA) {
// 		ostringstream err;
// 		err << "Error: one or more quality files were specified with -Q but -f was not" << endl
// 			<< "enabled.  -Q works only in combination with -f and -C." << endl;
// 		throw BowtieException(err.str());
// 	}
// 	if(qualities.size() && !color) {
// 		ostringstream err;
// 		err << "Error: one or more quality files were specified with -Q but -C was not" << endl
// 			<< "enabled.  -Q works only in combination with -f and -C." << endl;
// 		throw BowtieException(err.str());
// 	}
// 	if(qualities1.size() && format != FASTA) {
// 		ostringstream err;
// 		err << "Error: one or more quality files were specified with --Q1 but -f was not" << endl
// 			<< "enabled.  --Q1 works only in combination with -f and -C." << endl;
// 		throw BowtieException(err.str());
// 	}
// 	if(qualities1.size() && !color) {
// 		ostringstream err;
// 		err << "Error: one or more quality files were specified with --Q1 but -C was not" << endl
// 			<< "enabled.  --Q1 works only in combination with -f and -C." << endl;
// 		throw BowtieException(err.str());
// 	}
// 	if(qualities2.size() && format != FASTA) {
// 		ostringstream err;
// 		err << "Error: one or more quality files were specified with --Q2 but -f was not" << endl
// 			<< "enabled.  --Q2 works only in combination with -f and -C." << endl;
// 		throw BowtieException(err.str());
// 	}
// 	if(qualities2.size() && !color) {
// 		ostringstream err;
// 		err << "Error: one or more quality files were specified with --Q2 but -C was not" << endl
// 			<< "enabled.  --Q2 works only in combination with -f and -C." << endl;
// 		throw BowtieException(err.str());
// 	}
// 	if(qualities1.size() > 0 && mates1.size() != qualities1.size()) {
// 		ostringstream err;
// 		err << "Error: " << mates1.size() << " mate files/sequences were specified with -1, but " << qualities1.size() << endl
// 			<< "quality files were specified with --Q1.  The same number of mate and quality" << endl
// 			<< "files must sequences must be specified with -1 and --Q1." << endl;
// 		throw BowtieException(err.str());
// 	}
// 	if(qualities2.size() > 0 && mates2.size() != qualities2.size()) {
// 		ostringstream err; err << "Error: " << mates2.size() << " mate files/sequences were specified with -2, but " << qualities2.size() << endl
// 			<< "quality files were specified with --Q2.  The same number of mate and quality" << endl
// 			<< "files must sequences must be specified with -2 and --Q2." << endl;
// 		throw BowtieException(err.str());
// 	}
	// Check for duplicate mate input files
	// if(format != CMDLINE) {
	// 	ostringstream err;
	// 	for(size_t i = 0; i < mates1.size(); i++) {
	// 		for(size_t j = 0; j < mates2.size(); j++) {
	// 			if(mates1[i] == mates2[j] && !quiet) {
	// 				err << "Warning: Same mate file \"" << mates1[i] << "\" appears as argument to both -1 and -2" << endl;
	// 			}
	// 		}
	// 	}
	// }

	if(ctx->tryHard) {
		// Increase backtracking limit to huge number
		ctx->maxBts = ctx->maxBtsBetter = INT_MAX;
		// Increase number of paired-end scan attempts to huge number
		ctx->mixedAttemptLim = UINT_MAX;
	}
	if(!ctx->stateful && ctx->sampleMax) {
		ctx->stateful = true;
	}
	if(ctx->strata && !ctx->stateful) {
		ostringstream err;
		err << "--strata must be combined with --best" << endl;
		throw BowtieException(err.str());
	}
	if(ctx->strata && !ctx->allHits && ctx->khits == 1 && ctx->mhits == 0xffffffff) {
		ostringstream err; err << "--strata has no effect unless combined with -k, -m or -a" << endl;
		throw BowtieException(err.str());
	}
	if(ctx->fuzzy && (!ctx->stateful && !paired)) {
		ostringstream err; err << "--fuzzy must be combined with --best or paired-end alignment" << endl;
		throw BowtieException(err.str());
	}
	// If both -s and -u are used, we need to adjust qUpto accordingly
	// since it uses patid to know if we've reached the -u limit (and
	// patids are all shifted up by skipReads characters)
	if(ctx->qUpto + ctx->skipReads > ctx->qUpto) {
		ctx->qUpto += ctx->skipReads;
	}
	if(ctx->useShmem && ctx->useMm && !ctx->quiet) {
		//cerr << "Warning: --shmem overrides --mm..." << endl;
		ctx->useMm = false;
	}
	if(ctx->snpPhred <= 10 && ctx->color && !ctx->quiet) {
		//cerr << "Warning: the colorspace SNP penalty (--snpphred) is very low: " << ctx->snpPhred << endl;
	}
// 	if(format == INPUT_CHAIN) {
// 		bool error = false;
// 		ostringstream err;
// 		if(!stateful) {
// 			err << "Error: --chainin must be combined with --best; aborting..." << endl;
// 			error = true;
// 		}
// 		if(paired) {
// 			err << "Error: --chainin cannot be combined with paired-end alignment; aborting..." << endl;
// 			error = true;
// 		}
// 		if(error) throw BowtieException(err.str());
// 	}

// 	if(ctx->outType == OUTPUT_CHAIN) {
// 		bool error = false;
// 		ostringstream err;
// 		if(refOut) {
// 			err << "Error: --chainout is not compatible with --refout; aborting..." << endl;
// 			error = true;
// 		}
// 		if(!stateful) {
// 			err << "Error: --chainout must be combined with --best; aborting..." << endl;
// 			error = true;
// 		}
// 		if(paired) {
// 			err << "Error: --chainout cannot be combined with paired-end alignment; aborting..." << endl;
// 			error = true;
// 		}
// 		if(error) throw BowtieException(err.str());
// 	}
// 	if(outType == OUTPUT_SAM && refOut) {
// 		ostringstream err; err << "Error: --refout cannot be combined with -S/--sam" << endl;
// 		throw BowtieException(err.str());
// 	}
	if(!ctx->mateFwSet) {
		if(ctx->color) {
			// Set colorspace default (--ff)
			ctx->mate1fw = true;
			ctx->mate2fw = true;
		} else {
			// Set nucleotide space default (--fr)
			ctx->mate1fw = true;
			ctx->mate2fw = false;
		}
	}
// 	if(outType != OUTPUT_FULL && suppressOuts.count() > 0 && !quiet) {
// 		cerr << "Warning: Ignoring --suppress because output type is not default." << endl;
// 		cerr << "         --suppress is only available for the default output type." << endl;
// 		suppressOuts.clear();
// 	}
}
static const char *argv0 = NULL;

#define FINISH_READ(p) \
	/* Don't do finishRead if the read isn't legit or if the read was skipped by the doneMask */ \
	if(!p->empty()) { \
	sink->finishRead(*p, true, !skipped); \
	} \
	skipped = false;

static inline void finishReadWithHitmask(PatternSourcePerThread* p,
										 HitSinkPerThread* sink,
										 SyncBitset& hitMask,
										 bool r,
										 bool& skipped)
{
	/* Don't do finishRead if the read isn't legit */
	if(!p->empty()) {
		/* r = whether to consider reporting the read as unaligned */
		bool reportUnAl = r;
		if(reportUnAl) {
			/* If the done-mask already shows the read as done, */
			/* then we already reported the unaligned read and */
			/* should refrain from re-reporting*/
			reportUnAl = !skipped;
			if(reportUnAl) {
				/* If there hasn't been a hit reported, then report */
				/* read as unaligned */
				reportUnAl = !hitMask.test(p->patid());
			}
		}
		if(sink->finishRead(*p, true, reportUnAl) > 0) {
			/* We reported a hit for the read, so we set the */
			/* appropriate bit in the hitMask to prevent it from */
			/* being reported as unaligned. */
			if(!reportUnAl && sink->dumpsReads()) {
				hitMask.setOver(p->patid());
			}
		}
	}
	skipped = false;
}

/// Macro for getting the next read, possibly aborting depending on
/// whether the result is empty or the patid exceeds the limit, and
/// marshaling the read into convenient variables.
#define GET_READ(p) \
	p->nextReadPair(); \
	if(p->empty() || p->patid() >= ctx->qUpto) { \
	p->bufa().clearAll(); \
	break; \
	} \
	assert(!empty(p->bufa().patFw)); \
	String<Dna5>& patFw  = p->bufa().patFw;  \
	patFw.data_begin += 0; /* suppress "unused" compiler warning */ \
	String<Dna5>& patRc  = p->bufa().patRc;  \
	patRc.data_begin += 0; /* suppress "unused" compiler warning */ \
	String<char>& qual = p->bufa().qual; \
	qual.data_begin += 0; /* suppress "unused" compiler warning */ \
	String<char>& qualRev = p->bufa().qualRev; \
	qualRev.data_begin += 0; /* suppress "unused" compiler warning */ \
	String<Dna5>& patFwRev  = p->bufa().patFwRev;  \
	patFwRev.data_begin += 0; /* suppress "unused" compiler warning */ \
	String<Dna5>& patRcRev  = p->bufa().patRcRev;  \
	patRcRev.data_begin += 0; /* suppress "unused" compiler warning */ \
	String<char>& name   = p->bufa().name;   \
	name.data_begin += 0; /* suppress "unused" compiler warning */ \
	uint32_t      patid  = p->patid();       \
	params.setPatId(patid);

/// Macro for getting the forward oriented version of next read,
/// possibly aborting depending on whether the result is empty or the
/// patid exceeds the limit, and marshaling the read into convenient
/// variables.
#define GET_READ_FW(p) \
	p->nextReadPair(); \
	if(p->empty() || p->patid() >= qUpto) { \
	p->bufa().clearAll(); \
	break; \
	} \
	params.setPatId(p->patid()); \
	assert(!empty(p->bufa().patFw)); \
	String<Dna5>& patFw  = p->bufa().patFw;  \
	patFw.data_begin += 0; /* suppress "unused" compiler warning */ \
	String<char>& qual = p->bufa().qual; \
	qual.data_begin += 0; /* suppress "unused" compiler warning */ \
	String<Dna5>& patFwRev  = p->bufa().patFwRev;  \
	patFwRev.data_begin += 0; /* suppress "unused" compiler warning */ \
	String<char>& qualRev = p->bufa().qualRev; \
	qualRev.data_begin += 0; /* suppress "unused" compiler warning */ \
	String<char>& name   = p->bufa().name;   \
	name.data_begin += 0; /* suppress "unused" compiler warning */ \
	uint32_t      patid  = p->patid();

#ifdef BOWTIE_PTHREADS
#define WORKER_EXIT() \
	patsrcFact->destroy(patsrc); \
	delete patsrcFact; \
	sinkFact->destroy(sink); \
	delete sinkFact; \
	if(tid > 0) { \
	exitThread(NULL); \
	} \
	return NULL;
#else
#define WORKER_EXIT() \
	patsrcFact->destroy(patsrc); \
	delete patsrcFact; \
	sinkFact->destroy(sink); \
	delete sinkFact; \
	return NULL;
#endif

#ifdef CHUD_PROFILING
#define CHUD_START() chudStartRemotePerfMonitor("Bowtie");
#define CHUD_STOP()  chudStopRemotePerfMonitor();
#else
#define CHUD_START()
#define CHUD_STOP()
#endif

/// Create a PatternSourcePerThread for the current thread according
/// to the global params and return a pointer to it
static PatternSourcePerThreadFactory*
createPatsrcFactory(PairedPatternSource& _patsrc, int tid) {
	PatternSourcePerThreadFactory *patsrcFact;
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	if(ctx->randReadsNoSync) {
		patsrcFact = new RandomPatternSourcePerThreadFactory(ctx->numRandomReads, ctx->lenRandomReads, ctx->nthreads, tid);
	} else {
		patsrcFact = new WrappedPatternSourcePerThreadFactory(_patsrc);
	}
	assert(patsrcFact != NULL);
	return patsrcFact;
}

/**
* Allocate a HitSinkPerThreadFactory on the heap according to the
* global params and return a pointer to it.
*/
static HitSinkPerThreadFactory*
createSinkFactory(HitSink& _sink) {
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	bool &nofw = ctx->nofw;
	bool &norc = ctx->norc;
	HitSinkPerThreadFactory *sink = NULL;
	if(ctx->format == INPUT_CHAIN) {
		assert(ctx->stateful);
		sink = new ChainingHitSinkPerThreadFactory(_sink, ctx->allHits ? 0xffffffff : ctx->khits, ctx->mhits, ctx->strata);
	} else if(!ctx->strata) {
		// Unstratified
		if(!ctx->allHits) {
			// First N good; "good" inherently ignores strata
			sink = new NGoodHitSinkPerThreadFactory(_sink, ctx->khits, ctx->mhits);
		} else {
			// All hits, spanning strata
			sink = new AllHitSinkPerThreadFactory(_sink, ctx->mhits);
		}
	} else {
		// Stratified
		assert(ctx->stateful);
		if(!ctx->allHits) {
			assert(ctx->stateful);
			// Buffer best hits, assuming they're arriving in best-
			// to-worst order
			sink = new NBestFirstStratHitSinkPerThreadFactory(_sink, ctx->khits, ctx->mhits);
		} else {
			assert(ctx->stateful);
			// Buffer best hits, assuming they're arriving in best-
			// to-worst order
			sink = new NBestFirstStratHitSinkPerThreadFactory(_sink, 0xffffffff/2, ctx->mhits);
		}
	}
	assert(sink != NULL);
	return sink;
}

/**
* Search through a single (forward) Ebwt index for exact end-to-end
* hits.  Assumes that index is already loaded into memory.
*/

static void *exactSearchWorker(void *vp) {
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	bool &nofw = ctx->nofw;
	bool &norc = ctx->norc;
	int tid = *((int*)vp);
	PairedPatternSource& _patsrc = *(ctx->exactSearch_patsrc);
	HitSink& _sink               = *(ctx->exactSearch_sink);
	Ebwt<String<Dna> >& ebwt     = *(ctx->exactSearch_ebwt);
	vector<String<Dna5> >& os    = *(ctx->exactSearch_os);
	const BitPairReference* refs =  ctx->exactSearch_refs;

	// Per-thread initialization
	PatternSourcePerThreadFactory *patsrcFact = createPatsrcFactory(_patsrc, tid);
	PatternSourcePerThread *patsrc = patsrcFact->create();
	HitSinkPerThreadFactory* sinkFact = createSinkFactory(_sink);
	HitSinkPerThread* sink = sinkFact->create();
	EbwtSearchParams<String<Dna> > params(
		*(sink),      // HitSink
		os,         // reference sequences
		true,       // read is forward
		true);       // index is forward
	GreedyDFSRangeSource bt(
		&ebwt, params,
		refs,           // reference sequence (for colorspace)
		0xffffffff,     // qualThresh
		0xffffffff,     // max backtracks (no max)
		0,              // reportPartials (don't)
		true,           // reportExacts
		ctx->rangeMode,      // reportRanges
		NULL,           // seedlings
		NULL,           // mutations
		ctx->verbose,        // verbose
		&os,
		false);         // considerQuals
	bool skipped = false;
	while(true) {
		if(BowtieContext::isCanceled()) {
			break;
		}
		FINISH_READ(patsrc);
		GET_READ(patsrc);
#include "search_exact.c"
	}
	FINISH_READ(patsrc);
	WORKER_EXIT();
}

/**
* A statefulness-aware worker driver.  Uses UnpairedExactAlignerV1.
*/
static void *exactSearchWorkerStateful(void *vp) {
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	bool &nofw = ctx->nofw; 
	bool &norc = ctx->norc;

	int tid = *((int*)vp);
	PairedPatternSource& _patsrc = *(ctx->exactSearch_patsrc);
	HitSink& _sink               = *(ctx->exactSearch_sink);
	Ebwt<String<Dna> >& ebwt     = *(ctx->exactSearch_ebwt);
	vector<String<Dna5> >& os    = *(ctx->exactSearch_os);
	BitPairReference* refs       =  ctx->exactSearch_refs;

	// Global initialization
	PatternSourcePerThreadFactory* patsrcFact = createPatsrcFactory(_patsrc, tid);
	HitSinkPerThreadFactory* sinkFact = createSinkFactory(_sink);

	ChunkPool *pool = new ChunkPool(ctx->chunkSz * 1024, ctx->chunkPoolMegabytes * 1024 * 1024, ctx->chunkVerbose);
	UnpairedExactAlignerV1Factory alSEfact(
		ebwt,
		NULL,
		!nofw,
		!norc,
		_sink,
		*sinkFact,
		NULL, //&cacheFw,
		NULL, //&cacheBw,
		ctx->cacheLimit,
		pool,
		refs,
		os,
		!ctx->noMaqRound,
		!ctx->better,
		ctx->strandFix,
		ctx->rangeMode,
		ctx->verbose,
		ctx->quiet,
		ctx->seed);
	PairedExactAlignerV1Factory alPEfact(
		ebwt,
		NULL,
		ctx->color,
		!nofw,
		!norc,
		ctx->useV1,
		_sink,
		*sinkFact,
		ctx->mate1fw,
		ctx->mate2fw,
		ctx->minInsert,
		ctx->maxInsert,
		ctx->dontReconcileMates,
		ctx->mhits,       // for symCeiling
		ctx->mixedThresh,
		ctx->mixedAttemptLim,
		NULL, //&cacheFw,
		NULL, //&cacheBw,
		ctx->cacheLimit,
		pool,
		refs, os,
		ctx->reportSe,
		!ctx->noMaqRound,
		ctx->strandFix,
		!ctx->better,
		ctx->rangeMode,
		ctx->verbose,
		ctx->quiet,
		ctx->seed);
	{
		MixedMultiAligner multi(
			ctx->prefetchWidth,
			ctx->qUpto,
			alSEfact,
			alPEfact,
			*patsrcFact);
		// Run that mother
		multi.run();
		// MultiAligner must be destroyed before patsrcFact
	}

	delete patsrcFact;
	delete sinkFact;
	delete pool;
#ifdef BOWTIE_PTHREADS
	if(tid > 0) exitThread(NULL);
#endif
	return NULL;
}

#define SET_A_FW(bt, p, params) \
	bt.setQuery(&p->bufa().patFw, &p->bufa().qual, &p->bufa().name); \
	params.setFw(true);
#define SET_A_RC(bt, p, params) \
	bt.setQuery(&p->bufa().patRc, &p->bufa().qualRev, &p->bufa().name); \
	params.setFw(false);
#define SET_B_FW(bt, p, params) \
	bt.setQuery(&p->bufb().patFw, &p->bufb().qual, &p->bufb().name); \
	params.setFw(true);
#define SET_B_RC(bt, p, params) \
	bt.setQuery(&p->bufb().patRc, &p->bufb().qualRev, &p->bufb().name); \
	params.setFw(false);

#ifdef BOWTIE_PTHREADS
#define PTHREAD_ATTRS (PTHREAD_CREATE_JOINABLE | PTHREAD_CREATE_DETACHED)
#endif

/**
* Search through a single (forward) Ebwt index for exact end-to-end
* hits.  Assumes that index is already loaded into memory.
*/
static void exactSearch(PairedPatternSource& _patsrc,
						HitSink& _sink,
						Ebwt<String<Dna> >& ebwt,
						vector<String<Dna5> >& os)
{
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	bool &color = ctx->color;
	ctx->exactSearch_patsrc = &_patsrc;
	ctx->exactSearch_sink   = &_sink;
	ctx->exactSearch_ebwt   = &ebwt;
	ctx->exactSearch_os     = &os;
	assert(!ebwt.isInMemory());
	{
		// Load the rest of (vast majority of) the backward Ebwt into
		// memory
		Timer _t(cerr, "Time loading forward index: ", ctx->timing);
		ebwt.loadIntoMemory(color ? 1 : 0, -1, !ctx->noRefNames, ctx->startVerbose);
	}

	BitPairReference *refs = NULL;
	bool pair = ctx->mates1.size() > 0 || ctx->mates12.size() > 0;
	if(color || (pair && ctx->mixedThresh < 0xffffffff)) {
		Timer _t(cerr, "Time loading reference: ", ctx->timing);
		refs = new BitPairReference(ctx->adjustedEbwtFileBase, color, ctx->sanityCheck, NULL, &os, false, true, ctx->useMm, ctx->useShmem, ctx->mmSweep, ctx->verbose, ctx->startVerbose);
		if(!refs->loaded()) throw BowtieException("Bit pair reference not loaded");
	}
	ctx->exactSearch_refs   = refs;

#ifdef BOWTIE_PTHREADS
	AutoArray<pthread_t> threads(ctx->nthreads-1);
	AutoArray<int> tids(ctx->nthreads-1);
#endif
	CHUD_START();
	{
		Timer _t(cerr, "Time for 0-mismatch search: ", ctx->timing);
#ifdef BOWTIE_PTHREADS
		for(int i = 0; i < ctx->nthreads-1; i++) {
			tids[i] = i+1;
			if(ctx->stateful) {
				createThread(&threads[i],
					exactSearchWorkerStateful,
					(void *)&tids[i]);
			} else {
				createThread(&threads[i],
					exactSearchWorker,
					(void *)&tids[i]);
			}
		}
#endif
		int tmp = 0;
		if(ctx->stateful) exactSearchWorkerStateful((void*)&tmp);
		else         exactSearchWorker((void*)&tmp);
#ifdef BOWTIE_PTHREADS
		for(int i = 0; i < ctx->nthreads-1; i++) joinThread(threads[i]);
#endif
	}
	if(refs != NULL) delete refs;
}

/**
* Search through a pair of Ebwt indexes, one for the forward direction
* and one for the backward direction, for exact end-to-end hits and 1-
* mismatch end-to-end hits.  In my experience, this is slightly faster
* than Maq (default) mode with the -n 1 option.
*
* Forward Ebwt (ebwtFw) is already loaded into memory and backward
* Ebwt (ebwtBw) is not loaded into memory.
*/


/**
* A statefulness-aware worker driver.  Uses Unpaired/Paired1mmAlignerV1.
*/
static void *mismatchSearchWorkerFullStateful(void *vp) {
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	bool &color = ctx->color;
	int tid = *((int*)vp);
	PairedPatternSource&   _patsrc = *(ctx->mismatchSearch_patsrc);
	HitSink&               _sink   = *(ctx->mismatchSearch_sink);
	Ebwt<String<Dna> >&    ebwtFw  = *(ctx->mismatchSearch_ebwtFw);
	Ebwt<String<Dna> >&    ebwtBw  = *(ctx->mismatchSearch_ebwtBw);
	vector<String<Dna5> >& os      = *(ctx->mismatchSearch_os);
	BitPairReference*      refs    =  ctx->mismatchSearch_refs;

	// Global initialization
	PatternSourcePerThreadFactory* patsrcFact = createPatsrcFactory(_patsrc, tid);
	HitSinkPerThreadFactory* sinkFact = createSinkFactory(_sink);
	ChunkPool *pool = new ChunkPool(ctx->chunkSz * 1024, ctx->chunkPoolMegabytes * 1024 * 1024, ctx->chunkVerbose);

	Unpaired1mmAlignerV1Factory alSEfact(
		ebwtFw,
		&ebwtBw,
		!ctx->nofw,
		!ctx->norc,
		_sink,
		*sinkFact,
		NULL, //&cacheFw,
		NULL, //&cacheBw,
		ctx->cacheLimit,
		pool,
		refs,
		os,
		!ctx->noMaqRound,
		!ctx->better,
		ctx->strandFix,
		ctx->rangeMode,
		ctx->verbose,
		ctx->quiet,
		ctx->seed);
	Paired1mmAlignerV1Factory alPEfact(
		ebwtFw,
		&ebwtBw,
		color,
		!ctx->nofw,
		!ctx->norc,
		ctx->useV1,
		_sink,
		*sinkFact,
		ctx->mate1fw,
		ctx->mate2fw,
		ctx->minInsert,
		ctx->maxInsert,
		ctx->dontReconcileMates,
		ctx->mhits,     // for symCeiling
		ctx->mixedThresh,
		ctx->mixedAttemptLim,
		NULL, //&cacheFw,
		NULL, //&cacheBw,
		ctx->cacheLimit,
		pool,
		refs, os,
		ctx->reportSe,
		!ctx->noMaqRound,
		!ctx->better,
		ctx->strandFix,
		ctx->rangeMode,
		ctx->verbose,
		ctx->quiet,
		ctx->seed);
	{
		MixedMultiAligner multi(
			ctx->prefetchWidth,
			ctx->qUpto,
			alSEfact,
			alPEfact,
			*patsrcFact);
		// Run that mother
		multi.run();
		// MultiAligner must be destroyed before patsrcFact
	}

	delete patsrcFact;
	delete sinkFact;
	delete pool;
#ifdef BOWTIE_PTHREADS
	if(tid > 0) exitThread(NULL);
#endif
	return NULL;
}

static void* mismatchSearchWorkerFull(void *vp){
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	bool &nofw = ctx->nofw;
	bool &norc = ctx->norc;
	int tid = *((int*)vp);
	PairedPatternSource&   _patsrc   = *(ctx->mismatchSearch_patsrc);
	HitSink&               _sink     = *(ctx->mismatchSearch_sink);
	Ebwt<String<Dna> >&    ebwtFw    = *(ctx->mismatchSearch_ebwtFw);
	Ebwt<String<Dna> >&    ebwtBw    = *(ctx->mismatchSearch_ebwtBw);
	vector<String<Dna5> >& os        = *(ctx->mismatchSearch_os);
	const BitPairReference* refs     =  ctx->mismatchSearch_refs;

	// Per-thread initialization
	PatternSourcePerThreadFactory* patsrcFact = createPatsrcFactory(_patsrc, tid);
	PatternSourcePerThread* patsrc = patsrcFact->create();
	HitSinkPerThreadFactory* sinkFact = createSinkFactory(_sink);
	HitSinkPerThread* sink = sinkFact->create();
	EbwtSearchParams<String<Dna> > params(
		*sink,      // HitSinkPerThread
		os,         // reference sequences
		true,       // read is forward
		false);     // index is mirror index
	GreedyDFSRangeSource bt(
		&ebwtFw, params,
		refs,           // reference sequence (for colorspace)
		0xffffffff,     // qualThresh
		0xffffffff,     // max backtracks (no max)
		0,              // reportPartials (don't)
		true,           // reportExacts
		ctx->rangeMode,      // reportRanges
		NULL,           // seedlings
		NULL,           // mutations
		ctx->verbose,        // verbose
		&os,
		false);         // considerQuals
	bool skipped = false;
	while(true) {
		if(BowtieContext::isCanceled()) {
			break;
		}
		FINISH_READ(patsrc);
		GET_READ(patsrc);
		uint32_t plen = length(patFw);
		uint32_t s = plen;
		uint32_t s3 = s >> 1; // length of 3' half of seed
		uint32_t s5 = (s >> 1) + (s & 1); // length of 5' half of seed
#define DONEMASK_SET(p)
#include "search_1mm_phase1.c"
#include "search_1mm_phase2.c"
#undef DONEMASK_SET
	} // End read loop
	FINISH_READ(patsrc);
	WORKER_EXIT();
}

/**
* Search through a single (forward) Ebwt index for exact end-to-end
* hits.  Assumes that index is already loaded into memory.
*/
static void mismatchSearchFull(PairedPatternSource& _patsrc,
							   HitSink& _sink,
							   Ebwt<String<Dna> >& ebwtFw,
							   Ebwt<String<Dna> >& ebwtBw,
							   vector<String<Dna5> >& os)
{
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	ctx->mismatchSearch_patsrc       = &_patsrc;
	ctx->mismatchSearch_sink         = &_sink;
	ctx->mismatchSearch_ebwtFw       = &ebwtFw;
	ctx->mismatchSearch_ebwtBw       = &ebwtBw;
	ctx->mismatchSearch_doneMask     = NULL;
	ctx->mismatchSearch_hitMask      = NULL;
	ctx->mismatchSearch_os           = &os;

	assert(!ebwtFw.isInMemory());
	assert(!ebwtBw.isInMemory());
	{
		// Load the other half of the index into memory
		Timer _t(cerr, "Time loading forward index: ", ctx->timing);
		ebwtFw.loadIntoMemory(ctx->color ? 1 : 0, -1, !ctx->noRefNames, ctx->startVerbose);
	}
	{
		// Load the other half of the index into memory
		Timer _t(cerr, "Time loading mirror index: ", ctx->timing);
		ebwtBw.loadIntoMemory(ctx->color ? 1 : 0, -1, !ctx->noRefNames, ctx->startVerbose);
	}
	// Create range caches, which are shared among all aligners
	BitPairReference *refs = NULL;
	bool pair = ctx->mates1.size() > 0 || ctx->mates12.size() > 0;
	if(ctx->color || (pair && ctx->mixedThresh < 0xffffffff)) {
		Timer _t(cerr, "Time loading reference: ", ctx->timing);
                refs = new BitPairReference(ctx->adjustedEbwtFileBase, ctx->color, ctx->sanityCheck, NULL, &os, false, true, ctx->useMm, ctx->useShmem, ctx->mmSweep, ctx->verbose, ctx->startVerbose);
		if(!refs->loaded()) throw BowtieException("Bit pair reference not loaded");
	}
	ctx->mismatchSearch_refs = refs;

#ifdef BOWTIE_PTHREADS
	// Allocate structures for threads
	AutoArray<pthread_t> threads(ctx->nthreads-1);
	AutoArray<int> tids(ctx->nthreads-1);
#endif
	CHUD_START();
	{
		Timer _t(cerr, "Time for 1-mismatch full-index search: ", ctx->timing);
#ifdef BOWTIE_PTHREADS
		for(int i = 0; i < ctx->nthreads-1; i++) {
			tids[i] = i+1;
			if(ctx->stateful)
				createThread(&threads[i], mismatchSearchWorkerFullStateful, (void *)&tids[i]);
			else
				createThread(&threads[i], mismatchSearchWorkerFull, (void *)&tids[i]);
		}
#endif
		// Go to town
		int tmp = 0;
		if(ctx->stateful) mismatchSearchWorkerFullStateful((void*)&tmp);
		else         mismatchSearchWorkerFull((void*)&tmp);
#ifdef BOWTIE_PTHREADS
		for(int i = 0; i < ctx->nthreads-1; i++) joinThread(threads[i]);
#endif
	}
	if(refs != NULL) delete refs;
}

#define SWITCH_TO_FW_INDEX() { \
	/* Evict the mirror index from memory if necessary */ \
	if(ebwtBw.isInMemory()) ebwtBw.evictFromMemory(); \
	assert(!ebwtBw.isInMemory()); \
	/* Load the forward index into memory if necessary */ \
	if(!ebwtFw.isInMemory()) { \
	Timer _t(cerr, "Time loading forward index: ", ctx->timing); \
	ebwtFw.loadIntoMemory(ctx->color ? 1 : 0, -1, !ctx->noRefNames, ctx->startVerbose); \
	} \
	assert(ebwtFw.isInMemory()); \
	_patsrc.reset(); /* rewind pattern source to first pattern */ \
}

#define SWITCH_TO_BW_INDEX() { \
	/* Evict the forward index from memory if necessary */ \
	if(ebwtFw.isInMemory()) ebwtFw.evictFromMemory(); \
	assert(!ebwtFw.isInMemory()); \
	/* Load the forward index into memory if necessary */ \
	if(!ebwtBw.isInMemory()) { \
	Timer _t(cerr, "Time loading mirror index: ", timing); \
	ebwtBw.loadIntoMemory(ctx->color ? 1 : 0, !noRefNames, startVerbose); \
	} \
	assert(ebwtBw.isInMemory()); \
	_patsrc.reset(); /* rewind pattern source to first pattern */ \
}

#define ASSERT_NO_HITS_FW(ebwtfw) \
	if(sanityCheck && os.size() > 0) { \
	vector<Hit> hits; \
	uint32_t threeRevOff = (seedMms <= 3) ? s : 0; \
	uint32_t twoRevOff   = (seedMms <= 2) ? s : 0; \
	uint32_t oneRevOff   = (seedMms <= 1) ? s : 0; \
	uint32_t unrevOff    = (seedMms == 0) ? s : 0; \
	if(hits.size() > 0) { \
	/* Print offending hit obtained by oracle */ \
	::printHit( \
	os, \
	hits[0], \
	patFw, \
	plen, \
	unrevOff, \
	oneRevOff, \
	twoRevOff, \
	threeRevOff, \
	ebwtfw);  /* ebwtFw */ \
	} \
	assert_eq(0, hits.size()); \
	}

#define ASSERT_NO_HITS_RC(ebwtfw) \
	if(sanityCheck && os.size() > 0) { \
	vector<Hit> hits; \
	uint32_t threeRevOff = (seedMms <= 3) ? s : 0; \
	uint32_t twoRevOff   = (seedMms <= 2) ? s : 0; \
	uint32_t oneRevOff   = (seedMms <= 1) ? s : 0; \
	uint32_t unrevOff    = (seedMms == 0) ? s : 0; \
	if(hits.size() > 0) { \
	/* Print offending hit obtained by oracle */ \
	::printHit( \
	os, \
	hits[0], \
	patRc, \
	plen, \
	unrevOff, \
	oneRevOff, \
	twoRevOff, \
	threeRevOff, \
	ebwtfw);  /* ebwtFw */ \
	} \
	assert_eq(0, hits.size()); \
	}

#define TWOTHREE_WORKER_SETUP() \
	int tid = *((int*)vp); \
	PairedPatternSource&           _patsrc  = *(ctx->twoOrThreeMismatchSearch_patsrc);   \
	HitSink&                       _sink    = *(ctx->twoOrThreeMismatchSearch_sink);     \
	vector<String<Dna5> >&         os       = *(ctx->twoOrThreeMismatchSearch_os);       \
	bool                           two      = ctx->twoOrThreeMismatchSearch_two; \
	PatternSourcePerThreadFactory* patsrcFact = createPatsrcFactory(_patsrc, tid); \
	PatternSourcePerThread* patsrc = patsrcFact->create(); \
	HitSinkPerThreadFactory* sinkFact = createSinkFactory(_sink); \
	HitSinkPerThread* sink = sinkFact->create(); \
	/* Per-thread initialization */ \
	EbwtSearchParams<String<Dna> > params( \
	*sink,       /* HitSink */ \
	os,          /* reference sequences */ \
	true,        /* read is forward */ \
	true);       /* index is forward */

/**
* A statefulness-aware worker driver.  Uses UnpairedExactAlignerV1.
*/
static void *twoOrThreeMismatchSearchWorkerStateful(void *vp) {
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	int tid = *((int*)vp);
	PairedPatternSource&   _patsrc = *(ctx->twoOrThreeMismatchSearch_patsrc);
	HitSink&               _sink   = *(ctx->twoOrThreeMismatchSearch_sink);
	Ebwt<String<Dna> >&    ebwtFw  = *(ctx->twoOrThreeMismatchSearch_ebwtFw);
	Ebwt<String<Dna> >&    ebwtBw  = *(ctx->twoOrThreeMismatchSearch_ebwtBw);
	vector<String<Dna5> >& os      = *(ctx->twoOrThreeMismatchSearch_os);
	BitPairReference*      refs    =  ctx->twoOrThreeMismatchSearch_refs;
	static bool            two     =  ctx->twoOrThreeMismatchSearch_two;

	// Global initialization
	PatternSourcePerThreadFactory* patsrcFact = createPatsrcFactory(_patsrc, tid);
	HitSinkPerThreadFactory* sinkFact = createSinkFactory(_sink);

	ChunkPool *pool = new ChunkPool(ctx->chunkSz * 1024, ctx->chunkPoolMegabytes * 1024 * 1024, ctx->chunkVerbose);
	Unpaired23mmAlignerV1Factory alSEfact(
		ebwtFw,
		&ebwtBw,
		two,
		!ctx->nofw,
		!ctx->norc,
		_sink,
		*sinkFact,
		NULL, //&cacheFw,
		NULL, //&cacheBw,
		ctx->cacheLimit,
		pool,
		refs,
		os,
		!ctx->noMaqRound,
		!ctx->better,
		ctx->strandFix,
		ctx->rangeMode,
		ctx->verbose,
		ctx->quiet,
		ctx->seed);
	Paired23mmAlignerV1Factory alPEfact(
		ebwtFw,
		&ebwtBw,
		ctx->color,
		!ctx->nofw,
		!ctx->norc,
		ctx->useV1,
		two,
		_sink,
		*sinkFact,
		ctx->mate1fw,
		ctx->mate2fw,
		ctx->minInsert,
		ctx->maxInsert,
		ctx->dontReconcileMates,
		ctx->mhits,       // for symCeiling
		ctx->mixedThresh,
		ctx->mixedAttemptLim,
		NULL, //&cacheFw,
		NULL, //&cacheBw,
		ctx->cacheLimit,
		pool,
		refs, os,
		ctx->reportSe,
		!ctx->noMaqRound,
		!ctx->better,
		ctx->strandFix,
		ctx->rangeMode,
		ctx->verbose,
		ctx->quiet,
		ctx->seed);
	{
		MixedMultiAligner multi(
			ctx->prefetchWidth,
			ctx->qUpto,
			alSEfact,
			alPEfact,
			*patsrcFact);
		// Run that mother
		multi.run();
		// MultiAligner must be destroyed before patsrcFact
	}

	delete patsrcFact;
	delete sinkFact;
	delete pool;
#ifdef BOWTIE_PTHREADS
	if(tid > 0) exitThread(NULL);
#endif
	return NULL;
}

static void* twoOrThreeMismatchSearchWorkerFull(void *vp) {
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	bool &nofw = ctx->nofw;
	bool &norc = ctx->norc;
	TWOTHREE_WORKER_SETUP();
	Ebwt<String<Dna> >& ebwtFw = *(ctx->twoOrThreeMismatchSearch_ebwtFw);
	Ebwt<String<Dna> >& ebwtBw = *(ctx->twoOrThreeMismatchSearch_ebwtBw);
	const BitPairReference* refs = ctx->twoOrThreeMismatchSearch_refs;
	GreedyDFSRangeSource btr1(
		&ebwtFw, params,
		refs,           // reference sequence (for colorspace)
		0xffffffff,     // qualThresh
		// Do not impose maximums in 2/3-mismatch mode
		0xffffffff,     // max backtracks (no limit)
		0,              // reportPartials (don't)
		true,           // reportExacts
		ctx->rangeMode,      // reportRanges
		NULL,           // seedlings
		NULL,           // mutations
		ctx->verbose,        // verbose
		&os,
		false);         // considerQuals
	GreedyDFSRangeSource bt2(
		&ebwtBw, params,
		refs,           // reference sequence (for colorspace)
		0xffffffff,     // qualThresh
		// Do not impose maximums in 2/3-mismatch mode
		0xffffffff,     // max backtracks (no limit)
		0,              // reportPartials (no)
		true,           // reportExacts
		ctx->rangeMode,      // reportRanges
		NULL,           // seedlings
		NULL,           // mutations
		ctx->verbose,        // verbose
		&os,
		false);         // considerQuals
	GreedyDFSRangeSource bt3(
		&ebwtFw, params,
		refs,           // reference sequence (for colorspace)
		0xffffffff,     // qualThresh (none)
		// Do not impose maximums in 2/3-mismatch mode
		0xffffffff,     // max backtracks (no limit)
		0,              // reportPartials (don't)
		true,           // reportExacts
		ctx->rangeMode,      // reportRanges
		NULL,           // seedlings
		NULL,           // mutations
		ctx->verbose,        // verbose
		&os,
		false);         // considerQuals
	GreedyDFSRangeSource bthh3(
		&ebwtFw, params,
		refs,           // reference sequence (for colorspace)
		0xffffffff,     // qualThresh
		// Do not impose maximums in 2/3-mismatch mode
		0xffffffff,     // max backtracks (no limit)
		0,              // reportPartials (don't)
		true,           // reportExacts
		ctx->rangeMode,      // reportRanges
		NULL,           // seedlings
		NULL,           // mutations
		ctx->verbose,        // verbose
		&os,
		false,          // considerQuals
		true);          // halfAndHalf
	bool skipped = false;
	while(true) { // Read read-in loop
		if(BowtieContext::isCanceled()) {
			break;
		}
		FINISH_READ(patsrc);
		GET_READ(patsrc);
		patid += 0; // kill unused variable warning
		uint32_t plen = length(patFw);
		uint32_t s = plen;
		uint32_t s3 = s >> 1; // length of 3' half of seed
		uint32_t s5 = (s >> 1) + (s & 1); // length of 5' half of seed
#define DONEMASK_SET(p)
#include "search_23mm_phase1.c"
#include "search_23mm_phase2.c"
#include "search_23mm_phase3.c"
#undef DONEMASK_SET
	}
	FINISH_READ(patsrc);
	// Threads join at end of Phase 1
	WORKER_EXIT();
}

template<typename TStr>
static void twoOrThreeMismatchSearchFull(
	PairedPatternSource& _patsrc,   /// pattern source
	HitSink& _sink,                 /// hit sink
	Ebwt<TStr>& ebwtFw,             /// index of original text
	Ebwt<TStr>& ebwtBw,             /// index of mirror text
	vector<String<Dna5> >& os,      /// text strings, if available (empty otherwise)
	bool two = true)                /// true -> 2, false -> 3
{
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	// Global initialization
	assert(!ebwtFw.isInMemory());
	assert(!ebwtBw.isInMemory());
	{
		// Load the other half of the index into memory
		Timer _t(cerr, "Time loading forward index: ", ctx->timing);
		ebwtFw.loadIntoMemory(ctx->color ? 1 : 0, -1, !ctx->noRefNames, ctx->startVerbose);
	}
	{
		// Load the other half of the index into memory
		Timer _t(cerr, "Time loading mirror index: ", ctx->timing);
		ebwtBw.loadIntoMemory(ctx->color ? 1 : 0, -1, !ctx->noRefNames, ctx->startVerbose);
	}
	// Create range caches, which are shared among all aligners
	BitPairReference *refs = NULL;
	bool pair = ctx->mates1.size() > 0 || ctx->mates12.size() > 0;
	if(ctx->color || (pair && ctx->mixedThresh < 0xffffffff)) {
		Timer _t(cerr, "Time loading reference: ", ctx->timing);
		refs = new BitPairReference(ctx->adjustedEbwtFileBase, ctx->color, ctx->sanityCheck, NULL, &os, false, true, ctx->useMm, ctx->useShmem, ctx->mmSweep, ctx->verbose, ctx->startVerbose);
		if(!refs->loaded()) BowtieException("Bit pair reference not loaded");
	}
	ctx->twoOrThreeMismatchSearch_refs     = refs;
	ctx->twoOrThreeMismatchSearch_patsrc   = &_patsrc;
	ctx->twoOrThreeMismatchSearch_sink     = &_sink;
	ctx->twoOrThreeMismatchSearch_ebwtFw   = &ebwtFw;
	ctx->twoOrThreeMismatchSearch_ebwtBw   = &ebwtBw;
	ctx->twoOrThreeMismatchSearch_os       = &os;
	ctx->twoOrThreeMismatchSearch_doneMask = NULL;
	ctx->twoOrThreeMismatchSearch_hitMask  = NULL;
	ctx->twoOrThreeMismatchSearch_two      = two;

#ifdef BOWTIE_PTHREADS
	AutoArray<pthread_t> threads(ctx->nthreads-1);
	AutoArray<int> tids(ctx->nthreads-1);
#endif
	CHUD_START();
	{
		Timer _t(cerr, "End-to-end 2/3-mismatch full-index search: ", ctx->timing);
#ifdef BOWTIE_PTHREADS
		for(int i = 0; i < ctx->nthreads-1; i++) {
			tids[i] = i+1;
			if(ctx->stateful)
				createThread(&threads[i], twoOrThreeMismatchSearchWorkerStateful, (void *)&tids[i]);
			else
				createThread(&threads[i], twoOrThreeMismatchSearchWorkerFull, (void *)&tids[i]);
		}
#endif
		int tmp = 0;
		if(ctx->stateful) twoOrThreeMismatchSearchWorkerStateful((void*)&tmp);
		else         twoOrThreeMismatchSearchWorkerFull((void*)&tmp);
#ifdef BOWTIE_PTHREADS
		for(int i = 0; i < ctx->nthreads-1; i++) joinThread(threads[i]);
#endif
	}
	if(refs != NULL) delete refs;
	return;
}

#define SEEDEDQUAL_WORKER_SETUP() \
	int tid = *((int*)vp); \
	PairedPatternSource&     _patsrc    = *(ctx->seededQualSearch_patsrc);    \
	HitSink&                 _sink      = *(ctx->seededQualSearch_sink);      \
	vector<String<Dna5> >&   os         = *(ctx->seededQualSearch_os);        \
	int                      qualCutoff = ctx->seededQualSearch_qualCutoff; \
	PatternSourcePerThreadFactory* patsrcFact = createPatsrcFactory(_patsrc, tid); \
	PatternSourcePerThread* patsrc = patsrcFact->create(); \
	HitSinkPerThreadFactory* sinkFact = createSinkFactory(_sink); \
	HitSinkPerThread* sink = sinkFact->create(); \
	/* Per-thread initialization */ \
	EbwtSearchParams<String<Dna> > params( \
	*sink,       /* HitSink */ \
	os,          /* reference sequences */ \
	true,        /* read is forward */ \
	true);       /* index is forward */

static void* seededQualSearchWorkerFull(void *vp) {
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	SEEDEDQUAL_WORKER_SETUP();
	bool &nofw = ctx->nofw;
	bool &norc = ctx->norc;
	bool &verbose = ctx->verbose;
	int &seedLen = ctx->seedLen;
	int &seedMms = ctx->seedMms;
	int &sanityCheck = ctx->sanityCheck;
	bool &noMaqRound = ctx->noMaqRound;
	int &qualThresh = ctx->qualThresh;
	bool &quiet = ctx->quiet;

	Ebwt<String<Dna> >& ebwtFw = *(ctx->seededQualSearch_ebwtFw);
	Ebwt<String<Dna> >& ebwtBw = *(ctx->seededQualSearch_ebwtBw);
	PartialAlignmentManager * pamRc = NULL;
	PartialAlignmentManager * pamFw = NULL;
	if(seedMms > 0) {
		pamRc = new PartialAlignmentManager(64);
		pamFw = new PartialAlignmentManager(64);
	}
	vector<PartialAlignment> pals;
	const BitPairReference* refs = ctx->seededQualSearch_refs;
	// GreedyDFSRangeSource for finding exact hits for the forward-
	// oriented read
	GreedyDFSRangeSource btf1(
		&ebwtFw, params,
		refs,           // reference sequence (for colorspace)
		qualCutoff,            // qualThresh
		ctx->maxBtsBetter,          // max backtracks
		0,                     // reportPartials (don't)
		true,                  // reportExacts
		ctx->rangeMode,             // reportRanges
		NULL,                  // seedlings
		NULL,                  // mutations
		verbose,               // verbose
		&os,
		false);                // considerQuals
	GreedyDFSRangeSource bt1(
		&ebwtFw, params,
		refs,           // reference sequence (for colorspace)
		qualCutoff,            // qualThresh
		ctx->maxBtsBetter,          // max backtracks
		0,                     // reportPartials (don't)
		true,                  // reportExacts
		ctx->rangeMode,             // reportRanges
		NULL,                  // seedlings
		NULL,                  // mutations
		ctx->verbose,               // verbose
		&os,                   // reference sequences
		true,                  // considerQuals
		false, !ctx->noMaqRound);
	// GreedyDFSRangeSource to search for hits for cases 1F, 2F, 3F
	GreedyDFSRangeSource btf2(
		&ebwtBw, params,
		refs,           // reference sequence (for colorspace)
		qualCutoff,            // qualThresh
		ctx->maxBtsBetter,          // max backtracks
		0,                     // reportPartials (no)
		true,                  // reportExacts
		ctx->rangeMode,             // reportRanges
		NULL,                  // partial alignment manager
		NULL,                  // mutations
		ctx->verbose,               // verbose
		&os,                   // reference sequences
		true,                  // considerQuals
		false, !noMaqRound);
	// GreedyDFSRangeSource to search for partial alignments for case 4R
	GreedyDFSRangeSource btr2(
		&ebwtBw, params,
		refs,           // reference sequence (for colorspace)
		qualCutoff,            // qualThresh (none)
		ctx->maxBtsBetter,          // max backtracks
		seedMms,               // report partials (up to seedMms mms)
		true,                  // reportExacts
		ctx->rangeMode,             // reportRanges
		pamRc,                 // partial alignment manager
		NULL,                  // mutations
		verbose,               // verbose
		&os,                   // reference sequences
		true,                  // considerQuals
		false, !noMaqRound);
	// GreedyDFSRangeSource to search for seedlings for case 4F
	GreedyDFSRangeSource btf3(
		&ebwtFw, params,
		refs,           // reference sequence (for colorspace)
		qualCutoff,            // qualThresh (none)
		ctx->maxBtsBetter,          // max backtracks
		seedMms,               // reportPartials (do)
		true,                  // reportExacts
		ctx->rangeMode,             // reportRanges
		pamFw,                 // seedlings
		NULL,                  // mutations
		verbose,               // verbose
		&os,                   // reference sequences
		true,                  // considerQuals
		false, !noMaqRound);
	// GreedyDFSRangeSource to search for hits for case 4R by extending
	// the partial alignments found in Phase 2
	GreedyDFSRangeSource btr3(
		&ebwtFw, params,
		refs,           // reference sequence (for colorspace)
		qualCutoff, // qualThresh
		ctx->maxBtsBetter,          // max backtracks
		0,       // reportPartials (don't)
		true,    // reportExacts
		ctx->rangeMode,// reportRanges
		NULL,    // seedlings
		NULL,    // mutations
		verbose, // verbose
		&os,     // reference sequences
		true,    // considerQuals
		false, !noMaqRound);
	// The half-and-half GreedyDFSRangeSource
	GreedyDFSRangeSource btr23(
		&ebwtFw, params,
		refs,           // reference sequence (for colorspace)
		qualCutoff, // qualThresh
		ctx->maxBtsBetter,          // max backtracks
		0,       // reportPartials (don't)
		true,    // reportExacts
		ctx->rangeMode,// reportRanges
		NULL,    // seedlings
		NULL,    // mutations
		verbose, // verbose
		&os,
		true,    // considerQuals
		true,    // halfAndHalf
		!noMaqRound);
	// GreedyDFSRangeSource to search for hits for case 4F by extending
	// the partial alignments found in Phase 3
	GreedyDFSRangeSource btf4(
		&ebwtBw, params,
		refs,           // reference sequence (for colorspace)
		qualCutoff, // qualThresh
		ctx->maxBtsBetter,          // max backtracks
		0,       // reportPartials (don't)
		true,    // reportExacts
		ctx->rangeMode,// reportRanges
		NULL,    // seedlings
		NULL,    // mutations
		verbose, // verbose
		&os,     // reference sequences
		true,    // considerQuals
		false, !noMaqRound);
	// Half-and-half GreedyDFSRangeSource for forward read
	GreedyDFSRangeSource btf24(
		&ebwtBw, params,
		refs,           // reference sequence (for colorspace)
		qualCutoff, // qualThresh
		ctx->maxBtsBetter,          // max backtracks
		0,       // reportPartials (don't)
		true,    // reportExacts
		ctx->rangeMode,// reportRanges
		NULL,    // seedlings
		NULL,    // mutations
		verbose, // verbose
		&os,
		true,    // considerQuals
		true,    // halfAndHalf
		!noMaqRound);
	String<QueryMutation> muts;
	bool skipped = false;
	while(true) {
		if(BowtieContext::isCanceled()) {
			break;
		}
		FINISH_READ(patsrc);
		GET_READ(patsrc);
		size_t plen = length(patFw);
		uint32_t s = seedLen;
		uint32_t s3 = (s >> 1); /* length of 3' half of seed */
		uint32_t s5 = (s >> 1) + (s & 1); /* length of 5' half of seed */
		uint32_t qs = min<uint32_t>(plen, s);
		uint32_t qs3 = qs >> 1;
		uint32_t qs5 = (qs >> 1) + (qs & 1);
#define DONEMASK_SET(p)
#include "search_seeded_phase1.c"
#include "search_seeded_phase2.c"
#include "search_seeded_phase3.c"
#include "search_seeded_phase4.c"
#undef DONEMASK_SET
	}
	FINISH_READ(patsrc);
	if(seedMms > 0) {
		delete pamRc;
		delete pamFw;
	}
	WORKER_EXIT();
}

static void* seededQualSearchWorkerFullStateful(void *vp) {
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	int tid = *((int*)vp);
	PairedPatternSource&     _patsrc    = *(ctx->seededQualSearch_patsrc);
	HitSink&                 _sink      = *(ctx->seededQualSearch_sink);
	Ebwt<String<Dna> >&      ebwtFw     = *(ctx->seededQualSearch_ebwtFw);
	Ebwt<String<Dna> >&      ebwtBw     = *(ctx->seededQualSearch_ebwtBw);
	vector<String<Dna5> >&   os         = *(ctx->seededQualSearch_os);
	int                      qualCutoff = ctx->seededQualSearch_qualCutoff;
	BitPairReference*        refs       = ctx->seededQualSearch_refs;

	// Global initialization
	PatternSourcePerThreadFactory* patsrcFact = createPatsrcFactory(_patsrc, tid);
	HitSinkPerThreadFactory* sinkFact = createSinkFactory(_sink);
	ChunkPool *pool = new ChunkPool(ctx->chunkSz * 1024, ctx->chunkPoolMegabytes * 1024 * 1024, ctx->chunkVerbose);

	AlignerMetrics *metrics = NULL;
	if(ctx->stats) {
		metrics = new AlignerMetrics();
	}
	UnpairedSeedAlignerFactory alSEfact(
		ebwtFw,
		&ebwtBw,
		!ctx->nofw,
		!ctx->norc,
		ctx->seedMms,
		ctx->seedLen,
		qualCutoff,
		ctx->maxBts,
		_sink,
		*sinkFact,
		NULL, //&cacheFw,
		NULL, //&cacheBw,
		ctx->cacheLimit,
		pool,
		refs,
		os,
		!ctx->noMaqRound,
		!ctx->better,
		ctx->strandFix,
		ctx->rangeMode,
		ctx->verbose,
		ctx->quiet,
		ctx->seed,
		metrics);
	PairedSeedAlignerFactory alPEfact(
		ebwtFw,
		&ebwtBw,
		ctx->color,
		ctx->useV1,
		!ctx->nofw,
		!ctx->norc,
		ctx->seedMms,
		ctx->seedLen,
		qualCutoff,
		ctx->maxBts,
		_sink,
		*sinkFact,
		ctx->mate1fw,
		ctx->mate2fw,
		ctx->minInsert,
		ctx->maxInsert,
		ctx->dontReconcileMates,
		ctx->mhits,       // for symCeiling
		ctx->mixedThresh,
		ctx->mixedAttemptLim,
		NULL, //&cacheFw,
		NULL, //&cacheBw,
		ctx->cacheLimit,
		pool,
		refs,
		os,
		ctx->reportSe,
		!ctx->noMaqRound,
		!ctx->better,
		ctx->strandFix,
		ctx->rangeMode,
		ctx->verbose,
		ctx->quiet,
		ctx->seed);
	{
		MixedMultiAligner multi(
			ctx->prefetchWidth,
			ctx->qUpto,
			alSEfact,
			alPEfact,
			*patsrcFact);
		// Run that mother
		multi.run();
		// MultiAligner must be destroyed before patsrcFact
	}
	if(metrics != NULL) {
		metrics->printSummary();
		delete metrics;
	}

	delete patsrcFact;
	delete sinkFact;
	delete pool;
#ifdef BOWTIE_PTHREADS
	if(tid > 0) {
		exitThread(NULL);
	}
#endif
	return NULL;
}

/**
* Search for a good alignments for each read using criteria that
* correspond somewhat faithfully to Maq's.  Search is aided by a pair
* of Ebwt indexes, one for the original references, and one for the
* transpose of the references.  Neither index should be loaded upon
* entry to this function.
*
* Like Maq, we treat the first 24 base pairs of the read (those
* closest to the 5' end) differently from the remainder of the read.
* We call the first 24 base pairs the "seed."
*/
template<typename TStr>
static void seededQualCutoffSearchFull(
									   int seedLen,                    /// length of seed (not a maq option)
									   int qualCutoff,                 /// maximum sum of mismatch qualities
									   /// like maq map's -e option
									   /// default: 70
									   int seedMms,                    /// max # mismatches allowed in seed
									   /// (like maq map's -n option)
									   /// Can only be 1 or 2, default: 1
									   PairedPatternSource& _patsrc,   /// pattern source
									   HitSink& _sink,                 /// hit sink
									   Ebwt<TStr>& ebwtFw,             /// index of original text
									   Ebwt<TStr>& ebwtBw,             /// index of mirror text
									   vector<String<Dna5> >& os)      /// text strings, if available (empty otherwise)
{

	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	// Global intialization
	assert_leq(seedMms, 3);

	ctx->seededQualSearch_patsrc   = &_patsrc;
	ctx->seededQualSearch_sink     = &_sink;
	ctx->seededQualSearch_ebwtFw   = &ebwtFw;
	ctx->seededQualSearch_ebwtBw   = &ebwtBw;
	ctx->seededQualSearch_os       = &os;
	ctx->seededQualSearch_doneMask = NULL;
	ctx->seededQualSearch_hitMask  = NULL;
	ctx->seededQualSearch_pamFw    = NULL;
	ctx->seededQualSearch_pamRc    = NULL;
	ctx->seededQualSearch_qualCutoff = qualCutoff;

	// Create range caches, which are shared among all aligners
	BitPairReference *refs = NULL;
	bool pair = ctx->mates1.size() > 0 || ctx->mates12.size() > 0;
	if(ctx->color || (pair && ctx->mixedThresh < 0xffffffff)) {
		Timer _t(cerr, "Time loading reference: ", ctx->timing);
		refs = new BitPairReference(ctx->adjustedEbwtFileBase, ctx->color, ctx->sanityCheck, NULL, &os, false, true, ctx->useMm, ctx->useShmem, ctx->mmSweep, ctx->verbose, ctx->startVerbose);
		if(!refs->loaded()) BowtieException("Bit pair reference not loaded");
	}
	ctx->seededQualSearch_refs = refs;

#ifdef BOWTIE_PTHREADS
	AutoArray<pthread_t> threads(ctx->nthreads-1);
	AutoArray<int> tids(ctx->nthreads-1);
#endif

	SWITCH_TO_FW_INDEX();
	assert(!ebwtBw.isInMemory());
	{
		// Load the other half of the index into memory
		Timer _t(cerr, "Time loading mirror index: ", ctx->timing);
		ebwtBw.loadIntoMemory(ctx->color ? 1 : 0, -1, !ctx->noRefNames, ctx->startVerbose);
	}
	CHUD_START();
	{
		// Phase 1: Consider cases 1R and 2R
		Timer _t(cerr, "Seeded quality full-index search: ", ctx->timing);
#ifdef BOWTIE_PTHREADS
		for(int i = 0; i < ctx->nthreads-1; i++) {
			tids[i] = i+1;
			if(ctx->stateful) createThread(&threads[i],
				seededQualSearchWorkerFullStateful,
				(void*)&tids[i]);
			else         createThread(&threads[i],
				seededQualSearchWorkerFull,
				(void*)&tids[i]);
		}
#endif
		int tmp = 0;
		if(ctx->stateful) seededQualSearchWorkerFullStateful((void*)&tmp);
		else         seededQualSearchWorkerFull((void*)&tmp);
#ifdef BOWTIE_PTHREADS
		for(int i = 0; i < ctx->nthreads-1; i++) joinThread(threads[i]);
#endif
	}
	if(refs != NULL) {
		delete refs;
	}
	ebwtBw.evictFromMemory();
}

/**
* Return a new dynamically allocated PatternSource for the given
* format, using the given list of strings as the filenames to read
* from or as the sequences themselves (i.e. if -c was used).
*/
static PatternSource*
patsrcFromStrings(int format,
				  const vector<string>& reads,
				  const vector<string>* quals)
{
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	switch(format) {
		case FASTA:
			return new FastaPatternSource (ctx->seed, reads, quals, ctx->color,
				ctx->randomizeQuals,
				ctx->useSpinlock,
				ctx->patDumpfile, ctx->verbose,
				ctx->trim3, ctx->trim5,
				ctx->solexaQuals, ctx->phred64Quals,
				ctx->integerQuals,
				ctx->skipReads);
		case FASTA_CONT:
			return new FastaContinuousPatternSource (
				ctx->seed, reads, ctx->fastaContLen,
				ctx->fastaContFreq,
				ctx->useSpinlock,
				ctx->patDumpfile, ctx->verbose,
				ctx->skipReads);
		case RAW:
			return new RawPatternSource   (ctx->seed, reads, ctx->color,
				ctx->randomizeQuals,
				ctx->useSpinlock,
				ctx->patDumpfile, ctx->verbose,
				ctx->trim3, ctx->trim5,
				ctx->skipReads);
		case FASTQ:
			return new FastqPatternSource (ctx->seed, reads, ctx->color,
				ctx->randomizeQuals,
				ctx->useSpinlock,
				ctx->patDumpfile, ctx->verbose,
				ctx->trim3, ctx->trim5,
				ctx->solexaQuals, ctx->phred64Quals,
				ctx->integerQuals, ctx->fuzzy,
				ctx->skipReads);
		case TAB_MATE:
			return new TabbedPatternSource(ctx->seed, reads, ctx->color,
				ctx->randomizeQuals,
				ctx->useSpinlock,
				ctx->patDumpfile, ctx->verbose,
				ctx->trim3, ctx->trim5,
				ctx->skipReads);
		case CMDLINE:
			return new VectorPatternSource(ctx->seed, reads, ctx->color,
				ctx->randomizeQuals,
				ctx->useSpinlock,
				ctx->patDumpfile, ctx->verbose,
				ctx->trim3, ctx->trim5,
				ctx->skipReads);
		case INPUT_CHAIN:
			return new ChainPatternSource (ctx->seed, reads, ctx->useSpinlock, ctx->patDumpfile,
				ctx->verbose, ctx->skipReads);
		case RANDOM:
			return new RandomPatternSource(ctx->seed, 2000000, ctx->lenRandomReads,
				ctx->useSpinlock, ctx->patDumpfile,
				ctx->verbose);
		default: {
			cerr << "Internal error; bad patsrc format: " << format << endl;
			throw 1;
				 }
	}
}

#define PASS_DUMP_FILES ctx->dumpAlBase, ctx->dumpUnalBase, ctx->dumpMaxBase

static string argstr;

template<typename TStr>
static void driver(U2::BowtieReadsReader* reader,
				   U2::BowtieReadsWriter* writer,
                   const U2::GUrl& resultFileUrl,
				   const char * type,
				   const string& ebwtFileBase,
				   const string& query,
				   const vector<string>& queries,
				   const vector<string>& qualities,
				   const string& outfile)
{
	BowtieContext::Search* ctx = BowtieContext::getSearchContext();
	if(ctx->verbose || ctx->startVerbose)  {
		cerr << "Entered driver(): "; logTime(cerr, true);
	}
	// Vector of the reference sequences; used for sanity-checking
	vector<String<Dna5> > os;
	// Read reference sequences from the command-line or from a FASTA file

	// Adjust
	ctx->adjustedEbwtFileBase = adjustEbwtBase("", ebwtFileBase, ctx->verbose);

	vector<PatternSource*> patsrcs_a;
	vector<PatternSource*> patsrcs_b;
	vector<PatternSource*> patsrcs_ab;

	// If there were any first-mates specified, we will operate in
	// stateful mode
	bool paired = ctx->mates1.size() > 0 || ctx->mates12.size() > 0;
	if(paired) ctx->stateful = true;

	// Create list of pattern sources for paired reads appearing
	// interleaved in a single file
	if(ctx->verbose || ctx->startVerbose) {
		cerr << "Creating paired-end patsrcs: "; logTime(cerr, true);
	}
	for(size_t i = 0; i < ctx->mates12.size(); i++) {
		const vector<string>* qs = &(ctx->mates12);
		vector<string> tmp;
		if(ctx->fileParallel) {
			// Feed query files one to each PatternSource
			qs = &tmp;
			tmp.push_back(ctx->mates12[i]);
			assert_eq(1, tmp.size());
		}
		patsrcs_ab.push_back(patsrcFromStrings(ctx->format, *qs, NULL));
		if(!ctx->fileParallel) {
			break;
		}
	}

	// Create list of pattern sources for paired reads
	for(size_t i = 0; i < ctx->mates1.size(); i++) {
		const vector<string>* qs = &(ctx->mates1);
		const vector<string>* quals = &(ctx->qualities1);
		vector<string> tmpSeq;
		vector<string> tmpQual;
		if(ctx->fileParallel) {
			// Feed query files one to each PatternSource
			qs = &tmpSeq;
			tmpSeq.push_back(ctx->mates1[i]);
			quals = &tmpSeq;
			tmpQual.push_back(ctx->qualities1[i]);
			assert_eq(1, tmpSeq.size());
		}
		if(quals->empty()) quals = NULL;
		patsrcs_a.push_back(patsrcFromStrings(ctx->format, *qs, quals));
		if(!ctx->fileParallel) {
			break;
		}
	}

	// Create list of pattern sources for paired reads
	for(size_t i = 0; i < ctx->mates2.size(); i++) {
		const vector<string>* qs = &(ctx->mates2);
		const vector<string>* quals = &(ctx->qualities2);
		vector<string> tmpSeq;
		vector<string> tmpQual;
		if(ctx->fileParallel) {
			// Feed query files one to each PatternSource
			qs = &tmpSeq;
			tmpSeq.push_back(ctx->mates2[i]);
			quals = &tmpQual;
			tmpQual.push_back(ctx->qualities2[i]);
			assert_eq(1, tmpSeq.size());
		}
		if(quals->empty()) quals = NULL;
		patsrcs_b.push_back(patsrcFromStrings(ctx->format, *qs, quals));
		if(!ctx->fileParallel) {
			break;
		}
	}
	// All mates/mate files must be paired
	assert_eq(patsrcs_a.size(), patsrcs_b.size());

	// Create list of pattern sources for the unpaired reads
	if(ctx->verbose || ctx->startVerbose) {
		cerr << "Creating single-end patsrcs: "; logTime(cerr, true);
	}
	for(size_t i = 0; i < queries.size(); i++) {
		const vector<string>* qs = &queries;
		const vector<string>* quals = &qualities;
		PatternSource* patsrc = NULL;
		vector<string> tmpSeq;
		vector<string> tmpQual;
		if(ctx->fileParallel) {
			// Feed query files one to each PatternSource
			qs = &tmpSeq;
			tmpSeq.push_back(queries[i]);
			quals = &tmpQual;
			tmpQual.push_back(qualities[i]);
			assert_eq(1, tmpSeq.size());
		}
		if(quals->empty()) quals = NULL;
		patsrc = new DNASequencesPatternSource(ctx->seed, reader);
		assert(patsrc != NULL);
		patsrcs_a.push_back(patsrc);
		patsrcs_b.push_back(NULL);
		if(!ctx->fileParallel) {
			break;
		}
	}

	if(ctx->verbose || ctx->startVerbose) {
		cerr << "Creating PatternSource: "; logTime(cerr, true);
	}
	PairedPatternSource *patsrc = NULL;
	if(ctx->mates12.size() > 0) {
		patsrc = new PairedSoloPatternSource(patsrcs_ab, ctx->seed);
	} else {
		patsrc = new PairedDualPatternSource(patsrcs_a, patsrcs_b, ctx->seed);
	}

	// Open hit output file
	if(ctx->verbose || ctx->startVerbose) {
		cerr << "Opening hit output file: "; logTime(cerr, true);
	}
	OutFileBuf *fout;
	if(!outfile.empty()) {
		if(ctx->refOut) {
			fout = NULL;
			if(!ctx->quiet) {
				cerr << "Warning: ignoring alignment output file " << outfile << " because --refout was specified" << endl;
			}
		} else {
			fout = new OutFileBuf(outfile.c_str(), false);
		}
	} else {
		if(ctx->outType == OUTPUT_CHAIN && !ctx->refOut) {
			cerr << "Error: Must specify an output file when output mode is --chain" << endl;
			throw 1;
		}
		fout = new OutFileBuf();
	}
	ReferenceMap* rmap = NULL;
	if(ctx->refMapFile != NULL) {
		if(ctx->verbose || ctx->startVerbose) {
			cerr << "About to load in a reference map file with name "
				<< ctx->refMapFile << ": "; logTime(cerr, true);
		}
		rmap = new ReferenceMap(ctx->refMapFile, !ctx->noRefNames);
	}
	AnnotationMap* amap = NULL;
	if(ctx->annotMapFile != NULL) {
		if(ctx->verbose || ctx->startVerbose) {
			cerr << "About to load in an annotation map file with name "
				<< ctx->annotMapFile << ": "; logTime(cerr, true);
		}
		amap = new AnnotationMap(ctx->annotMapFile);
	}
	// Initialize Ebwt object and read in header
	if(ctx->verbose || ctx->startVerbose) {
		cerr << "About to initialize fw Ebwt: "; logTime(cerr, true);
	}
	Ebwt<TStr> ebwt(ctx->adjustedEbwtFileBase,
		ctx->color,  // index is colorspace
                -1,     // don't care about entireReverse
		true,     // index is for the forward direction
		/* overriding: */ ctx->offRate,
		/* overriding: */ ctx->isaRate,
		ctx->useMm,    // whether to use memory-mapped files
		ctx->useShmem, // whether to use shared memory
		ctx->mmSweep,  // sweep memory-mapped files
		!ctx->noRefNames, // load names?
		rmap,     // reference map, or NULL if none is needed
		ctx->verbose, // whether to be talkative
		ctx->startVerbose, // talkative during initialization
		false /*passMemExc*/,
		ctx->sanityCheck);
	Ebwt<TStr>* ebwtBw = NULL;
	// We need the mirror index if mismatches are allowed
	if(ctx->mismatches > 0 || ctx->maqLike) {
		if(ctx->verbose || ctx->startVerbose) {
			cerr << "About to initialize rev Ebwt: "; logTime(cerr, true);
		}
		ebwtBw = new Ebwt<TStr>(ctx->adjustedEbwtFileBase + ".rev",
			ctx->color,  // index is colorspace
			-1,     // don't care about entireReverse
			false, // index is for the reverse direction
			/* overriding: */ ctx->offRate,
			/* overriding: */ ctx->isaRate,
			ctx->useMm,    // whether to use memory-mapped files
			ctx->useShmem, // whether to use shared memory
			ctx->mmSweep,  // sweep memory-mapped files
			!ctx->noRefNames, // load names?
			rmap,     // reference map, or NULL if none is needed
			ctx->verbose,  // whether to be talkative
			ctx->startVerbose, // talkative during initialization
			false /*passMemExc*/,
			ctx->sanityCheck);
	}
	if(!os.empty()) {
		for(size_t i = 0; i < os.size(); i++) {
			size_t olen = seqan::length(os[i]);
			int longestStretch = 0;
			int curStretch = 0;
			for(size_t j = 0; j < olen; j++) {
				if((int)os[i][j] < 4) {
					curStretch++;
					if(curStretch > longestStretch) longestStretch = curStretch;
				} else {
					curStretch = 0;
				}
			}
                        if(longestStretch < (ctx->color ? 2 : 1)) {
				os.erase(os.begin() + i);
				i--;
			}
		}
	}
	if(ctx->sanityCheck && !os.empty()) {
		// Sanity check number of patterns and pattern lengths in Ebwt
		// against original strings
		assert_eq(os.size(), ebwt.nPat());
		for(size_t i = 0; i < os.size(); i++) {
			assert_eq(length(os[i]), ebwt.plen()[i] + (ctx->color ? 1 : 0));
		}
		ebwt.loadIntoMemory(ctx->color ? 1 : 0, -1, !ctx->noRefNames, ctx->startVerbose);
		ebwt.checkOrigs(os, ctx->color, false);
		ebwt.evictFromMemory();
	}
	{
		Timer _t(cerr, "Time searching: ", ctx->timing);
		if(ctx->verbose || ctx->startVerbose) {
			cerr << "Creating HitSink: "; logTime(cerr, true);
		}
		// Set up hit sink; if sanityCheck && !os.empty() is true,
		// then instruct the sink to "retain" hits in a vector in
		// memory so that we can easily sanity check them later on
		HitSink *sink;
		RecalTable *table = NULL;
		if(ctx->recal) {
			table = new RecalTable(ctx->recalMaxCycle, ctx->recalMaxQual, ctx->recalQualShift);
		}
		vector<string>* refnames = &ebwt.refnames();
		if(ctx->noRefNames) refnames = NULL;
		sink = new MAlignmentHitSink(writer,
			fout, PASS_DUMP_FILES, false, ctx->sampleMax,
			table, refnames);

		if(ctx->verbose || ctx->startVerbose) {
			cerr << "Dispatching to search driver: "; logTime(cerr, true);
		}
		if(ctx->maqLike) {
			seededQualCutoffSearchFull(ctx->seedLen,
				ctx->qualThresh,
				ctx->seedMms,
				*patsrc,
				*sink,
				ebwt,    // forward index
				*ebwtBw, // mirror index (not optional)
				os);     // references, if available
		}
		else if(ctx->mismatches > 0) {
			if(ctx->mismatches == 1) {
				assert(ebwtBw != NULL);
				mismatchSearchFull(*patsrc, *sink, ebwt, *ebwtBw, os);
			} else if(ctx->mismatches == 2 || ctx->mismatches == 3) {
				twoOrThreeMismatchSearchFull(*patsrc, *sink, ebwt, *ebwtBw, os, ctx->mismatches == 2);
			} else {
				cerr << "Error: " << ctx->mismatches << " is not a supported number of mismatches" << endl;
				throw 1;
			}
		} else {
			// Search without mismatches
			// Note that --fast doesn't make a difference here because
			// we're only loading half of the index anyway
			exactSearch(*patsrc, *sink, ebwt, os);
		}
		// Evict any loaded indexes from memory
		if(ebwt.isInMemory()) {
			ebwt.evictFromMemory();
		}
		if(ebwtBw != NULL) {
			delete ebwtBw;
		}
		if(!ctx->quiet) {
			sink->finish(ctx->hadoopOut); // end the hits section of the hit file
		}
		for(size_t i = 0; i < patsrcs_a.size(); i++) {
			assert(patsrcs_a[i] != NULL);
			delete patsrcs_a[i];
		}
		for(size_t i = 0; i < patsrcs_b.size(); i++) {
			if(patsrcs_b[i] != NULL) {
				delete patsrcs_b[i];
			}
		}
		for(size_t i = 0; i < patsrcs_ab.size(); i++) {
			if(patsrcs_ab[i] != NULL) {
				delete patsrcs_ab[i];
			}
		}
		static_cast<MAlignmentHitSink*>(sink)->commitResultMA();
		delete patsrc;
		delete sink;
		delete amap;
		delete rmap;
		if(fout != NULL) delete fout;
	}
}

void driverAdapter(U2::BowtieReadsReader* reader,
				   U2::BowtieReadsWriter* writer,
                   const U2::GUrl& resultUrl,
				   const char * type,
				   const string& ebwtFileBase,
				   const string& query,
				   const vector<string>& queries,
				   const vector<string>& qualities,
				   const string& outfile) 
{
	driver<String<Dna, Alloc<> > >(reader, writer, resultUrl, type, ebwtFileBase, query, queries, qualities, outfile);	
}
