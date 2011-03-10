#ifndef _U2_RF_SARRAY_ALG_H_
#define _U2_RF_SARRAY_ALG_H_

#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtCore/QMutex>

#include <U2Algorithm/SArrayIndex.h>
#include <U2Algorithm/SArrayBasedFindTask.h>
#include "RFBase.h"

namespace U2 {

class RFSArrayWSubtask;

// finds exact matches using SArray index
class RFSArrayWAlgorithm : public RFAlgorithmBase {
    Q_OBJECT
    friend class RFSArrayWSubtask;
public:
    RFSArrayWAlgorithm(RFResultsListener* rl, const char* seqX, int sizeX, 
                    const char* seqY, int sizeY, DNAAlphabet *al, int w);
    
    void prepare();
    
    QList<Task*> onSubTaskFinished(Task* subTask);
    
    void run();
    
    /** 8x/GAP mem usage (GAP~=W/2)*/
    void calculate(RFSArrayWSubtask* t);
    
private:
    void run(RFSArrayWSubtask* t);
    void addResult(int a, int s, int l, RFSArrayWSubtask* t);
    void processBoundaryResults();

    static int getWGap(int w);

    const char*     arraySeq;
    const char*     searchSeq;
    int             ARRAY_SIZE;
    int             SEARCH_SIZE;
    bool            arrayIsX;
    int             arrayPercent;

    QMutex          boundaryMutex;
    
    quint32         bitMaskCharBitsNum;
    const quint32*  bitMask;
    BitsTable       bt;


    /** boundary results */
    QVector<RFResult> bresults;

    CreateSArrayIndexTask*  indexTask;
    int                     nThreads;

};

class RFSArrayWSubtask : public Task, public SArrayIndex::SAISearchContext {
    Q_OBJECT
    friend class RFSArrayWAlgorithm;
public:
    RFSArrayWSubtask(RFSArrayWAlgorithm* owner, int sStart, int sEnd, int _tid);
    
    void run();

    RFSArrayWAlgorithm* owner;
    const int           sStart;
    const int           sEnd;
    const int           tid;
};

class CheckEdge {
public:

    CheckEdge(const char* _posS = NULL, const char* _lastS = NULL, qint32 _diag = 0) 
        : posS(_posS), lastS(_lastS), diag(_diag){ next = this; prev = this;}

    void fromChain() {
        prev->next = next;
        next->prev = prev;
    }

    void toChain(CheckEdge* chain) {
        next = chain;
        prev = chain->prev;
        chain->prev->next = this;
        chain->prev = this;
    }

    const char*     posS;
    const char*     lastS;
    CheckEdge*      next;
    CheckEdge*      prev;
    int             diag;
};

class EdgePool {
public:
    EdgePool(int size) {
        pool.resize(size);
        for (int i = 0; i < size; i++) {
            pool[i] = new CheckEdge();
        }
        nAvailable = size;
    }
    ~EdgePool() {qDeleteAll(pool);}


    CheckEdge* allocEdge(const char* posS, const char* lastS, int diag) {
        if (nAvailable != 0) {
            assert(nAvailable > 0);
            CheckEdge* edge = pool[--nAvailable];
            edge->posS = posS; edge->lastS = lastS; edge->diag = diag; edge->next = NULL; edge->prev = edge;
            pool[nAvailable] = NULL;
            return edge;
        }
        return new CheckEdge(posS, lastS, diag);
    }

    void returnEdge(CheckEdge* e) {
        if (nAvailable != pool.size()) {
            assert(nAvailable < pool.size() && nAvailable >=0);
            assert(pool[nAvailable] == NULL);
            pool[nAvailable++] = e;
        } else {
            delete e;
        }
    }

    int                 nAvailable;
    QVector<CheckEdge*> pool;
};

}//namespace

#endif
