#ifdef HMMER_BUILD_WITH_SSE2

#ifndef __HMMSEARCH_SSE_H__
#define __HMMSEARCH_SSE_H__

QList<float> sseScoring( unsigned char * dsq, int seqlen, plan7_s* hmm, HMMSeqGranulation * gr, U2::TaskStateInfo& ti );

#endif // __HMMSEARCH_SSE_H__

#endif // HMMER_SSE
