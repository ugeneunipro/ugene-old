#ifndef _U2_RF_DIAGONAL_ALG_H_
#define _U2_RF_DIAGONAL_ALG_H_

#include "RFBase.h"

#include <QtCore/QMutex>

namespace U2 {

class RFDiagonalWKSubtask;

class RFDiagonalAlgorithmWK : public RFAlgorithmBase {
    Q_OBJECT
    friend class RFDiagonalWKSubtask;
public:
    RFDiagonalAlgorithmWK(RFResultsListener* rl, const char* seqX, int sizeX, const char* seqY, int sizeY, 
                            DNAAlphabetType seqType,  int w, int k);

    void prepare();

protected:    
    int                 START_DIAG;
    int                 END_DIAG;
};

class RFDiagonalWKSubtask : public Task {
    Q_OBJECT
    friend class RFDiagonalAlgorithmWK;
public:
    RFDiagonalWKSubtask(RFDiagonalAlgorithmWK* owner, int threadNum, int totalThreads);

    void run();

private:        
    void processDiagonal(int x, int y);
    int getDiagLen(int d) const;
    int processMatch(const char* x, const char* y, const char* xEnd, const char* yEnd, int c);

    RFDiagonalAlgorithmWK*  owner;
    
    int                     threadNum;
    int                     nThreads;
    const char*             dataX;
    const char*             dataY;
    
    QVector<RFResult>       diagResults;

    quint64                 areaS;
    quint64                 currentS;
    quint64                 nextPercentS;
};

} //namespace


#endif
