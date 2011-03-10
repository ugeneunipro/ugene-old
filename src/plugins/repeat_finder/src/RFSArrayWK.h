#ifndef _U2_RF_SARRAYWK_ALG_H_
#define _U2_RF_SARRAYWK_ALG_H_

#include <U2Algorithm/SArrayIndex.h>
#include <U2Algorithm/SArrayBasedFindTask.h>
#include "RFBase.h"

#include <QtCore/QVector>

namespace U2 {

class SArrayIndex;
class RFSArrayWKSubtask;
class RFRollingArray;

class RFSArrayWKAlgorithm : public RFAlgorithmBase  {
    Q_OBJECT
    friend class RFSArrayWKSubtask;
public:
    RFSArrayWKAlgorithm(RFResultsListener* rl, const char* seqX, int sizeX, const char* seqY, int sizeY, 
                        DNAAlphabetType seqType, int w, int k);

    ~RFSArrayWKAlgorithm() {cleanup();}

    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);

    void cleanup();

private:
    void addResult(int a, int s, int l);
    void calculate(RFSArrayWKSubtask* t);
    
public:	
    QVector<int>    diagOffsets; //holds start position for the last checked window
    int             ARRAY_SIZE;
    int             SEARCH_SIZE;

    const char      *arraySeq;
    const char      *searchSeq;
    bool            arrayIsX;
    
    quint32         q;

    CreateSArrayIndexTask*  indexTask;
    int                     nThreads;

};

class RFSArrayWKSubtask : public Task, public SArrayIndex::SAISearchContext {
    Q_OBJECT
    friend class RFSArrayWKAlgorithm;
public:
    RFSArrayWKSubtask(RFSArrayWKAlgorithm* owner, int _tid);
    virtual ~RFSArrayWKSubtask(){}
    void run();

    RFSArrayWKAlgorithm*    owner;

    const int   tid;
};

} //namespace

#endif 
