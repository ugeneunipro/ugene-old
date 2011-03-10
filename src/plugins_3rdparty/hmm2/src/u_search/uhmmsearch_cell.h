#ifdef UGENE_CELL
#ifndef __UHMMSEARCH_CELL_H__
#define __UHMMSEARCH_CELL_H__

struct plan7_s;
struct threshold_s;
struct histogram_s;
struct tophit_s;

namespace U2 {
class TaskStateInfo;

void main_loop_spe(struct plan7_s *hmm, const char * seq, int seqlen, struct threshold_s *thresh, int do_forward,
                    int do_null2, int do_xnu, struct histogram_s *histogram, struct tophit_s *ghit, struct tophit_s *dhit, 
                    int *ret_nseq, U2::TaskStateInfo & ti);

}  //namespace
#endif //__UHMMSEARCH_CELL_H__

#endif //UGENE_CELL
