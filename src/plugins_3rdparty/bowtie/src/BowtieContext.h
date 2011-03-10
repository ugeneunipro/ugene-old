#ifndef _BOWTIE_CONTEXT_H_
#define _BOWTIE_CONTEXT_H_

#include <QtCore/QSemaphore>

#include <U2Core/TLSTask.h>
#include <vector>
#include <string>
#include "alphabet.h"
#include "formats.h"
#include "bitset.h"
#include "hit.h"
#include "log.h"

#ifdef _CRTDBG_MAP_ALLOC //tested only on windows
#define _CRTDBG_MAPALLOC
#include <iostream>
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif


using namespace std;
using namespace seqan;

#define BOWTIE_CONTEXT_ID "bowtie"

class BowtieContext;

class BitPairReference;
class PartialAlignmentManager;
template <class T > class Ebwt;

class BowtieException: public std::exception {
public:
	BowtieException(std::string str) {msg = str;}
	~BowtieException() throw() {};
	virtual const char* what() const throw() { return msg.c_str(); }

private:
	std::string msg;
};

class BowtieContext : public U2::TLSContext {

public:

	struct Search {

		Search() : suppressOuts(64) {}
		void resetOptions();

		vector<string> mates1;  // mated reads (first mate)
		vector<string> mates2;  // mated reads (second mate)
		vector<string> mates12; // mated reads (1st/2nd interleaved in 1 file)
		string adjustedEbwtFileBase;
		bool verbose;      // be talkative
		bool startVerbose; // be talkative at startup
		bool quiet;        // print nothing but the alignments
		int sanityCheck;   // enable expensive sanity checks
		int format;        // default read format is FASTQ
		string origString; // reference text, or filename(s)
		int seed;          // srandom() seed
		int timing;        // whether to report basic timing data
		bool allHits;      // for multihits, report just one
		bool rangeMode;    // report BWT ranges instead of ref locs
		int showVersion;   // just print version and quit?
		int ipause;        // pause before maching?
		uint32_t qUpto;    // max # of queries to read
		int trim5;         // amount to trim from 5' end
		int trim3;         // amount to trim from 3' end
		int reportOpps;    // whether to report # of other mappings
		int offRate;       // keep default offRate
		int isaRate;       // keep default isaRate
		int mismatches;    // allow 0 mismatches by default
		char *patDumpfile; // filename to dump patterns to
		bool solexaQuals;  // quality strings are solexa quals, not phred, and subtract 64 (not 33)
		bool phred64Quals; // quality chars are phred, but must subtract 64 (not 33)
		bool integerQuals; // quality strings are space-separated strings of integers, not ASCII
		int maqLike;       // do maq-like searching
		int seedLen;       // seed length (changed in Maq 0.6.4 from 24)
		int seedMms;       // # mismatches allowed in seed (maq's -n)
		int qualThresh;    // max qual-weighted hamming dist (maq's -e)
		int maxBtsBetter;  // max # backtracks allowed in half-and-half mode
		int maxBts;        // max # backtracks allowed in half-and-half mode
		int nthreads;      // number of pthreads operating concurrently
		output_types outType;  // style of output
		bool randReadsNoSync;  // true -> generate reads from per-thread random source
		int numRandomReads;    // # random reads (see Random*PatternSource in pat.h)
		int lenRandomReads;    // len of random reads (see Random*PatternSource in pat.h)
		bool noRefNames;       // true -> print reference indexes; not names
		string dumpAlBase;     // basename of same-format files to dump aligned reads to
		string dumpUnalBase;   // basename of same-format files to dump unaligned reads to
		string dumpMaxBase;    // basename of same-format files to dump reads with more than -m valid alignments to
		uint32_t khits;  // number of hits per read; >1 is much slower
		uint32_t mhits;  // don't report any hits if there are > mhits
		bool better;     // true -> guarantee alignments from best possible stratum
		bool strata;     // true -> don't stop at stratum boundaries
		bool refOut;     // if true, alignments go to per-ref files
		int partitionSz; // output a partitioning key in first field
		bool noMaqRound; // true -> don't round quals to nearest 10 like maq
		bool useSpinlock;  // false -> don't use of spinlocks even if they're #defines
		bool fileParallel; // separate threads read separate input files in parallel
		bool useShmem;     // use shared memory to hold the index
		bool useMm;        // use memory-mapped files to hold the index
		bool mmSweep;      // sweep through memory-mapped files immediately after mapping
		bool stateful;     // use stateful aligners
		uint32_t prefetchWidth; // number of reads to process in parallel w/ --stateful
		uint32_t minInsert;     // minimum insert size (Maq = 0, SOAP = 400)
		uint32_t maxInsert;     // maximum insert size (Maq = 250, SOAP = 600)
		bool mate1fw;           // -1 mate aligns in fw orientation on fw strand
		bool mate2fw;           // -2 mate aligns in rc orientation on fw strand
		bool mateFwSet;         // true -> user set --ff/--fr/--rf
		uint32_t mixedThresh;   // threshold for when to switch to paired-end mixed mode (see aligner.h)
		uint32_t mixedAttemptLim; // number of attempts to make in "mixed mode" before giving up on orientation
		bool dontReconcileMates;  // suppress pairwise all-versus-all way of resolving mates
		uint32_t cacheLimit;      // ranges w/ size > limit will be cached
		uint32_t cacheSize;       // # words per range cache
		int offBase;              // offsets are 0-based by default, but configurable
		bool tryHard;             // set very high maxBts, mixedAttemptLim
		uint32_t skipReads;       // # reads/read pairs to skip
		bool nofw; // don't align fw orientation of read
		bool norc; // don't align rc orientation of read
		bool strandFix;  // attempt to fix strand bias
		bool randomizeQuals; // randomize quality values
		bool stats; // print performance stats
		int chunkPoolMegabytes;    // max MB to dedicate to best-first search frames per thread
		int chunkSz;    // size of single chunk disbursed by ChunkPool
		bool chunkVerbose; // have chunk allocator output status messages?
		bool recal;
		int recalMaxCycle;
		int recalMaxQual;
		int recalQualShift;
		bool useV1;
		bool reportSe;
		const char * refMapFile;  // file containing a map from index coordinates to another coordinate system
		const char * annotMapFile;  // file containing a map from reference coordinates to annotations
		size_t fastaContLen;
		size_t fastaContFreq;
		bool hadoopOut; // print Hadoop status and summary messages
		bool fuzzy;
		bool fullRef;
		bool samNoHead; // don't print any header lines in SAM output
		bool samNoSQ;   // don't print @SQ header lines
		bool color;     // true -> inputs are colorspace
		bool colorExEnds; // true -> nucleotides on either end of decoded cspace alignment should be excluded
		string rgs; // SAM outputs for @RG header line
		int snpPhred; // probability of SNP, for scoring colorspace alignments
		Bitset suppressOuts; // output fields to suppress
		bool sampleMax; // whether to report a random alignment when maxed-out via -m/-M
		int defaultMapq; // default mapping quality to print in SAM mode
		bool colorSeq; // true -> show colorspace alignments as colors, not decoded bases
		bool colorQual; // true -> show colorspace qualities as original quals, not decoded quals
		bool printCost; // true -> print stratum and cost
		bool showSeed;
		vector<string> qualities;
		vector<string> qualities1;
		vector<string> qualities2;
		MUTEX_T gLock;

		PairedPatternSource*   exactSearch_patsrc;
		HitSink*               exactSearch_sink;
		Ebwt<String<Dna> >*    exactSearch_ebwt;
		vector<String<Dna5> >* exactSearch_os;
		BitPairReference*      exactSearch_refs;

		PairedPatternSource*           mismatchSearch_patsrc;
		HitSink*                       mismatchSearch_sink;
		Ebwt<String<Dna> >*            mismatchSearch_ebwtFw;
		Ebwt<String<Dna> >*            mismatchSearch_ebwtBw;
		vector<String<Dna5> >*         mismatchSearch_os;
		SyncBitset*                    mismatchSearch_doneMask;
		SyncBitset*                    mismatchSearch_hitMask;
		BitPairReference*              mismatchSearch_refs;

		PairedPatternSource*     seededQualSearch_patsrc;
		HitSink*                 seededQualSearch_sink;
		Ebwt<String<Dna> >*      seededQualSearch_ebwtFw;
		Ebwt<String<Dna> >*      seededQualSearch_ebwtBw;
		vector<String<Dna5> >*   seededQualSearch_os;
		SyncBitset*              seededQualSearch_doneMask;
		SyncBitset*              seededQualSearch_hitMask;
		PartialAlignmentManager* seededQualSearch_pamFw;
		PartialAlignmentManager* seededQualSearch_pamRc;
		int                      seededQualSearch_qualCutoff;
		BitPairReference*        seededQualSearch_refs;

		PairedPatternSource*           twoOrThreeMismatchSearch_patsrc;
		HitSink*                       twoOrThreeMismatchSearch_sink;
		Ebwt<String<Dna> >*            twoOrThreeMismatchSearch_ebwtFw;
		Ebwt<String<Dna> >*            twoOrThreeMismatchSearch_ebwtBw;
		vector<String<Dna5> >*         twoOrThreeMismatchSearch_os;
		SyncBitset*                    twoOrThreeMismatchSearch_doneMask;
		SyncBitset*                    twoOrThreeMismatchSearch_hitMask;
		bool                           twoOrThreeMismatchSearch_two;
		BitPairReference*              twoOrThreeMismatchSearch_refs;
        
        string                  refName;
        int                     refLength;

		bool sortAlignment; //Sort result alignment by offset
		
	} search;

	// Build
	struct Build {
		void resetOptions();

		bool verbose;
		int sanityCheck;
		int format;
		uint32_t bmax;
		uint32_t bmaxMultSqrt;
		uint32_t bmaxDivN;
		int dcv;
		int noDc;
		int entireSA;
		int seed;
		int showVersion;
		bool doubleEbwt;
		//   Ebwt parameters
		int32_t lineRate;
		int32_t linesPerSide;
		int32_t offRate;
		int32_t ftabChars;
		int  bigEndian;
		bool nsToAs;
		bool autoMem;
		bool packed;
		bool writeRef;
		bool justRef;
		int reverseType;
		bool color;

		bool secondEbwtRound;

		/// Array of Colbourn and Ling calculated difference covers up to
		/// r = 16 (maxV = 5953)
		struct sampleEntry {
			uint32_t maxV;
			uint32_t numSamples;
			uint32_t samples[128];
		};

		sampleEntry clDCs[16];
		bool clDCs_calced; /// have clDCs been calculated?
	} build;

	struct BowtieWorkerArg {
		int id;
		void *(*start_routine) (void *);
		void *arg;
	};

	struct Worker {
		QSemaphore start;
		QSemaphore stop;
	};

	// 5 64-element buckets for bucket-sorting A, C, G, T, $
	uint32_t bkts[4][4 * 1024 * 1024];

	SyncLogger glog;

	U2::TaskStateInfo &ti;

    int* numHitsOverall;


	QList<BowtieWorkerArg> jobList;

	QList<Worker*> workerList;

	volatile bool hasError;

	void runWorker(int* id, void *(*start_routine) (void *), void *arg);

	void joinWorker(int id);

	BowtieContext(U2::TaskStateInfo& stateInfo, int nThreads);

	~BowtieContext();

	static BowtieContext* getContext();

	static BowtieContext::Search* getSearchContext();

	static BowtieContext::Build* getBuildContext();

	static bool isCanceled();

	static void verbose(const std::string& s);
};

#endif //_BOWTIE_CONTEXT_H_
