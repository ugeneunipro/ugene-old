/**
 * range.h
 */

#ifndef RANGE_H_
#define RANGE_H_

#include <vector>
#include <stdint.h>

/**
 * A range along with the alignment it represents.
 */
struct Range {
	Range() :
		top(0xffffffff), bot(0), cost(0), stratum(0), numMms(0),
		fw(true), mate1(true), ebwt(NULL)
	{
		mms.clear();
		refcs.clear();
	}

	bool valid() const {
		return top < 0xffffffff;
	}

	void invalidate() {
		top = 0xffffffff;
	}

	uint32_t top;     // top of range
	uint32_t bot;     // bottom of range
	uint16_t cost;    // cost
	uint32_t stratum; // stratum
	uint32_t numMms;  // # mismatches
	bool fw;          // the forward orientation of read aligned?
	bool mate1;       // read aligned is #1 mate/single?
	std::vector<uint32_t> mms;   // list of positions with mismatches
	std::vector<uint8_t>  refcs; // reference characters at mismatch positions
	const Ebwt<seqan::String<seqan::Dna> > *ebwt;

	bool repOk() const {
		assert_eq(refcs.size(), mms.size());
		assert_eq(numMms, mms.size());
		assert_leq(stratum, numMms);
		return true;
	}
};

#endif /* RANGE_H_ */
