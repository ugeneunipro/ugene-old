#ifndef REF_READ_H_
#define REF_READ_H_

#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <ctype.h>
#include <fstream>
#include <stdexcept>
#include <seqan/sequence.h>
#include "alphabet.h"
#include "assert_helpers.h"
#include "filebuf.h"
#include "word_io.h"

using namespace std;
using namespace seqan;

/**
 * Encapsulates a stretch of the reference containing only unambiguous
 * characters.  From an ordered list of RefRecords, one can (almost)
 * deduce the "shape" of the reference sequences (almost because we
 * lose information about stretches of ambiguous characters at the end
 * of reference sequences).
 */
struct RefRecord {
	RefRecord() : off(), len(), first() { }
	RefRecord(uint32_t _off, uint32_t _len, bool _first) :
		off(_off), len(_len), first(_first)
	{ }

	RefRecord(FILE *in, bool swap) {
		assert(in != NULL);
		if(!fread(&off, 4, 1, in)) {
			cerr << "Error reading RefRecord offset from FILE" << endl;
			throw 1;
		}
		if(swap) off = endianSwapU32(off);
		if(!fread(&len, 4, 1, in)) {
			cerr << "Error reading RefRecord offset from FILE" << endl;
			throw 1;
		}
		if(swap) len = endianSwapU32(len);
		first = fgetc(in) ? true : false;
	}

	RefRecord(int in, bool swap) {
		off = readU32(in, swap);
		len = readU32(in, swap);
		char c;
		#ifdef _MSC_VER
		if(!read((std::FILE*)in, &c, 1)) {
		#else
		if(!read(in, &c, 1)) {
		#endif
			cerr << "Error reading RefRecord 'first' flag" << endl;
			throw 1;
		}
		first = (c ? true : false);
	}

	void write(std::ostream& out, bool be) {
		writeU32(out, off, be);
		writeU32(out, len, be);
		out.put(first ? 1 : 0);
	}

	uint32_t off; /// Offset of the first character in the record
	uint32_t len; /// Length of the record
	bool   first; /// Whether this record is the first for a reference sequence
};

enum {
	REF_READ_FORWARD = 0, // don't reverse reference sequence
	REF_READ_REVERSE,     // reverse entire reference sequence
	REF_READ_REVERSE_EACH // reverse each unambiguous stretch of reference
};

/**
 * Parameters governing treatment of references as they're read in.
 */
struct RefReadInParams {
	RefReadInParams(bool col, int r, bool nsToA, bool bisulf) :
		color(col), reverse(r), nsToAs(nsToA), bisulfite(bisulf) { }
	// extract colors from reference
	bool color;
	// reverse each reference sequence before passing it along
	int reverse;
	// convert ambiguous characters to As
	bool nsToAs;
	// bisulfite-convert the reference
	bool bisulfite;
};

extern RefRecord
fastaRefReadSize(
	FileBuf& in,
	const RefReadInParams& rparms,
	bool first,
	BitpairOutFileBuf* bpout = NULL);

extern std::pair<size_t, size_t>
fastaRefReadSizes(
	vector<FileBuf*>& in,
	vector<RefRecord>& recs,
	vector<uint32_t>& plens,
	const RefReadInParams& rparms,
	BitpairOutFileBuf* bpout,
	int& numSeqs);

extern void
reverseRefRecords(
	const vector<RefRecord>& src,
	vector<RefRecord>& dst,
	bool recursive = false,
	bool verbose = false);

/**
 * Reads the next sequence from the given FASTA file and appends it to
 * the end of dst, optionally reversing it.
 */
template <typename TStr>
static RefRecord fastaRefReadAppend(FileBuf& in,
                                    bool first,
                                    TStr& dst,
                                    RefReadInParams& rparms,
                                    string* name = NULL)
{
	typedef typename Value<TStr>::Type TVal;
	int c;
	static int lastc = '>';
	if(first) {
		c = in.getPastWhitespace();
		if(c != '>') {
			cerr << "Reference file does not seem to be a FASTA file" << endl;
			throw 1;
		}
		lastc = c;
	}
	assert_neq(-1, lastc);

	// RefRecord params
	size_t len = 0;
	size_t off = 0;
	first = true;

	size_t ilen = length(dst);

	// Chew up the id line; if the next line is either
	// another id line or a comment line, keep chewing
	int lc = -1; // last-DNA char variable for color conversion
	c = lastc;
	if(c == '>' || c == '#') {
		do {
			while (c == '#') {
				if((c = in.getPastNewline()) == -1) {
					lastc = -1;
					goto bail;
				}
			}
			assert_eq('>', c);
			while(true) {
				c = in.get();
				if(c == -1) {
					lastc = -1;
					goto bail;
				}
				if(c == '\n' || c == '\r') {
					while(c == '\r' || c == '\n') c = in.get();
					if(c == -1) {
						lastc = -1;
						goto bail;
					}
					break;
				}
				if (name) name->push_back(c);
			}
			// c holds the first character on the line after the name
			// line
		} while (c == '>' || c == '#');
	} else {
		ASSERT_ONLY(int cc = toupper(c));
		assert(cc != 'A' && cc != 'C' && cc != 'G' && cc != 'T');
		first = false;
	}

	// Skip over an initial stretch of gaps or ambiguous characters.
	// For colorspace we skip until we see two consecutive unambiguous
	// characters (i.e. the first unambiguous color).
	while(true) {
		int cat = dna4Cat[c];
		if(rparms.nsToAs && cat == 2) {
			c = 'A';
		}
		int cc = toupper(c);
		if(rparms.bisulfite && cc == 'C') c = cc = 'T';
		if(cat == 1) {
			// This is a DNA character
			if(rparms.color) {
				if(lc != -1) {
					// Got two consecutive unambiguous DNAs
					break; // to read-in loop
				}
				// Keep going; we need two consecutive unambiguous DNAs
				lc = charToDna5[(int)c];
				// The 'if(off > 0)' takes care of the case where
				// the reference is entirely unambiguous and we don't
				// want to incorrectly increment off.
				if(off > 0) off++;
			} else {
				break; // to read-in loop
			}
		} else if(cat == 2) {
			if(lc != -1 && off == 0) {
				off++;
			}
			lc = -1;
			off++; // skip it
		} else if(c == '>') {
			lastc = '>';
			goto bail;
		}
		c = in.get();
		if(c == -1) {
			lastc = -1;
			goto bail;
		}
	}
	if(first && rparms.color && off > 0) {
		// Handle the case where the first record has ambiguous
		// characters but we're in color space; one of those counts is
		// spurious
		off--;
	}
	assert(!rparms.color || lc != -1);
	assert_eq(1, dna4Cat[c]);

	// in now points just past the first character of a sequence
	// line, and c holds the first character
	while(true) {
		// Note: can't have a comment in the middle of a sequence,
		// though a comment can end a sequence
		int cat = dna4Cat[c];
		assert_neq(2, cat);
		if(cat == 1) {
			// Consume it
			if(!rparms.color || lc != -1) len++;
			// Add it to referenece buffer
			if(rparms.color) {
				appendValue(dst, (Dna)dinuc2color[charToDna5[(int)c]][lc]);
			} else if(!rparms.color) {
				appendValue(dst, (Dna)(char)c);
			}
			assert_lt((uint8_t)(Dna)dst[length(dst)-1], 4);
			lc = charToDna5[(int)c];
		}
		c = in.get();
		if(rparms.nsToAs && dna4Cat[c] == 2) c = 'A';
		if (c == -1 || c == '>' || c == '#' || dna4Cat[c] == 2) {
			lastc = c;
			break;
		}
		if(rparms.bisulfite && toupper(c) == 'C') c = 'T';
	}

  bail:
	// Optionally reverse the portion that we just appended.
	// ilen = length of buffer before this last sequence was appended.
	if(rparms.reverse == REF_READ_REVERSE_EACH) {
		// Find limits of the portion we just appended
		size_t nlen = length(dst);
		assert_eq(nlen - ilen, len);
		if(len > 0) {
			size_t halfway =  ilen + (len>>1);
			// Reverse it in-place
			for(size_t i = ilen; i < halfway; i++) {
				size_t diff = i-ilen;
				size_t j = nlen-diff-1;
				TVal tmp = dst[i];
				dst[i] = dst[j];
				dst[j] = tmp;
			}
		}
	}
	return RefRecord(off, len, first);
}

#endif /*ndef REF_READ_H_*/
