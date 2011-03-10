#include "BowtieContext.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <seqan/index.h>
#include <seqan/sequence.h>
#include <seqan/file.h>
#include "assert_helpers.h"
#include "endian_swap.h"
#include "ebwt.h"
#include "formats.h"
#include "sequence_io.h"
#include "tokenize.h"
#include "timer.h"
#include "ref_read.h"
#include "filebuf.h"
#include "reference.h"

/**
 * \file Driver for the bowtie-build indexing tool.
 */

static void resetOptions() {
	BowtieContext::getBuildContext()->resetOptions();
}

/**
 * Drive the Ebwt construction process and optionally sanity-check the
 * result.
 */
template<typename TStr>
static void driver(const string& infile,
                   vector<string>& infiles,
                   const string& outfile,
                   bool reverse = false)
{
	struct FileBufHelper {
		~FileBufHelper() {for(size_t i = 0; i < v.size(); i++) delete v[i];}
		vector<FileBuf*> v;
	} is_helper;
	vector<FileBuf*> &is = is_helper.v;
	BowtieContext::Build* ctx = BowtieContext::getBuildContext();
	bool bisulfite = false;
        RefReadInParams refparams(ctx->color, reverse ? ctx->reverseType : REF_READ_FORWARD, ctx->nsToAs, bisulfite);
	assert_gt(infiles.size(), 0);
	if(ctx->format == CMDLINE) {
		// Adapt sequence strings to stringstreams open for input
		stringstream *ss = new stringstream();
		for(size_t i = 0; i < infiles.size(); i++) {
			(*ss) << ">" << i << endl << infiles[i] << endl;
		}
		FileBuf *fb = new FileBuf(ss);
		bool check = (fb != NULL) && (!fb->eof()) && (fb->get() == '>') && (!fb->eof());
		ASSERT_ONLY(fb->reset());
		if(!check) {
			throw new BowtieException("invalid reference sequence file");
		}
		is.push_back(fb);
	} else {
		// Adapt sequence files to ifstreams
		for(size_t i = 0; i < infiles.size(); i++) {
			FILE *f = fopen(infiles[i].c_str(), "r");
			if (f == NULL) {
				ostringstream err;
				err << "Error: could not open "<< infiles[i] << endl;
				throw BowtieException(err.str());
			}
			FileBuf *fb = new FileBuf(f);
			bool check = (fb != NULL) && (!fb->eof()) && (fb->get() == '>') && (!fb->eof());
			ASSERT_ONLY(fb->reset());
			if(!check) {
				throw new BowtieException("invalid reference sequence file");
			}
			is.push_back(fb);
		}
	}
	// Vector for the ordered list of "records" comprising the input
	// sequences.  A record represents a stretch of unambiguous
	// characters in one of the input sequences.
	vector<RefRecord> szs;
	vector<uint32_t> plens;
	std::pair<size_t, size_t> sztot;
	{
		if(ctx->verbose) BowtieContext::verbose("Reading reference sizes");
		Timer _t(cout, "  Time reading reference sizes: ", ctx->verbose);
		if(!reverse && (ctx->writeRef || ctx->justRef)) {
			// For forward reference, dump it to .3.ebwt and .4.ebwt
			// files
			string file3 = outfile + ".3.ebwt";
			string file4 = outfile + ".4.ebwt";
			// Open output stream for the '.3.ebwt' file which will
			// hold the size records.
			ofstream fout3(file3.c_str(), ios::binary);
			if(!fout3.good()) {
				ostringstream err;
				err << "Could not open index file for writing: \"" << file3 << "\"" << endl
					 << "Please make sure the directory exists and that permissions allow writing by" << endl
					 << "Bowtie." << endl;
				throw BowtieException(err.str());
			}
			BitpairOutFileBuf bpout(file4.c_str());
			// Read in the sizes of all the unambiguous stretches of
			// the genome into a vector of RefRecords.  The input
			// streams are reset once it's done.
			writeU32(fout3, 1, ctx->bigEndian); // endianness sentinel
			if(ctx->color) {
				refparams.color = false;
				// Make sure the .3.ebwt and .4.ebwt files contain
				// nucleotides; not colors
				int numSeqs = 0;
				std::pair<size_t, size_t> sztot2 =
					fastaRefReadSizes(is, szs, plens, refparams, &bpout, numSeqs);
				refparams.color = true;
				writeU32(fout3, szs.size(), ctx->bigEndian); // write # records
				for(size_t i = 0; i < szs.size(); i++) {
					szs[i].write(fout3, ctx->bigEndian);
				}
				szs.clear();
				plens.clear();
				// Now read in the colorspace size records; these are
				// the ones that were indexed
				int numSeqs2 = 0;
				sztot = fastaRefReadSizes(is, szs, plens, refparams, NULL, numSeqs2);
				assert_geq(numSeqs, numSeqs2);
				//assert_eq(sztot2.second, sztot.second + numSeqs);
			} else {
				int numSeqs = 0;
				sztot = fastaRefReadSizes(is, szs, plens, refparams, &bpout, numSeqs);
				writeU32(fout3, szs.size(), ctx->bigEndian); // write # records
				for(size_t i = 0; i < szs.size(); i++) szs[i].write(fout3, ctx->bigEndian);
			}
			if(sztot.first == 0) {
				ostringstream err;
				err << "Error: No unambiguous stretches of characters in the input.  Aborting..." << endl;
				throw BowtieException(err.str());
			}
			assert_gt(sztot.first, 0);
			assert_gt(sztot.second, 0);
			bpout.close();
			fout3.close();
#ifndef NDEBUG
			if(ctx->sanityCheck) {
				BitPairReference bpr(
					outfile, // ebwt basename
					ctx->color,   // expect color?
					true,    // sanity check?
					&infiles,// files to check against
					NULL,    // sequences to check against
					ctx->format == CMDLINE, // whether infiles contains strings
					true,    // load sequence?
					false,   // use memory-mapped files
					false,   // use shared memory
					false,   // sweep through memory-mapped memory
					false,   // be talkative
					false);  // be talkative
			}
#endif
		} else {
			// Read in the sizes of all the unambiguous stretches of the
			// genome into a vector of RefRecords
			int numSeqs = 0;
			sztot = fastaRefReadSizes(is, szs, plens, refparams, NULL, numSeqs);
#ifndef NDEBUG
			if(refparams.color) {
				refparams.color = false;
				vector<RefRecord> szs2;
				vector<uint32_t> plens2;
				int numSeqs2 = 0;
				std::pair<size_t, size_t> sztot2 =
					fastaRefReadSizes(is, szs2, plens2, refparams, NULL, numSeqs2);
				assert_leq(numSeqs, numSeqs2);
				// One less color than base
				//assert_geq(sztot2.second, sztot.second + numSeqs);
				refparams.color = true;
			}
#endif
		}
	}
	if(ctx->justRef) return;
	assert_gt(sztot.first, 0);
	assert_gt(sztot.second, 0);
	assert_gt(szs.size(), 0);
	// Construct Ebwt from input strings and parameters
	Ebwt<TStr> ebwt(refparams.color ? 1 : 0,
	                ctx->lineRate,
	                ctx->linesPerSide,
	                ctx->offRate,      // suffix-array sampling rate
	                -1,           // ISA sampling rate
	                ctx->ftabChars,    // number of chars in initial arrow-pair calc
	                outfile,      // basename for .?.ebwt files
	                !reverse,     // fw
	                !ctx->entireSA,    // useBlockwise
	                ctx->bmax,         // block size for blockwise SA builder
	                ctx->bmaxMultSqrt, // block size as multiplier of sqrt(len)
	                ctx->bmaxDivN,     // block size as divisor of len
	                ctx->noDc? 0 : ctx->dcv,// difference-cover period
	                is,           // list of input streams
	                szs,          // list of reference sizes
	                plens,        // list of not-all-gap reference sequence lengths
	                sztot.first,  // total size of all unambiguous ref chars
	                refparams,    // reference read-in parameters
	                ctx->seed,         // pseudo-random number generator seed
	                -1,           // override offRate
	                -1,           // override isaRate
	                ctx->verbose,      // be talkative
	                ctx->autoMem,      // pass exceptions up to the toplevel so that we can adjust memory settings automatically
	                ctx->sanityCheck); // verify results and internal consistency
	// Note that the Ebwt is *not* resident in memory at this time.  To
	// load it into memory, call ebwt.loadIntoMemory()
	if(ctx->verbose) {
		// Print Ebwt's vital stats
		ebwt.eh().print(cout);
	}
	if(ctx->sanityCheck) {
		// Try restoring the original string (if there were
		// multiple texts, what we'll get back is the joined,
		// padded string, not a list)
		ebwt.loadIntoMemory(
			refparams.color ? 1 : 0,
			-1,
			false,
			false);
		TStr s2; ebwt.restore(s2);
		ebwt.evictFromMemory();
		{
			TStr joinedss = Ebwt<TStr>::join(
				is,          // list of input streams
				szs,         // list of reference sizes
				sztot.first, // total size of all unambiguous ref chars
				refparams,   // reference read-in parameters
				ctx->seed);       // pseudo-random number generator seed
			if(refparams.reverse == REF_READ_REVERSE) {
				reverseInPlace(joinedss);
			}
			assert_eq(length(joinedss), length(s2));
			assert_eq(joinedss, s2);
		}
		if(ctx->verbose) {
			if(length(s2) < 1000) {
				cout << "Passed restore check: " << s2 << endl;
			} else {
				cout << "Passed restore check: (" << length(s2) << " chars)" << endl;
			}
		}
	}
}

static const char *argv0 = NULL;

/**
 * main function.  Parses command-line arguments.
 */
int bowtieBuildAdapter(const string& infile, const string& outfile) {
	BowtieContext::Build* ctx = BowtieContext::getBuildContext();
	resetOptions();
	vector<string> infiles;

	tokenize(infile, ",", infiles);
	if(infiles.size() < 1) {
		cerr << "Tokenized input file list was empty!" << endl;
		return 1;
	}
	// Seed random number generator
	srand(ctx->seed);
	{
		if(!ctx->packed) {
			try {
				driver<String<Dna, Alloc<> > >(infile, infiles, outfile);
			} catch(bad_alloc& e) {
				if(ctx->autoMem) {
					cerr << "Switching to a packed string representation." << endl;
					ctx->packed = true;
				} else {
					throw e;
				}
			}
		}
		if(ctx->packed) {
			driver<String<Dna, Packed<Alloc<> > > >(infile, infiles, outfile);
		}
	}
	if(ctx->doubleEbwt) {
		ctx->secondEbwtRound = true;
		srand(ctx->seed);
		Timer timer(cout, "Total time for backward call to driver() for mirror index: ", ctx->verbose);
		if(!ctx->packed) {
			try {
				driver<String<Dna, Alloc<> > >(infile, infiles, outfile + ".rev", true);
			} catch(bad_alloc& e) {
				if(ctx->autoMem) {
					cerr << "Switching to a packed string representation." << endl;
					ctx->packed = true;
				} else {
					throw e;
				}
			}
		}
		if(ctx->packed) {
			driver<String<Dna, Packed<Alloc<> > > >(infile, infiles, outfile + ".rev", true);
		}
	}
	return 0;
}
