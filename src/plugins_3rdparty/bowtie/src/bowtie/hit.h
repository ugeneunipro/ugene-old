#ifndef HIT_H_
#define HIT_H_

#include "u64.h"
#include <vector>
#include <stdint.h>
#include <iostream>
#include <sstream>
#ifdef _MSC_VER
#include <strstream>
#endif
#include <fstream>
#include <string>
#include <stdexcept>
#include <seqan/sequence.h>
#include "alphabet.h"
#include "assert_helpers.h"
#include "spinlock.h"
#include "threading.h"
#include "bitset.h"
#include "tokenize.h"
#include "pat.h"
#include "formats.h"
#include "filebuf.h"
#include "edit.h"
#include "refmap.h"
#include "annot.h"

/**
 * Classes for dealing with reporting alignments.
 */

using namespace std;
using namespace seqan;

/// Constants for the various output modes
enum output_types {
	OUTPUT_FULL = 1,
	OUTPUT_CONCISE,
	OUTPUT_BINARY,
	OUTPUT_CHAIN,
	OUTPUT_SAM,
	OUTPUT_NONE
};

/// Names of the various output modes
static const std::string output_type_names[] = {
	"Invalid!",
	"Full",
	"Concise",
	"Binary",
	"None"
};

typedef pair<uint32_t,uint32_t> U32Pair;

/**
 * Encapsulates a hit, including a text-id/text-offset pair, a pattern
 * id, and a boolean indicating whether it matched as its forward or
 * reverse-complement version.
 */
class Hit {
public:
	Hit() : stratum(-1) { }

	/**
	 * Initialize this Ent to represent the given hit.
	 */
	void toHitSetEnt(HitSetEnt& hse, AnnotationMap *amap) const {
		hse.h = h;
		hse.fw = fw ? 1 : 0;
		hse.oms = oms;
		hse.stratum = stratum;
		hse.cost = cost;
		if(mms.count() == 0) return;
		for(int i = 0; i < (int)length(); i++) {
			if(mms.test(i)) {
				hse.expand();
				hse.back().type = EDIT_TYPE_MM;
				hse.back().pos = i;
				hse.back().chr = refcs[i];
			}
		}
	}

	/**
	 * Convert a list of Hit objects to a single HitSet object suitable
	 * for chaining.
	 */
	static void toHitSet(const std::vector<Hit>& hits, HitSet& hs,
	                     AnnotationMap *amap)
	{
		if(hits.empty()) return;
		// Initialize HitSet
		hs.name = hits.front().patName;
		hs.seq  = hits.front().patSeq;
		hs.qual = hits.front().quals;
		hs.color = hits.front().color;
		if(!hits.front().fw) {
			// Re-reverse
			reverseComplementInPlace(hs.seq, hs.color);
			reverseInPlace(hs.qual);
		}
		// Convert hits to entries
		hs.ents.resize(hits.size());
		for(size_t i = 0; i < hs.ents.size(); i++) {
			hits[i].toHitSetEnt(hs.ents[i], amap);
		}
	}

	/**
	 * Populate a vector of Hits with hits from the given HitSet.
	 */
	static void fromHitSet(std::vector<Hit>& hits, const HitSet& hs) {
		assert(hs.sorted());
		hits.resize(hs.size());
		for(size_t i = 0; i < hs.size(); i++) {
			hits[i].h = hs[i].h;
			hits[i].oms = hs[i].oms;
			hits[i].fw = hs[i].fw;
			hits[i].stratum = hs[i].stratum;
			hits[i].cost = hs[i].cost;
			hits[i].mate = 0; //hs[i].mate;
			hits[i].patName = hs.name;
			hits[i].patSeq = hs.seq;
			hits[i].quals = hs.qual;
			hits[i].color = hs.color;
			size_t len = seqan::length(hs.seq);
			if(!hs[i].fw) {
				reverseComplementInPlace(hits[i].patSeq, hs.color);
				reverseInPlace(hits[i].quals);
			}
			hits[i].refcs.resize(len);
			for(size_t j = 0; j < hs[i].size(); j++) {
				if(hs[i][j].type != EDIT_TYPE_MM) continue;
				hits[i].mms.set(hs[i][j].pos);
				hits[i].refcs[hs[i][j].pos] = hs[i][j].chr;
			}
		}
	}

	U32Pair             h;       /// reference index & offset
	U32Pair             mh;      /// reference index & offset for mate
	uint32_t            patId;   /// read index
	String<char>        patName; /// read name
	String<Dna5>        patSeq;  /// read sequence
	String<Dna5>        colSeq;  /// original color sequence, not decoded
	String<char>        quals;   /// read qualities
	String<char>        colQuals;/// original color qualities, not decoded
	FixedBitset<1024>   mms;     /// nucleotide mismatch mask
	FixedBitset<1024>   cmms;    /// color mismatch mask (if relevant)
	vector<char>        refcs;   /// reference characters for mms
	vector<char>        crefcs;  /// reference characters for cmms
	uint32_t            oms;     /// # of other possible mappings; 0 -> this is unique
	bool                fw;      /// orientation of read in alignment
	bool                mfw;     /// orientation of mate in alignment
	uint16_t            mlen;    /// length of mate
	int8_t              stratum; /// stratum of hit (= mismatches in seed)
	uint32_t            cost;    /// total cost, factoring in stratum and quality penalty
	uint8_t             mate;    /// matedness; 0 = not a mate
	                             ///            1 = upstream mate
	                             ///            2 = downstream mate
	bool                color;   /// read is in colorspace?
	uint32_t            seed;    /// pseudo-random seed for aligned read

	/**
	 * Return true if this Hit is internally consistent.  Otherwise,
	 * throw an assertion.
	 */
	bool repOk() const {
		assert_geq(cost, (uint32_t)(stratum << 14));
		return true;
	}

	size_t length() const { return seqan::length(patSeq); }

	Hit& operator = (const Hit &other) {
		this->h       = other.h;
		this->mh      = other.mh;
		this->patId   = other.patId;
		this->patName = other.patName;
		this->patSeq  = other.patSeq;
		this->colSeq  = other.colSeq;
		this->quals   = other.quals;
		this->colQuals= other.colQuals;
		this->mms     = other.mms;
		this->cmms    = other.cmms;
		this->refcs   = other.refcs;
		this->crefcs  = other.crefcs;
		this->oms     = other.oms;
		this->fw      = other.fw;
		this->mfw     = other.mfw;
		this->mlen    = other.mlen;
		this->stratum = other.stratum;
		this->cost    = other.cost;
		this->mate    = other.mate;
		this->color   = other.color;
		this->cmms    = other.cmms;
		this->seed    = other.seed;
		return *this;
	}
};

/**
 * Compare hits a and b; a < b if its cost is less than B's.  If
 * there's a tie, break on position and orientation.
 */
class HitCostCompare {
public:
	bool operator() (const Hit& a, const Hit& b) {
		if(a.cost < b.cost) return true;
		if(a.cost > b.cost) return false;
		if(a.h < b.h) return true;
		if(a.h > b.h) return false;
		if(a.fw < b.fw) return true;
		if(a.fw > b.fw) return false;
		return false;
	}
};

/// Sort by text-id then by text-offset
bool operator< (const Hit& a, const Hit& b);

/**
 * Table for holding recalibration counts, along the lines of the table
 * presented in the SOAPsnp paper in Genome Res.  Each element maps a
 * read allele (o), quality score (q), cycle (c), and reference allele
 * (H), to a count of how many times that combination is observed in a
 * reported alignment.
 *
 * RecalTable is not synchronized, so it's assumed that threads are
 * incrementing the counters from within critical sections.
 */
class RecalTable {
public:
	RecalTable(int maxCycle,
	           int maxQual,
	           int qualShift) : maxCycle_(maxCycle),
	                            maxQual_(maxQual),
	                            qualShift_(qualShift),
	                            shift1_(6 - qualShift_),
	                            shift2_(shift1_ + 2),
	                            shift3_(shift2_ + 2),
	                            ents_(NULL), len_(0)
	{
		if(maxCycle == 0) {
			cerr << "Warning: maximum cycle for recalibration table is 0" << endl;
		} else if(maxQual >> qualShift == 0) {
			cerr << "Warning: maximum quality value " << maxQual << ", when shifted, is 0" << endl;
		} else if(qualShift > 5) {
			cerr << "Warning: quality shift value " << qualShift << " exceeds ceiling of 5" << endl;
		} else {
			try {
				len_ = maxCycle_ * 4 /* subj alleles*/ * 4 /* ref alleles */ * 64 /* quals */;
				ents_ = new uint32_t[len_];
				if(ents_ == NULL) {
					throw std::bad_alloc();
				}
				memset(ents_, 0, len_ << 2);
			} catch(std::bad_alloc& e) {
				cerr << "Error allocating recalibration table with " << len_ << " entries" << endl;
				throw 1;
			}
		}
	}

	~RecalTable() {
		if(ents_ != NULL) delete[] ents_;
	}

	/**
	 * Factor a new alignment into the recalibration table.
	 */
	void commitHit(const Hit& h) {
		// Iterate through the pattern from 5' to 3', calculate the
		// shifted quality value, obtain the reference character, and
		// increment the appropriate counter
		assert(h.repOk());
		for(int i = 0; i < (int)h.length(); i++) {
			int ii = i;
			if(!h.fw) {
				ii = h.length() - ii - 1;
			}
			int qc = (int)h.patSeq[ii];
			int rc = qc;
			if(h.mms.test(i)) {
				rc = charToDna5[(int)h.refcs[i]];
				assert_neq(rc, qc);
			}
			int q = (int)h.quals[ii]-33;
			assert_lt(q, 64);
			q >>= qualShift_;
			ents_[calcIdx(i, qc, rc, q)]++;
		}
	}

	/**
	 * Print the contents of the recalibration table.
	 */
	void print (std::ostream& out) const {
		if(ents_ == NULL) return;
		const int lim = maxCycle_;
		for(int i = 0; i < lim; i++) {
			out << "t" << i << "\t";
			// Iterate over subject alleles
			for(int j = 0; j < 4; j++) {
				// Iterate over reference alleles
				for(int k = 0; k < 4; k++) {
					// Iterate over qualities
					int lim2 = maxQual_ >> qualShift_;
					for(int l = 0; l < lim2; l++) {
						out << ents_[calcIdx(i, j, k, l)] << '\t';
					}
				}
			}
			out << endl;
		}
	}

protected:

	/**
	 * Calculate index into the ents_ array given cycle, subject
	 * allele, reference allele, and (shifted) quality.
	 */
	int calcIdx(int cyc, int sa, int ra, int q) const {
		int ret = q | (ra << shift1_) | (sa << shift2_) | (cyc << shift3_);
		assert_lt(ret, len_);
		return ret;
	}

	const int maxCycle_;
	const int maxQual_;
	const int qualShift_;
	const int shift1_;
	const int shift2_;
	const int shift3_;
	uint32_t *ents_;
	int len_;
};

#define DECL_HIT_DUMPS \
	const std::string& dumpAl, \
	const std::string& dumpUnal, \
	const std::string& dumpMax

#define INIT_HIT_DUMPS \
	dumpAlBase_(dumpAl), \
	dumpUnalBase_(dumpUnal), \
	dumpMaxBase_(dumpMax)

#define DECL_HIT_DUMPS2 \
	DECL_HIT_DUMPS, \
	bool onePairFile, \
	bool sampleMax, \
	RecalTable *recalTable, \
	std::vector<std::string>* refnames

#define PASS_HIT_DUMPS \
	dumpAl, \
	dumpUnal, \
	dumpMax

#define PASS_HIT_DUMPS2 \
	PASS_HIT_DUMPS, \
	onePairFile, \
	sampleMax, \
	recalTable, \
	refnames

/**
 * Encapsulates an object that accepts hits, optionally retains them in
 * a vector, and does something else with them according to
 * descendent's implementation of pure virtual member reportHitImpl().
 */
class HitSink {
public:
	explicit HitSink(OutFileBuf* out,
			DECL_HIT_DUMPS,
			bool onePairFile,
			bool sampleMax,
			RecalTable *table,
			vector<string>* refnames = NULL) :
		_outs(),
		_deleteOuts(false),
		recalTable_(table),
		_refnames(refnames),
		_numWrappers(0),
		_locks(),
		INIT_HIT_DUMPS,
		onePairFile_(onePairFile),
		sampleMax_(sampleMax),
		first_(true),
		numAligned_(U64(0)),
		numUnaligned_(U64(0)),
		numMaxed_(U64(0)),
		numReported_(U64(0)),
		numReportedPaired_(U64(0)),
		quiet_(false),
		ssmode_(ios_base::out)
	{
		_outs.push_back(out);
		_locks.resize(1);
		MUTEX_INIT(_locks[0]);
		MUTEX_INIT(_mainlock);
		initDumps();
	}

	/**
	 * Open a number of output streams; usually one per reference
	 * sequence.  For now, we give then names refXXXXX.map where XXXXX
	 * is the 0-padded reference index.  Someday we may want to include
	 * the name of the reference sequence in the filename somehow.
	 */
	explicit HitSink(size_t numOuts,
			DECL_HIT_DUMPS,
			bool onePairFile,
			bool sampleMax,
			RecalTable *table,
			vector<string>* refnames = NULL) :
		_outs(),
		_deleteOuts(true),
		recalTable_(table),
		_refnames(refnames),
		_locks(),
		INIT_HIT_DUMPS,
		onePairFile_(onePairFile),
		sampleMax_(sampleMax),
		quiet_(false),
		ssmode_(ios_base::out)
	{
		// Open all files for writing and initialize all locks
		for(size_t i = 0; i < numOuts; i++) {
			_outs.push_back(NULL); // we open output streams lazily
			_locks.resize(i+1);
			MUTEX_INIT(_locks[i]);
		}
		MUTEX_INIT(_mainlock);
		initDumps();
	}

	/**
	 * Destroy HitSinkobject;
	 */
	virtual ~HitSink() {
		closeOuts();
		if(_deleteOuts) {
			// Delete all non-NULL output streams
			for(size_t i = 0; i < _outs.size(); i++) {
				if(_outs[i] != NULL) {
					delete _outs[i];
					_outs[i] = NULL;
				}
			}
		}
		destroyDumps();
	}

	/**
	 * Call this whenever this HitSink is wrapped by a new
	 * HitSinkPerThread.  This helps us keep track of whether the main
	 * lock or any of the per-stream locks will be contended.
	 */
	void addWrapper() {
		_numWrappers++;
	}

	/**
	 * Called by concrete subclasses to figure out which elements of
	 * the _outs/_locks array to use when outputting the alignment.
	 */
	size_t refIdxToStreamIdx(size_t refIdx) {
		if(refIdx >= _outs.size()) return 0;
		return refIdx;
	}

	/**
	 * Append a single hit to the given output stream.
	 */
	virtual void append(ostream& o, const Hit& h) = 0;

	/**
	 * Report a batch of hits; all in the given vector.
	 */
	virtual void reportHits(vector<Hit>& hs) {
		reportHits(hs, 0, hs.size());
	}

	/**
	 * Report a batch of hits from a vector, perhaps subsetting it.
	 */
	virtual void reportHits(vector<Hit>& hs, size_t start, size_t end) {
		assert_geq(end, start);
		if(end-start == 0) return;
		bool paired = hs[start].mate > 0;
		// Sort reads so that those against the same reference sequence
		// are consecutive.
		if(_outs.size() > 1 && end-start > 2) {
			sort(hs.begin() + start, hs.begin() + end);
		}
		char buf[4096];
		for(size_t i = start; i < end; i++) {
			const Hit& h = hs[i];
			assert(h.repOk());
			bool diff = false;
			if(i > start) {
				diff = (refIdxToStreamIdx(h.h.first) != refIdxToStreamIdx(hs[i-1].h.first));
				if(diff) unlock(hs[i-1].h.first);
			}
#ifndef _MSC_VER
			ostringstream ss(ssmode_);
			ss.rdbuf()->pubsetbuf(buf, 4096);
#else
			ostrstream ss(buf, 4096);
#endif
			append(ss, h);
			if(i == start || diff) {
				lock(h.h.first);
			}
			out(h.h.first).writeChars(buf, ss.tellp());
		}
		unlock(hs[end-1].h.first);
		mainlock();
		commitHits(hs);
		first_ = false;
		numAligned_++;
		if(paired) numReportedPaired_ += (end-start);
		else       numReported_ += (end-start);
		mainunlock();
	}

	void commitHit(const Hit& hit) {
		if(recalTable_ != NULL) {
			recalTable_->commitHit(hit);
		}
	}

	void commitHits(const std::vector<Hit>& hits) {
		if(recalTable_ != NULL) {
			const size_t sz = hits.size();
			for(size_t i = 0; i < sz; i++) {
				commitHit(hits[i]);
			}
		}
	}

	/**
	 * Called when all alignments are complete.  It is assumed that no
	 * synchronization is necessary.
	 */
	void finish(bool hadoopOut) {
		// Close output streams
		closeOuts();
		if(!quiet_) {
			// Print information about how many unpaired and/or paired
			// reads were aligned.
			uint64_t tot = numAligned_ + numUnaligned_ + numMaxed_;
			double alPct = 0.0, unalPct = 0.0, maxPct = 0.0;
			if(tot > 0) {
				alPct   = 100.0 * (double)numAligned_ / (double)tot;
				unalPct = 100.0 * (double)numUnaligned_ / (double)tot;
				maxPct  = 100.0 * (double)numMaxed_ / (double)tot;
			}
			cerr << "# reads processed: " << tot << endl;
			cerr << "# reads with at least one reported alignment: "
			     << numAligned_ << " (" << fixed << setprecision(2)
			     << alPct << "%)" << endl;
			cerr << "# reads that failed to align: "
			     << numUnaligned_ << " (" << fixed << setprecision(2)
			     << unalPct << "%)" << endl;
			if(numMaxed_ > 0) {
				if(sampleMax_) {
					cerr << "# reads with alignments sampled due to -M: "
						 << numMaxed_ << " (" << fixed << setprecision(2)
						 << maxPct << "%)" << endl;
				} else {
					cerr << "# reads with alignments suppressed due to -m: "
						 << numMaxed_ << " (" << fixed << setprecision(2)
						 << maxPct << "%)" << endl;
				}
			}
			if(first_) {
				assert_eq(U64(0), numReported_);
				cerr << "No alignments" << endl;
			}
			else if(numReportedPaired_ > 0 && numReported_ == 0) {
				cerr << "Reported " << (numReportedPaired_ >> 1)
					 << " paired-end alignments to " << _outs.size()
					 << " output stream(s)" << endl;
			}
			else if(numReported_ > 0 && numReportedPaired_ == 0) {
				cerr << "Reported " << numReported_
					 << " alignments to " << _outs.size()
					 << " output stream(s)" << endl;
			}
			else {
				assert_gt(numReported_, 0);
				assert_gt(numReportedPaired_, 0);
				cerr << "Reported " << (numReportedPaired_ >> 1)
					 << " paired-end alignments and " << numReported_
					 << " singleton alignments to " << _outs.size()
					 << " output stream(s)" << endl;
			}
			if(hadoopOut) {
				cerr << "reporter:counter:Bowtie,Reads with reported alignments," << numAligned_ << endl;
				cerr << "reporter:counter:Bowtie,Reads with no alignments," << numUnaligned_ << endl;
				cerr << "reporter:counter:Bowtie,Reads exceeding -m limit," << numMaxed_ << endl;
				cerr << "reporter:counter:Bowtie,Unpaired alignments reported," << numReported_ << endl;
				cerr << "reporter:counter:Bowtie,Paired alignments reported," << numReportedPaired_ << endl;
			}
		}
		// Print the recalibration table.
		if(recalTable_ != NULL) {
			recalTable_->print(cout);
		}
	}

	/// Returns the alignment output stream; if the stream needs to be
	/// created, create it
	OutFileBuf& out(size_t refIdx) {
		size_t strIdx = refIdxToStreamIdx(refIdx);
		if(_outs[strIdx] == NULL) {
			assert(_deleteOuts);
			ostringstream oss;
			oss << "ref";
			if     (strIdx < 10)    oss << "0000";
			else if(strIdx < 100)   oss << "000";
			else if(strIdx < 1000)  oss << "00";
			else if(strIdx < 10000) oss << "0";
			oss << strIdx << ".map";
			_outs[strIdx] = new OutFileBuf(oss.str().c_str(), ssmode_ == ios_base::binary);
		}
		assert(_outs[strIdx] != NULL);
		return *(_outs[strIdx]);
	}

	/**
	 * Lock the monolithic lock for this HitSink.  This is useful when,
	 * for example, outputting a read to an unaligned-read file.
	 */
	void mainlock() {
		MUTEX_LOCK(_mainlock);
	}

	/**
	 * Unlock the monolithic lock for this HitSink.  This is useful
	 * when, for example, outputting a read to an unaligned-read file.
	 */
	void mainunlock() {
		MUTEX_UNLOCK(_mainlock);
	}

	/**
	 * Return true iff this HitSink dumps aligned reads to an output
	 * stream (i.e., iff --alfa or --alfq are specified).
	 */
	bool dumpsAlignedReads() {
		return dumpAlignFlag_;
	}

	/**
	 * Return true iff this HitSink dumps unaligned reads to an output
	 * stream (i.e., iff --unfa or --unfq are specified).
	 */
	bool dumpsUnalignedReads() {
		return dumpUnalignFlag_;
	}

	/**
	 * Return true iff this HitSink dumps maxed-out reads to an output
	 * stream (i.e., iff --maxfa or --maxfq are specified).
	 */
	bool dumpsMaxedReads() {
		return dumpMaxedFlag_ || dumpUnalignFlag_;
	}

	/**
	 * Return true iff this HitSink dumps either unaligned or maxed-
	 * out reads to an output stream (i.e., iff --unfa, --maxfa,
	 * --unfq, or --maxfq are specified).
	 */
	bool dumpsReads() {
		return dumpAlignFlag_ || dumpUnalignFlag_ || dumpMaxedFlag_;
	}

	/**
	 * Dump an aligned read to all of the appropriate output streams.
	 * Be careful to synchronize correctly - there may be multiple
	 * simultaneous writers.
	 */
	void dumpAlign(PatternSourcePerThread& p) {
		if(!dumpAlignFlag_) return;
		if(!p.paired() || onePairFile_) {
			// Dump unpaired read to an aligned-read file of the same format
			if(!dumpAlBase_.empty()) {
				MUTEX_LOCK(dumpAlignLock_);
				if(dumpAl_ == NULL) {
					assert(dumpAlQv_ == NULL);
					dumpAl_ = openOf(dumpAlBase_, 0, "");
					assert(dumpAl_ != NULL);
					if(p.bufa().qualOrigBufLen > 0) {
						dumpAlQv_ = openOf(dumpAlBase_ + ".qual", 0, "");
						assert(dumpAlQv_ != NULL);
					}
				}
				dumpAl_->write(p.bufa().readOrigBuf, p.bufa().readOrigBufLen);
				if(dumpAlQv_ != NULL) {
					dumpAlQv_->write(p.bufa().qualOrigBuf, p.bufa().qualOrigBufLen);
				}
				MUTEX_UNLOCK(dumpAlignLock_);
			}
		} else {
			// Dump paired-end read to an aligned-read file (or pair of
			// files) of the same format
			if(!dumpAlBase_.empty()) {
				MUTEX_LOCK(dumpAlignLockPE_);
				if(dumpAl_1_ == NULL) {
					assert(dumpAlQv_1_ == NULL);
					assert(dumpAlQv_2_ == NULL);
					dumpAl_1_ = openOf(dumpAlBase_, 1, "");
					dumpAl_2_ = openOf(dumpAlBase_, 2, "");
					assert(dumpAl_1_ != NULL);
					assert(dumpAl_2_ != NULL);
					if(p.bufa().qualOrigBufLen > 0) {
						dumpAlQv_1_ = openOf(dumpAlBase_ + ".qual", 1, "");
						dumpAlQv_2_ = openOf(dumpAlBase_ + ".qual", 2, "");
						assert(dumpAlQv_1_ != NULL);
						assert(dumpAlQv_2_ != NULL);
					}
				}
				dumpAl_1_->write(p.bufa().readOrigBuf, p.bufa().readOrigBufLen);
				dumpAl_2_->write(p.bufb().readOrigBuf, p.bufb().readOrigBufLen);
				if(dumpAlQv_1_ != NULL) {
					dumpAlQv_1_->write(p.bufa().qualOrigBuf, p.bufa().qualOrigBufLen);
					dumpAlQv_2_->write(p.bufb().qualOrigBuf, p.bufb().qualOrigBufLen);
				}
				MUTEX_UNLOCK(dumpAlignLockPE_);
			}
		}
	}

	/**
	 * Dump an unaligned read to all of the appropriate output streams.
	 * Be careful to synchronize correctly - there may be multiple
	 * simultaneous writers.
	 */
	void dumpUnal(PatternSourcePerThread& p) {
		if(!dumpUnalignFlag_) return;
		if(!p.paired() || onePairFile_) {
			// Dump unpaired read to an unaligned-read file of the same format
			if(!dumpUnalBase_.empty()) {
				MUTEX_LOCK(dumpUnalLock_);
				if(dumpUnal_ == NULL) {
					assert(dumpUnalQv_ == NULL);
					dumpUnal_ = openOf(dumpUnalBase_, 0, "");
					assert(dumpUnal_ != NULL);
					if(p.bufa().qualOrigBufLen > 0) {
						dumpUnalQv_ = openOf(dumpUnalBase_ + ".qual", 0, "");
						assert(dumpUnalQv_ != NULL);
					}
				}
				dumpUnal_->write(p.bufa().readOrigBuf, p.bufa().readOrigBufLen);
				if(dumpUnalQv_ != NULL) {
					dumpUnalQv_->write(p.bufa().qualOrigBuf, p.bufa().qualOrigBufLen);
				}
				MUTEX_UNLOCK(dumpUnalLock_);
			}
		} else {
			// Dump paired-end read to an unaligned-read file (or pair
			// of files) of the same format
			if(!dumpUnalBase_.empty()) {
				MUTEX_LOCK(dumpUnalLockPE_);
				if(dumpUnal_1_ == NULL) {
					assert(dumpUnal_1_ == NULL);
					assert(dumpUnal_2_ == NULL);
					dumpUnal_1_ = openOf(dumpUnalBase_, 1, "");
					dumpUnal_2_ = openOf(dumpUnalBase_, 2, "");
					assert(dumpUnal_1_ != NULL);
					assert(dumpUnal_2_ != NULL);
					if(p.bufa().qualOrigBufLen > 0) {
						dumpUnalQv_1_ = openOf(dumpUnalBase_ + ".qual", 1, "");
						dumpUnalQv_2_ = openOf(dumpUnalBase_ + ".qual", 2, "");
					}
				}
				dumpUnal_1_->write(p.bufa().readOrigBuf, p.bufa().readOrigBufLen);
				dumpUnal_2_->write(p.bufb().readOrigBuf, p.bufb().readOrigBufLen);
				if(dumpUnalQv_1_ != NULL) {
					dumpUnalQv_1_->write(p.bufa().qualOrigBuf, p.bufa().qualOrigBufLen);
					dumpUnalQv_2_->write(p.bufb().qualOrigBuf, p.bufb().qualOrigBufLen);
				}
				MUTEX_UNLOCK(dumpUnalLockPE_);
			}
		}
	}

	/**
	 * Dump a maxed-out read to all of the appropriate output streams.
	 * Be careful to synchronize correctly - there may be multiple
	 * simultaneous writers.
	 */
	void dumpMaxed(PatternSourcePerThread& p) {
		if(!dumpMaxedFlag_) {
			if(dumpUnalignFlag_) dumpUnal(p);
			return;
		}
		if(!p.paired() || onePairFile_) {
			// Dump unpaired read to an maxed-out-read file of the same format
			if(!dumpMaxBase_.empty()) {
				MUTEX_LOCK(dumpMaxLock_);
				if(dumpMax_ == NULL) {
					dumpMax_ = openOf(dumpMaxBase_, 0, "");
					assert(dumpMax_ != NULL);
					if(p.bufa().qualOrigBufLen > 0) {
						dumpMaxQv_ = openOf(dumpMaxBase_ + ".qual", 0, "");
					}
				}
				dumpMax_->write(p.bufa().readOrigBuf, p.bufa().readOrigBufLen);
				if(dumpMaxQv_ != NULL) {
					dumpMaxQv_->write(p.bufa().qualOrigBuf, p.bufa().qualOrigBufLen);
				}
				MUTEX_UNLOCK(dumpMaxLock_);
			}
		} else {
			// Dump paired-end read to a maxed-out-read file (or pair
			// of files) of the same format
			if(!dumpMaxBase_.empty()) {
				MUTEX_LOCK(dumpMaxLockPE_);
				if(dumpMax_1_ == NULL) {
					assert(dumpMaxQv_1_ == NULL);
					assert(dumpMaxQv_2_ == NULL);
					dumpMax_1_ = openOf(dumpMaxBase_, 1, "");
					dumpMax_2_ = openOf(dumpMaxBase_, 2, "");
					assert(dumpMax_1_ != NULL);
					assert(dumpMax_2_ != NULL);
					if(p.bufa().qualOrigBufLen > 0) {
						dumpMaxQv_1_ = openOf(dumpMaxBase_ + ".qual", 1, "");
						dumpMaxQv_2_ = openOf(dumpMaxBase_ + ".qual", 2, "");
					}
				}
				dumpMax_1_->write(p.bufa().readOrigBuf, p.bufa().readOrigBufLen);
				dumpMax_2_->write(p.bufb().readOrigBuf, p.bufb().readOrigBufLen);
				if(dumpMaxQv_1_ != NULL) {
					dumpMaxQv_1_->write(p.bufa().qualOrigBuf, p.bufa().qualOrigBufLen);
					dumpMaxQv_2_->write(p.bufb().qualOrigBuf, p.bufb().qualOrigBufLen);
				}
				MUTEX_UNLOCK(dumpMaxLockPE_);
			}
		}
	}

	/**
	 * Report a maxed-out read.  Typically we do nothing, but we might
	 * want to print a placeholder when output is chained.
	 */
	virtual void reportMaxed(vector<Hit>& hs, PatternSourcePerThread& p) {
		mainlock();
		numMaxed_++;
		mainunlock();
	}

	/**
	 * Report an unaligned read.  Typically we do nothing, but we might
	 * want to print a placeholder when output is chained.
	 */
	virtual void reportUnaligned(PatternSourcePerThread& p) {
		mainlock();
		numUnaligned_++;
		mainunlock();
	}

protected:

	/// Implementation of hit-report
	virtual void reportHit(const Hit& h) {
		assert(h.repOk());
		mainlock();
		commitHit(h);
		first_ = false;
		if(h.mate > 0) numReportedPaired_++;
		else           numReported_++;
		numAligned_++;
		mainunlock();
	}

	/**
	 * Close (and flush) all OutFileBufs.
	 */
	void closeOuts() {
		// Flush and close all non-NULL output streams
		for(size_t i = 0; i < _outs.size(); i++) {
			if(_outs[i] != NULL && !_outs[i]->closed()) {
				_outs[i]->close();
			}
		}
	}

	/**
	 * Lock the output buffer for the output stream for reference with
	 * index 'refIdx'.  By default, hits for all references are
	 * directed to the same output stream, but if --refout is
	 * specified, each reference has its own reference stream.
	 */
	void lock(size_t refIdx) {
		size_t strIdx = refIdxToStreamIdx(refIdx);
		MUTEX_LOCK(_locks[strIdx]);
	}

	/**
	 * Lock the output buffer for the output stream for reference with
	 * index 'refIdx'.  By default, hits for all references are
	 * directed to the same output stream, but if --refout is
	 * specified, each reference has its own reference stream.
	 */
	void unlock(size_t refIdx) {
		size_t strIdx = refIdxToStreamIdx(refIdx);
		MUTEX_UNLOCK(_locks[strIdx]);
	}

	vector<OutFileBuf*> _outs;        /// the alignment output stream(s)
	bool                _deleteOuts;  /// Whether to delete elements of _outs upon exit
	RecalTable         *recalTable_;  /// recalibration table
	vector<string>*     _refnames;    /// map from reference indexes to names
	int                 _numWrappers; /// # threads owning a wrapper for this HitSink
	vector<MUTEX_T>     _locks;       /// pthreads mutexes for per-file critical sections
	MUTEX_T             _mainlock;    /// pthreads mutexes for fields of this object

	// Output filenames for dumping
	std::string dumpAlBase_;
	std::string dumpUnalBase_;
	std::string dumpMaxBase_;

	bool onePairFile_;
	bool sampleMax_;

	// Output streams for dumping sequences
	std::ofstream *dumpAl_;       // for single-ended reads
	std::ofstream *dumpAl_1_;     // for first mates
	std::ofstream *dumpAl_2_;     // for second mates
	std::ofstream *dumpUnal_;     // for single-ended reads
	std::ofstream *dumpUnal_1_;   // for first mates
	std::ofstream *dumpUnal_2_;   // for second mates
	std::ofstream *dumpMax_;      // for single-ended reads
	std::ofstream *dumpMax_1_;    // for first mates
	std::ofstream *dumpMax_2_;    // for second mates

	// Output streams for dumping qualities
	std::ofstream *dumpAlQv_;     // for single-ended reads
	std::ofstream *dumpAlQv_1_;   // for first mates
	std::ofstream *dumpAlQv_2_;   // for second mates
	std::ofstream *dumpUnalQv_;   // for single-ended reads
	std::ofstream *dumpUnalQv_1_; // for first mates
	std::ofstream *dumpUnalQv_2_; // for second mates
	std::ofstream *dumpMaxQv_;    // for single-ended reads
	std::ofstream *dumpMaxQv_1_;  // for first mates
	std::ofstream *dumpMaxQv_2_;  // for second mates

	/**
	 * Open an ofstream with given name; output error message and quit
	 * if it fails.
	 */
	std::ofstream* openOf(const std::string& name,
	                      int mateType,
	                      const std::string& suffix)
	{
		std::string s = name;
		size_t dotoff = name.find_last_of(".");
		if(mateType == 1) {
			if(dotoff == string::npos) {
				s += "_1"; s += suffix;
			} else {
				s = name.substr(0, dotoff) + "_1" + s.substr(dotoff);
			}
		} else if(mateType == 2) {
			if(dotoff == string::npos) {
				s += "_2"; s += suffix;
			} else {
				s = name.substr(0, dotoff) + "_2" + s.substr(dotoff);
			}
		} else if(mateType != 0) {
			cerr << "Bad mate type " << mateType << endl; throw 1;
		}
		std::ofstream* tmp = new ofstream(s.c_str(), ios::out);
		if(tmp->fail()) {
			if(mateType == 0) {
				cerr << "Could not open single-ended aligned/unaligned-read file for writing: " << name << endl;
			} else {
				cerr << "Could not open paired-end aligned/unaligned-read file for writing: " << name << endl;
			}
			throw 1;
		}
		return tmp;
	}

	/**
	 * Initialize all the locks for dumping.
	 */
	void initDumps() {
		dumpAl_       = dumpAl_1_     = dumpAl_2_     = NULL;
		dumpUnal_     = dumpUnal_1_   = dumpUnal_2_   = NULL;
		dumpMax_      = dumpMax_1_    = dumpMax_2_    = NULL;
		dumpAlQv_     = dumpAlQv_1_   = dumpAlQv_2_   = NULL;
		dumpUnalQv_   = dumpUnalQv_1_ = dumpUnalQv_2_ = NULL;
		dumpMaxQv_    = dumpMaxQv_1_  = dumpMaxQv_2_  = NULL;
		dumpAlignFlag_   = !dumpAlBase_.empty();
		dumpUnalignFlag_ = !dumpUnalBase_.empty();
		dumpMaxedFlag_   = !dumpMaxBase_.empty();
		MUTEX_INIT(dumpAlignLock_);
		MUTEX_INIT(dumpAlignLockPE_);
		MUTEX_INIT(dumpUnalLock_);
		MUTEX_INIT(dumpUnalLockPE_);
		MUTEX_INIT(dumpMaxLock_);
		MUTEX_INIT(dumpMaxLockPE_);
	}

	void destroyDumps() {
		if(dumpAl_       != NULL) { dumpAl_->close();       delete dumpAl_; }
		if(dumpAl_1_     != NULL) { dumpAl_1_->close();     delete dumpAl_1_; }
		if(dumpAl_2_     != NULL) { dumpAl_2_->close();     delete dumpAl_2_; }
		if(dumpUnal_     != NULL) { dumpUnal_->close();     delete dumpUnal_; }
		if(dumpUnal_1_   != NULL) { dumpUnal_1_->close();   delete dumpUnal_1_; }
		if(dumpUnal_2_   != NULL) { dumpUnal_2_->close();   delete dumpUnal_2_; }
		if(dumpMax_      != NULL) { dumpMax_->close();      delete dumpMax_; }
		if(dumpMax_1_    != NULL) { dumpMax_1_->close();    delete dumpMax_1_; }
		if(dumpMax_2_    != NULL) { dumpMax_2_->close();    delete dumpMax_2_; }
		if(dumpAlQv_     != NULL) { dumpAlQv_->close();     delete dumpAlQv_; }
		if(dumpAlQv_1_   != NULL) { dumpAlQv_1_->close();   delete dumpAlQv_1_; }
		if(dumpAlQv_2_   != NULL) { dumpAlQv_2_->close();   delete dumpAlQv_2_; }
		if(dumpUnalQv_   != NULL) { dumpUnalQv_->close();   delete dumpUnalQv_; }
		if(dumpUnalQv_1_ != NULL) { dumpUnalQv_1_->close(); delete dumpUnalQv_1_; }
		if(dumpUnalQv_2_ != NULL) { dumpUnalQv_2_->close(); delete dumpUnalQv_2_; }
		if(dumpMaxQv_    != NULL) { dumpMaxQv_->close();    delete dumpMaxQv_; }
		if(dumpMaxQv_1_  != NULL) { dumpMaxQv_1_->close();  delete dumpMaxQv_1_; }
		if(dumpMaxQv_2_  != NULL) { dumpMaxQv_2_->close();  delete dumpMaxQv_2_; }
	}

	// Locks for dumping
	MUTEX_T dumpAlignLock_;
	MUTEX_T dumpAlignLockPE_; // _1 and _2
	MUTEX_T dumpUnalLock_;
	MUTEX_T dumpUnalLockPE_; // _1 and _2
	MUTEX_T dumpMaxLock_;
	MUTEX_T dumpMaxLockPE_;   // _1 and _2

	// false -> no dumping
	bool dumpAlignFlag_;
	bool dumpUnalignFlag_;
	bool dumpMaxedFlag_;

	volatile bool     first_;       /// true -> first hit hasn't yet been reported
	volatile uint64_t numAligned_;  /// # reads with >= 1 alignment
	volatile uint64_t numUnaligned_;/// # reads with no alignments
	volatile uint64_t numMaxed_;    /// # reads with # alignments exceeding -m ceiling
	volatile uint64_t numReported_; /// # single-ended alignments reported
	volatile uint64_t numReportedPaired_; /// # paired-end alignments reported
	bool quiet_;  /// true -> don't print alignment stats at the end
	ios_base::openmode ssmode_;     /// output mode for stringstreams
};

/**
 * A per-thread wrapper for a HitSink.  Incorporates state that a
 * single search thread cares about.
 */
class HitSinkPerThread {
public:
	HitSinkPerThread(HitSink& sink, uint32_t max, uint32_t n) :
		_sink(sink),
		_bestRemainingStratum(0),
		_numValidHits(U64(0)),
		_hits(),
		_bufferedHits(),
		hitsForThisRead_(),
		_max(max),
		_n(n)
	{
		_sink.addWrapper();
		assert_gt(_n, 0);
	}

	virtual ~HitSinkPerThread() { }

	/// Return the vector of retained hits
	vector<Hit>& retainedHits()   { return _hits; }

	/// Finalize current read
	virtual uint32_t finishRead(PatternSourcePerThread& p, bool report, bool dump) {
		uint32_t ret = finishReadImpl();
		_bestRemainingStratum = 0;
		if(!report) {
			_bufferedHits.clear();
			return 0;
		}
		bool maxed = (ret > _max);
		bool unal = (ret == 0);
		if(dump && (unal || maxed)) {
			// Either no reportable hits were found or the number of
			// reportable hits exceeded the -m limit specified by the
			// user
			assert(ret == 0 || ret > _max);
			if(maxed) _sink.dumpMaxed(p);
			else      _sink.dumpUnal(p);
		}
		ret = 0;
		if(maxed) {
			// Report that the read maxed-out; useful for chaining output
			if(dump) _sink.reportMaxed(_bufferedHits, p);
			_bufferedHits.clear();
		} else if(unal) {
			// Report that the read failed to align; useful for chaining output
			if(dump) _sink.reportUnaligned(p);
		} else {
			// Flush buffered hits
			assert_gt(_bufferedHits.size(), 0);
			if(_bufferedHits.size() > _n) {
				_bufferedHits.resize(_n);
			}
			_sink.reportHits(_bufferedHits);
			_sink.dumpAlign(p);
			ret = _bufferedHits.size();
			_bufferedHits.clear();
		}
		assert_eq(0, _bufferedHits.size());
		return ret;
	}

	virtual uint32_t finishReadImpl() = 0;

	/**
	 * Implementation for hit reporting; update per-thread _hits and
	 * _numReportableHits variables and call the master HitSink to do the actual
	 * reporting
	 */
	virtual void bufferHit(const Hit& h, int stratum) {
#ifndef NDEBUG
		// Ensure all buffered hits have the same patid
		for(size_t i = 1; i < _bufferedHits.size(); i++) {
			assert_eq(_bufferedHits[0].patId, _bufferedHits[i].patId);
		}
#endif
		_bufferedHits.push_back(h);
	}

	/**
	 * Concrete subclasses override this to (possibly) report a hit and
	 * return true iff the caller should continue to report more hits.
	 */
	virtual bool reportHit(const Hit& h, int stratum) {
		assert(h.repOk());
		_numValidHits++;
		return true;
	}

	/// Return the number of valid hits so far (not necessarily
	/// reported).  It's up to the concrete subclasses
	uint64_t numValidHits()    { return _numValidHits; }

	/**
	 * Return true if there are no more reportable hits.
	 */
	bool finishedWithStratum(int stratum) {
		bool ret = finishedWithStratumImpl(stratum);
		_bestRemainingStratum = stratum+1;
		return ret;
	}

	/**
	 * Use the given set of hits as a starting point.  By default, we don't
	 */
	virtual bool setHits(HitSet& hs) {
		if(!hs.empty()) {
			cerr << "Error: default setHits() called with non-empty HitSet" << endl;
			throw 1;
		}
		return false;
	}

	/**
	 * Return true if there are no reportable hits with the given cost
	 * (or worse).
	 */
	virtual bool irrelevantCost(uint16_t cost) {
		return false;
	}

	/**
	 * Concrete subclasses override this to determine whether the
	 * search routine should keep searching after having finished
	 * reporting all alignments at the given stratum.
	 */
	virtual bool finishedWithStratumImpl(int stratum) = 0;

	/// The mhits maximum
	uint32_t overThresh() { return _max; }

	/// Whether this thread, for this read, knows that we have already
	/// exceeded the mhits maximum
	bool exceededOverThresh() { return hitsForThisRead_ > _max; }

	/// Return whether we span strata
	virtual bool spanStrata() = 0;

	/// Return whether we report only the best possible hits
	virtual bool best() = 0;

	/**
	 * Return true iff the underlying HitSink dumps unaligned or
	 * maxed-out reads.
	 */
	bool dumpsReads() {
		return _sink.dumpsReads();
	}

	/**
	 * Return true iff there are currently no buffered hits.
	 */
	bool empty() const {
		return _bufferedHits.empty();
	}

	/**
	 * Return the number of currently buffered hits.
	 */
	bool size() const {
		return _bufferedHits.size();
	}

	/**
	 * Return max # hits to report (*2 in paired-end mode because mates
	 * count separately)
	 */
	virtual uint32_t maxHits() {
		return _n;
	}

protected:
	HitSink&    _sink; /// Ultimate destination of reported hits
	/// Least # mismatches in alignments that will be reported in the
	/// future.  Updated by the search routine.
	int         _bestRemainingStratum;
	/// # hits reported to this HitSink so far (not all of which were
	/// necesssary reported to _sink)
	uint64_t    _numValidHits;
	vector<Hit> _hits; /// Repository for retained hits
	/// Buffered hits, to be reported and flushed at end of read-phase
	vector<Hit> _bufferedHits;

	// Following variables are declared in the parent but maintained in
	// the concrete subcalsses
	uint32_t hitsForThisRead_; /// # hits for this read so far
	uint32_t _max; /// don't report any hits if there were > _max
	uint32_t _n;   /// report at most _n hits
};

/**
 * Abstract parent factory for HitSinkPerThreads.
 */
class HitSinkPerThreadFactory {
public:
	virtual ~HitSinkPerThreadFactory() { }
	virtual HitSinkPerThread* create() const = 0;
	virtual HitSinkPerThread* createMult(uint32_t m) const = 0;

	/// Free memory associated with a per-thread hit sink
	virtual void destroy(HitSinkPerThread* sink) const {
		assert(sink != NULL);
		// Free the HitSinkPerThread
		delete sink;
	}
};

/**
 * Report first N good alignments encountered; trust search routine
 * to try alignments in something approximating a best-first order.
 * Best used in combination with a stringent alignment policy.
 */
class NGoodHitSinkPerThread : public HitSinkPerThread {

public:
	NGoodHitSinkPerThread(
			HitSink& sink,
			uint32_t n,
			uint32_t max) :
				HitSinkPerThread(sink, max, n)
	{ }

	virtual bool spanStrata() {
		return true; // we span strata
	}

	virtual bool best() {
		return false; // we settle for "good" hits
	}

	/// Finalize current read
	virtual uint32_t finishReadImpl() {
		uint32_t ret = hitsForThisRead_;
		hitsForThisRead_ = 0;
		return ret;
	}

	/**
	 * Report and then return true if we've already reported N good
	 * hits.  Ignore the stratum - it's not relevant for finding "good"
	 * hits.
	 */
	virtual bool reportHit(const Hit& h, int stratum) {
		HitSinkPerThread::reportHit(h, stratum);
		hitsForThisRead_++;
		if(hitsForThisRead_ > _max) {
			return true; // done - report nothing
		}
		//if(hitsForThisRead_ <= _n) {
			// Only report hit if we haven't
			bufferHit(h, stratum);
		//}
		if(hitsForThisRead_ == _n &&
		   (_max == 0xffffffff || _max < _n))
		{
			return true; // already reported N good hits and max isn't set; stop!
		}
		return false; // not at N or max yet; keep going
	}

	/**
	 * Always return true; search routine should only stop if it's
	 * already reported N hits.
	 */
	virtual bool finishedWithStratumImpl(int stratum) { return false; }
};

/**
 * Concrete factory for FirstNGoodHitSinkPerThreads.
 */
class NGoodHitSinkPerThreadFactory : public HitSinkPerThreadFactory {
public:
	NGoodHitSinkPerThreadFactory(
			HitSink& sink,
			uint32_t n,
			uint32_t max) :
			sink_(sink),
			n_(n),
			max_(max)
	{ }

	/**
	 * Allocate a new NGoodHitSinkPerThread object on the heap,
	 * using the parameters given in the constructor.
	 */
	virtual HitSinkPerThread* create() const {
		return new NGoodHitSinkPerThread(sink_, n_, max_);
	}
	virtual HitSinkPerThread* createMult(uint32_t m) const {
		uint32_t max = max_ * (max_ == 0xffffffff ? 1 : m);
		uint32_t n = n_ * (n_ == 0xffffffff ? 1 : m);
		return new NGoodHitSinkPerThread(sink_, n, max);
	}

private:
	HitSink& sink_;
	uint32_t n_;
	uint32_t max_;
};

/**
 * Report the first N best alignments encountered in a single
 * alignment stratum assuming that we're receiving the alignments in
 * best-first order.
 */
class NBestFirstStratHitSinkPerThread : public HitSinkPerThread {

public:
	NBestFirstStratHitSinkPerThread(
			HitSink& sink,
			uint32_t n,
			uint32_t max,
			uint32_t mult) :
				HitSinkPerThread(sink, max, n),
				bestStratum_(999), mult_(mult)
	{ }

	/**
	 * false -> we do not allow strata to be spanned
	 */
	virtual bool spanStrata() {
		return false; // we do not span strata
	}

	/**
	 * true -> we report best hits
	 */
	virtual bool best() {
		return true;
	}

	/**
	 * Report and then return false if we've already reported N.
	 */
	virtual bool reportHit(const Hit& h, int stratum) {
		HitSinkPerThread::reportHit(h, stratum);
		// This hit is within th best possible remaining stratum,
		// so it should definitely count
		hitsForThisRead_++;
		// It doesn't exceed the limit, so buffer it
		if(stratum < bestStratum_) {
			bestStratum_ = stratum;
		}
		if(hitsForThisRead_ > _max) {
			return true; // done - report nothing
		}
		//if(hitsForThisRead_ <= _n) {
			bufferHit(h, stratum);
		//}
		if(hitsForThisRead_ == _n &&
		   (_max == 0xffffffff || _max < _n))
		{
			return true; // already reported N good hits; stop!
		}
		return false; // not at N yet; keep going
	}

	/**
	 * Finalize current read by reporting any buffered hits from best
	 * to worst until they're all reported or until we've reported all
	 * N
	 */
	virtual uint32_t finishReadImpl() {
		uint32_t ret = hitsForThisRead_;
		hitsForThisRead_ = 0;
		bestStratum_ = 999;
		const size_t sz = _bufferedHits.size();
		for(size_t i = 0; i < sz; i++) {
			// Set 'oms' according to the number of other alignments
			// at this stratum
			_bufferedHits[i].oms = (sz / mult_) - 1;
		}
		return ret;
	}

	/**
	 * If we had any alignments at all and we're now moving on to a new
	 * stratum, then we're done.
	 */
	virtual bool finishedWithStratumImpl(int stratum) {
		return hitsForThisRead_ > 0;
	}

	/**
	 * If there have been any hits reported so far, classify any
	 * subsequent alignments with higher strata as irrelevant.
	 */
	virtual bool irrelevantCost(uint16_t cost) {
		if(hitsForThisRead_) {
			// irrelevant iff at worse stratum
			return ((int)cost >> 14) > bestStratum_;
		}
		return false;
	}

private:

	int bestStratum_; /// best stratum observed so far
	uint32_t mult_; /// number of batched-up alignments
};

/**
 * Concrete factory for NBestStratHitSinkPerThread.
 */
class NBestFirstStratHitSinkPerThreadFactory : public HitSinkPerThreadFactory {
public:
	NBestFirstStratHitSinkPerThreadFactory(
			HitSink& sink,
			uint32_t n,
			uint32_t max) :
			sink_(sink),
			n_(n),
			max_(max)
	{ }

	/**
	 * Allocate a new NGoodHitSinkPerThread object on the heap,
	 * using the parameters given in the constructor.
	 */
	virtual HitSinkPerThread* create() const {
		return new NBestFirstStratHitSinkPerThread(sink_, n_, max_, 1);
	}
	virtual HitSinkPerThread* createMult(uint32_t m) const {
		uint32_t max = max_ * (max_ == 0xffffffff ? 1 : m);
		uint32_t n = n_ * (n_ == 0xffffffff ? 1 : m);
		return new NBestFirstStratHitSinkPerThread(sink_, n, max, m);
	}

private:
	HitSink& sink_;
	uint32_t n_;
	uint32_t max_;
};

/**
 *
 */
class ChainingHitSinkPerThread : public HitSinkPerThread {
public:

	ChainingHitSinkPerThread(HitSink& sink,
	                         uint32_t n,
	                         uint32_t max,
	                         bool strata,
	                         uint32_t mult) :
	HitSinkPerThread(sink, max, n),
	mult_(mult), strata_(strata), cutoff_(0xffff)
	{
		hs_ = NULL;
		hsISz_ = 0;
	}

	/**
	 * Return true iff we're allowed to span strata.
	 */
	virtual bool spanStrata() { return !strata_; }

	/**
	 * true -> we report best hits
	 */
	virtual bool best() { return true; }

	/**
	 * Report and then return false if we've already reported N.
	 */
	virtual bool reportHit(const Hit& h, int stratum) {
		HitSinkPerThread::reportHit(h, stratum);
		assert(hs_->sorted());
		assert(hs_ != NULL);
		assert_eq(h.stratum, stratum);
		assert_eq(1, mult_);
		assert(consistentStrata());
		assert(!irrelevantCost(h.cost));

		if(!hs_->empty() && strata_ && stratum < hs_->front().stratum) {
			hs_->clear();
			_bufferedHits.clear();
			hitsForThisRead_ = 0;
		}
		assert(consistentStrata());

		size_t replPos = 0;
		if(!hs_->empty() && hs_->tryReplacing(h.h, h.fw, h.cost, replPos)) {
			if(replPos != 0xffffffff) {
				// Replaced an existing hit
				assert_lt(replPos, _bufferedHits.size());
				_bufferedHits[replPos] = h;
				hs_->sort();
			}
			// Size didn't change, so no need to check against _max and _n
			assert(hs_->sorted());
			assert(consistentStrata());
		} else {
			// Added a new hit
			hs_->expand();
			hs_->back().h = h.h;
			hs_->back().fw = h.fw;
			hs_->back().stratum = h.stratum;
			hs_->back().cost = h.cost;
			hitsForThisRead_++;
			if(hs_->size() > _max) {
				assert_eq(hs_->size(), _bufferedHits.size());
				return true; // done - report nothing
			}
			_bufferedHits.push_back(h);
			if(hsISz_ == 0 &&
			   hs_->size() == _n &&
			   (_max == 0xffffffff || _max < _n))
			{
				assert_eq(hs_->size(), _bufferedHits.size());
				return true; // already reported N good hits; stop!
			}
			hs_->sort();
			assert(consistentStrata());
		}
		assert_eq(hs_->size(), _bufferedHits.size());
		updateCutoff();
		return false; // not at N yet; keep going
	}

	/**
	 * Finalize current read by reporting any buffered hits from best
	 * to worst until they're all reported or until we've reported all
	 * N
	 */
	virtual uint32_t finishReadImpl() {
		assert_eq(1, mult_);
		assert(hs_ != NULL);
		assert(consistentStrata());
		uint32_t ret = hitsForThisRead_;
		hitsForThisRead_ = 0;
		if(!hs_->empty() && hs_->size() < _n) {
			const size_t sz = _bufferedHits.size();
			for(size_t i = 0; i < sz; i++) {
				// Set 'oms' according to the number of other alignments
				// at this stratum
				_bufferedHits[i].oms = (sz / mult_) - 1;
			}
		}
		std::sort(_bufferedHits.begin(), _bufferedHits.end(), HitCostCompare());
		if(hs_->size() > _n) {
			_bufferedHits.resize(_n);
		}
		assert(consistentStrata());
		// Make sure that a chained, maxed-out read gets treated as
		// maxed-out and not as unaligned
		if(hs_->empty() && hs_->maxedStratum != -1) {
			assert(_bufferedHits.empty());
			// Boy, this is stupid.  Need to switch to just using HitSet
			// internally all the time.
			_bufferedHits.resize(_max+1);
			for(size_t i = 0; i < _max+1; i++) {
				_bufferedHits[i].stratum = hs_->maxedStratum;
			}
			ret = _max+1;
		} else if(!hs_->empty() && hs_->maxedStratum != -1) {
			assert_lt(hs_->front().stratum, hs_->maxedStratum);
		}
		return ret;
	}

	/**
	 * Set the initial set of Hits.
	 */
	virtual bool setHits(HitSet& hs) {
		hs_ = &hs;
		assert(hs_ != NULL);
		hsISz_ = hs.size();
		cutoff_ = 0xffff;
		hitsForThisRead_ = hs.size();
		assert(_bufferedHits.empty());
		assert_geq(hs.maxedStratum, -1);
		assert_lt(hs.maxedStratum, 4);
		if(!hs.empty()) {
			assert_eq(-1, hs.maxedStratum);
			hs.sort();
			Hit::fromHitSet(_bufferedHits, hs);
			assert(!_bufferedHits.empty());
			assert_leq(_bufferedHits.size(), _max);
			assert(consistentStrata());
		} else if(hs.maxedStratum != -1) {
			if(hs.maxedStratum == 0) {
				cutoff_ = 0;
				// Already done
				return true;
			}
			cutoff_ = (hs.maxedStratum << 14);
		}
		assert_eq(hs_->size(), _bufferedHits.size());
		updateCutoff();
		return false;
	}

	/**
	 * If we had any alignments at all and we're now moving on to a new
	 * stratum, then we're done.
	 */
	virtual bool finishedWithStratumImpl(int stratum) {
		assert(false);
		return false;
	}

	/**
	 * If there have been any hits reported so far, classify any
	 * subsequent alignments with higher strata as irrelevant.
	 */
	virtual bool irrelevantCost(uint16_t cost) {
		if(cutoff_ == 0) return false;
		return cost > cutoff_;
	}

protected:

#ifndef NDEBUG
	/**
	 * Sanity check that, if we're in strata_ mode, all 'stratum's
	 * should be the same among hits.
	 */
	bool consistentStrata() {
		if(hs_->empty() || !strata_) return true;
		int stratum = hs_->front().stratum;
		for(size_t i = 1; i < hs_->size(); i++) {
			assert_eq(stratum, (*hs_)[i].stratum);
		}
		return true;
	}
#endif

	/**
	 * Update the lowest relevant cost.
	 */
	void updateCutoff() {
		ASSERT_ONLY(uint16_t origCutoff = cutoff_);
		assert(hs_->sorted());
		assert(hs_->empty() || hs_->back().cost >= hs_->front().cost);
		bool atCapacity = (hs_->size() >= _n);
		if(atCapacity && (_max == 0xffffffff || _max < _n)) {
			cutoff_ = min(hs_->back().cost, cutoff_);
		}
		if(strata_ && !hs_->empty()) {
			uint16_t sc = hs_->back().cost;
			sc = ((sc >> 14) + 1) << 14;
			cutoff_ = min(cutoff_, sc);
			assert_leq(cutoff_, origCutoff);
		}
	}

	HitSet *hs_;
	size_t hsISz_;
	uint32_t mult_;
	bool strata_; /// true -> reporting is stratified
	uint16_t cutoff_; /// the smallest irrelevant cost
};

/**
 * Concrete factory for ChainingHitSinkPerThread.
 */
class ChainingHitSinkPerThreadFactory : public HitSinkPerThreadFactory {
public:
	ChainingHitSinkPerThreadFactory(
			HitSink& sink,
			uint32_t n,
			uint32_t max,
			bool strata) :
			sink_(sink),
			n_(n),
			max_(max),
			strata_(strata)
	{ }

	/**
	 * Allocate a new NGoodHitSinkPerThread object on the heap,
	 * using the parameters given in the constructor.
	 */
	virtual HitSinkPerThread* create() const {
		return new ChainingHitSinkPerThread(sink_, n_, max_, strata_, 1);
	}
	virtual HitSinkPerThread* createMult(uint32_t m) const {
		uint32_t max = max_ * (max_ == 0xffffffff ? 1 : m);
		uint32_t n = n_ * (n_ == 0xffffffff ? 1 : m);
		return new ChainingHitSinkPerThread(sink_, n, max, strata_, m);
	}

private:
	HitSink& sink_;
	uint32_t n_;
	uint32_t max_;
	bool strata_;
};

/**
 * Report all valid alignments.
 */
class AllHitSinkPerThread : public HitSinkPerThread {

public:
	AllHitSinkPerThread(
			HitSink& sink,
	        uint32_t max) :
		    HitSinkPerThread(sink, max, 0xffffffff) { }

	virtual bool spanStrata() {
		return true; // we span strata
	}

	virtual bool best() {
		return true; // we report "best" hits
	}

	/**
	 * Report and always return true; we're finiding all hits so that
	 * search routine should always continue.
	 */
	virtual bool reportHit(const Hit& h, int stratum) {
		HitSinkPerThread::reportHit(h, stratum);
		hitsForThisRead_++;
		if(hitsForThisRead_ > _max) {
			return true; // done - report nothing
		}
		bufferHit(h, stratum);
		return false; // reporting all; always keep going
	}

	/**
	 * Finalize; do nothing because we haven't buffered anything
	 */
	virtual uint32_t finishReadImpl() {
		uint32_t ret = hitsForThisRead_;
		hitsForThisRead_ = 0;
		return ret;
	}

	/**
	 * Always return false; search routine should not stop.
	 */
	virtual bool finishedWithStratumImpl(int stratum) { return false; }
};

/**
 * Concrete factory for AllHitSinkPerThread.
 */
class AllHitSinkPerThreadFactory : public HitSinkPerThreadFactory {
public:
	AllHitSinkPerThreadFactory(
			HitSink& sink,
			uint32_t max) :
			sink_(sink),
			max_(max)
	{ }

	/**
	 * Allocate a new NGoodHitSinkPerThread object on the heap,
	 * using the parameters given in the constructor.
	 */
	virtual HitSinkPerThread* create() const {
		return new AllHitSinkPerThread(sink_, max_);
	}
	virtual HitSinkPerThread* createMult(uint32_t m) const {
		uint32_t max = max_ * (max_ == 0xffffffff ? 1 : m);
		return new AllHitSinkPerThread(sink_, max);
	}

private:
	HitSink& sink_;
	uint32_t max_;
};

/**
 * Sink that prints lines like this:
 * (pat-id)[-|+]:<hit1-text-id,hit2-text-offset>,<hit2-text-id...
 *
 * Activated with --concise
 */
class ConciseHitSink : public HitSink {
public:
	/**
	 * Construct a single-stream ConciseHitSink (default)
	 */
	ConciseHitSink(OutFileBuf* out,
			       int offBase,
	               DECL_HIT_DUMPS2,
	               bool reportOpps = false) :
		HitSink(out, PASS_HIT_DUMPS2),
		_reportOpps(reportOpps),
		offBase_(offBase) { }

	/**
	 * Construct a multi-stream ConciseHitSink with one stream per
	 * reference string (see --refout)
	 */
	ConciseHitSink(size_t numOuts,
	               int offBase,
	               DECL_HIT_DUMPS2,
	               bool reportOpps = false) :
		HitSink(numOuts, PASS_HIT_DUMPS2),
		_reportOpps(reportOpps),
		offBase_(offBase) { }

	/**
	 * Append a verbose, readable hit to the given output stream.
	 */
	static void append(ostream& ss, const Hit& h, int offBase, bool reportOpps) {
		ss << h.patId;
		if(h.mate > 0) {
			assert(h.mate == 1 || h.mate == 2);
			ss << '/' << (int)h.mate;
		}
		ss << (h.fw? "+" : "-") << ":";
		// .first is text id, .second is offset
		ss << "<" << h.h.first << "," << (h.h.second + offBase) << "," << h.mms.count();
		if(reportOpps) ss << "," << h.oms;
		ss << ">" << endl;
	}

	/**
	 * Append a verbose, readable hit to the given output stream.
	 */
	void append(ostream& ss, const Hit& h) {
		ConciseHitSink::append(ss, h, this->offBase_, this->_reportOpps);
	}

protected:

	/**
	 * Report a concise alignment to the appropriate output stream.
	 */
	virtual void reportHit(const Hit& h) {
		HitSink::reportHit(h);
		ostringstream ss;
		append(ss, h);
		lock(h.h.first);
		out(h.h.first).writeString(ss.str());
		unlock(h.h.first);
	}

private:
	bool _reportOpps;
	int  offBase_;     /// Add this to reference offsets before outputting.
	                   /// (An easy way to make things 1-based instead of
	                   /// 0-based)
};

/**
 * Print the given string.  If ws = true, print only up to and not
 * including the first space or tab.  Useful for printing reference
 * names.
 */
inline void printUptoWs(std::ostream& os, const std::string& str, bool ws) {
	if(!ws) {
		os << str;
	} else {
		size_t pos = str.find_first_of(" \t");
		if(pos != string::npos) {
			os << str.substr(0, pos);
		} else {
			os << str;
		}
	}
}

/**
 * Sink that prints lines like this:
 * pat-name \t [-|+] \t ref-name \t ref-off \t pat \t qual \t #-alt-hits \t mm-list
 */
class VerboseHitSink : public HitSink {
public:
	/**
	 * Construct a single-stream VerboseHitSink (default)
	 */
	VerboseHitSink(OutFileBuf* out,
	               int offBase,
	               bool colorSeq,
	               bool colorQual,
	               bool printCost,
	               const Bitset& suppressOuts,
	               ReferenceMap *rmap,
	               AnnotationMap *amap,
	               bool fullRef,
	               DECL_HIT_DUMPS2,
				   int partition = 0) :
	HitSink(out, PASS_HIT_DUMPS2),
	partition_(partition),
	offBase_(offBase),
	colorSeq_(colorSeq),
	colorQual_(colorQual),
	cost_(printCost),
	suppress_(suppressOuts),
	fullRef_(fullRef),
	rmap_(rmap), amap_(amap)
	{ }

	/**
	 * Construct a multi-stream VerboseHitSink with one stream per
	 * reference string (see --refout)
	 */
	VerboseHitSink(size_t numOuts,
	               int offBase,
	               bool colorSeq,
	               bool colorQual,
	               bool printCost,
	               const Bitset& suppressOuts,
	               ReferenceMap *rmap,
	               AnnotationMap *amap,
	               bool fullRef,
	               DECL_HIT_DUMPS2,
				   int partition = 0) :
	HitSink(numOuts, PASS_HIT_DUMPS2),
	partition_(partition),
	offBase_(offBase),
	colorSeq_(colorSeq),
	colorQual_(colorQual),
	cost_(printCost),
	suppress_(64),
	fullRef_(fullRef),
	rmap_(rmap),
	amap_(amap)
	{ }

	// In hit.cpp
	static void append(ostream& ss,
	                   const Hit& h,
	                   const vector<string>* refnames,
	                   ReferenceMap *rmap,
	                   AnnotationMap *amap,
	                   bool fullRef,
	                   int partition,
	                   int offBase,
	                   bool colorSeq,
	                   bool colorQual,
	                   bool cost,
	                   const Bitset& suppress);

	/**
	 * Append a verbose, readable hit to the output stream
	 * corresponding to the hit.
	 */
	virtual void append(ostream& ss, const Hit& h) {
		VerboseHitSink::append(ss, h, _refnames, rmap_, amap_,
		                       fullRef_, partition_, offBase_,
		                       colorSeq_, colorQual_, cost_,
		                       suppress_);
	}

	/**
	 * See hit.cpp
	 */
	virtual void reportMaxed(vector<Hit>& hs, PatternSourcePerThread& p);

protected:

	/**
	 * Report a verbose, human-readable alignment to the appropriate
	 * output stream.
	 */
	virtual void reportHit(const Hit& h) {
		reportHit(h, true);
	}

	/**
	 * Report a verbose, human-readable alignment to the appropriate
	 * output stream.
	 */
	virtual void reportHit(const Hit& h, bool count) {
		if(count) HitSink::reportHit(h);
		ostringstream ss;
		append(ss, h);
		// Make sure to grab lock before writing to output stream
		lock(h.h.first);
		out(h.h.first).writeString(ss.str());
		unlock(h.h.first);
	}

private:
	int      partition_;   /// partition size, or 0 if partitioning is disabled
	int      offBase_;     /// Add this to reference offsets before outputting.
	                       /// (An easy way to make things 1-based instead of
	                       /// 0-based)
	bool     colorSeq_;    /// true -> print colorspace alignment sequence in colors
	bool     colorQual_;   /// true -> print colorspace quals as originals, not decoded
	bool     cost_;        /// true -> print statum and cost
	Bitset   suppress_;    /// output fields to suppress
	bool fullRef_;         /// print full reference name
	ReferenceMap *rmap_;   /// mapping to reference coordinate system.
	AnnotationMap *amap_;  ///
};

/**
 * Sink for outputting alignments in a binary format.
 */
class ChainingHitSink : public HitSink {
public:

	/**
	 * Construct a single-stream BinaryHitSink (default)
	 */
	ChainingHitSink(OutFileBuf* out, bool strata, AnnotationMap *amap, DECL_HIT_DUMPS2) :
	HitSink(out, PASS_HIT_DUMPS2), amap_(amap), strata_(strata)
	{
		ssmode_ |= ios_base::binary;
	}

	/**
	 * Report a batch of hits.
	 */
	virtual void reportHits(vector<Hit>& hs);

	/**
	 * Append a binary alignment to the output stream corresponding to
	 * the reference sequence involved.
	 */
	virtual void append(ostream& o, const Hit& h) {
		cerr << "Error: ChainingHitSink::append() not implemented" << endl;
		throw 1;
	}

	/**
	 * See hit.cpp
	 */
	virtual void reportMaxed(vector<Hit>& hs, PatternSourcePerThread& p);

	/**
	 * See hit.cpp
	 */
	virtual void reportUnaligned(PatternSourcePerThread& p);

protected:
	AnnotationMap *amap_;
	bool strata_;
};

/**
 * Sink that does nothing.
 */
class StubHitSink : public HitSink {
public:
	StubHitSink() : HitSink(new OutFileBuf(".tmp"), "", "", "", false, false, NULL) { }
	virtual void append(ostream& o, const Hit& h) { }
};

#endif /*HIT_H_*/

