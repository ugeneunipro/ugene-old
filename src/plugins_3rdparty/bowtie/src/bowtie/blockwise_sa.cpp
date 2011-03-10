#include "BowtieContext.h"
#include "blockwise_sa.h"
#include "ebwt.h"

template<>
bool Ebwt<String<Dna, Packed<> > >::isPacked() {
	return true;
}

template<>
void KarkkainenBlockwiseSA<String<Dna, Packed<> > >::qsort(String<uint32_t>& bucket)
{
	const String<Dna, Packed<> >& t = this->text();
	uint32_t *s = begin(bucket);
	uint32_t slen = seqan::length(bucket);
	uint32_t len = seqan::length(t);
	if(_dc != NULL) {
		// Use the difference cover as a tie-breaker if we have it
		VMSG_NL("  (Using difference cover)");
		// Can't use the text's 'host' array because the backing
		// store for the packed string is not one-char-per-elt.
		mkeyQSortSufDcU8(t, t, len, s, slen, *_dc,
			ValueSize<Dna>::VALUE,
			this->verbose(), this->sanityCheck());
	} else {
		VMSG_NL("  (Not using difference cover)");
		// We don't have a difference cover - just do a normal
		// suffix sort
		mkeyQSortSuf(t, s, slen, ValueSize<Dna>::VALUE,
			this->verbose(), this->sanityCheck());
	}
}

