#ifndef SSPRED_AVPRED_H
#define SSPRED_AVPRED_H

#include <QtCore/QVector>
#include <QtCore/QByteArray>
#include <QtCore/QStringList>

#include "ssdefs.h"

class QByteArray;
class QTemporaryFile;

class PsiPassOne {
    
    const char    *wtfnm;
    int     *nwtsum, *fwt_to, *lwt_to;
    float   *activation, *bias, **weight;
    int     profile[MAXSEQLEN][20];
    int     seqlen;
    QTemporaryFile* matrixFile;
    QByteArray seq;
    QStringList weightFileNames;
public:
    PsiPassOne(QTemporaryFile* matFile, const QStringList& weightFiles);
    ~PsiPassOne();
    void compute_output(void);
    void load_wts(const char *fname);
    void init(void);
    int getmtx();
    void predict();
    int runPsiPass();
   
};


#endif // SSPRED_AVPRED_H

