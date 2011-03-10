#ifndef SSPRED_HMULTI_H
#define SSPRED_HMULTI_H

#include <QtCore/QVector>
#include <QtCore/QByteArray>
#include "ssdefs.h"

class QTemporaryFile;


class PsiPassTwo {
    const char           *wtfnm;
    int             nwtsum, *fwt_to, *lwt_to;
    REAL            *activation, *bias, **weight;
    float           profile[MAXSEQLEN][3];
    char            seq[MAXSEQLEN];
    int             seqlen, nprof;
public:
    PsiPassTwo();
    ~PsiPassTwo();
    void compute_output(void);
    void load_wts(const char *fname);
    void init(void);
    QByteArray predict(int niters, float dca, float dcb, const char *outname);
    int getss(FILE * lfil);
    int runPsiPass(int argc, const char *argv[], QByteArray& result);
};

#endif // SSPRED_HMULTI_H

