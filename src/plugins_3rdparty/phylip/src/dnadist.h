#ifndef DNADIST_H
#define DNADIST_H

#include <QString>
#include <QList>

#include "seq.h"
#include "phylip.h"
#define iterationsd     100   /* number of iterates of EM for each distance */

typedef struct valrec {
	double rat, ratxv, z1, y1, z1zz, z1yy, z1xv;
} valrec;

extern Phylip_Char infilename[FNMLNGTH], outfilename[FNMLNGTH], catfilename[FNMLNGTH], weightfilename[FNMLNGTH];
extern long sites, categs, weightsum, datasets, ith, rcategs;
extern boolean freqsfrom, jukes, kimura, logdet, gama, invar, similarity, lower, f84,
weights, progress, ctgry, mulsets, justwts, firstset, baddists;
extern boolean matrix_flags;       /* Matrix output format */
extern node **nodep;
extern double xi, xv, ttratio, ttratio0, freqa, freqc, freqg, freqt, freqr, freqy,
freqar, freqcy, freqgr, freqty, cvi, invarfrac, sumrates, fracchange;
extern steptr oldweight;
extern double rate[maxcategs];
extern double **d;
extern double sumweightrat;                  /* these values were propagated  */
extern double *weightrat;                    /* to global values from         */
extern valrec tbl[maxcategs];                /* function makedists.           */


class DNADistModelTypes {
public:
    static QString F84;
    static QString Kimura;
    static QString JukesCantor;
    static QString LogDet;
    static QList<QString> getDNADistModelTypes();
};

namespace U2 {

class CreatePhyTreeSettings;

void setDNADistSettings(const CreatePhyTreeSettings& settings);
const CreatePhyTreeSettings& getDNADistSettings();

}

#ifndef OLDC
/* function  prototypes */ 
void   getoptions(void);
void   allocrest(void);
void   reallocsites(void);
void   doinit(void);
void   inputcategories(void);
void   printcategories(void);
void   inputoptions(void);
void   dnadist_sitesort(void);
void   dnadist_sitecombine(void);
void   dnadist_sitescrunch(void);
void   makeweights(void);
void   dnadist_makevalues(void);
void   dnadist_empiricalfreqs(void);
void   getinput(void);
void   inittable(void);
double lndet(double (*a)[4]);
void   makev(long, long, double *);
void   makedists(void);
void   writedists(void);
/* function  prototypes */
#endif

#endif
