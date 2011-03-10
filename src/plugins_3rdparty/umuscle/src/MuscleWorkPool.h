#ifndef _MUSCLEWORKPOOL_H_
#define _MUSCLEWORKPOOL_H_

#include "MuscleParallel.h"
#include "MuscleUtils.h"

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QSemaphore>

#include "muscle/scorehistory.h"

namespace U2 {

    enum TreeNodeStatus {
        TreeNodeStatus_WaitForChild,
        TreeNodeStatus_Available,
        TreeNodeStatus_Processing,
        TreeNodeStatus_Done
    };

    enum RefineTreeNodeStatus {
        RefineTreeNodeStatus_Available,
        RefineTreeNodeStatus_Processing,
        RefineTreeNodeStatus_Done,
        RefineTreeNodeStatus_DoneFinaly
    };

    struct MuscleWorkPool {
        MuscleWorkPool(MuscleContext *_ctx, const MuscleTaskSettings  &_config, TaskStateInfo& _ti, int _nThreads, const MAlignment& _ma, MAlignment& _res, bool _mhack);
            
        ~MuscleWorkPool() ;

        unsigned getJob();
        unsigned getNextJob(unsigned uNodeIndex);

        MuscleContext       *ctx;
        const MuscleTaskSettings  &config;
        MAlignment          ma;
        MAlignment&         res;
        bool                mhack;
        SeqVect             v;
        Tree                GuideTree;
        MSA                 a;
        WEIGHT              *Weights;
        ProgNode            *ProgNodes;
        MuscleParamsHelper  *ph;
        TaskStateInfo&      ti;
        TreeNodeStatus*     treeNodeStatus;
        unsigned*           treeNodeIndexes;
        int                 nThreads;
        unsigned            uJoin;
        QMutex              jobMgrMutex;
        QMutex              proAligMutex;
        ////////////////////////////
        // Refine
        ////////////////////////////
        void refineConstructot();
        void refineClear();
        void reset();

        unsigned refineGetJob(MSA *_msaIn, int workerID);

        unsigned refineGetNextJob(MSA *_msaIn, bool accepted, SCORE scoreMax, unsigned index, int workerID);

        unsigned isRefineDone() {
            return refineDone; /*|| ctx->isCanceled();*/
        }

        bool                *ptrbOscillating;
        unsigned            oscillatingIter;
        bool                bAnyAccepted;
        unsigned            *InternalNodeIndexes;
        unsigned            uInternalNodeCount;
        bool                bReversed;
        bool                bRight;
        unsigned            uIter;
        ScoreHistory        *History;
        bool                bLockLeft;
        bool                bLockRight;

        bool                refineDone;
        QSemaphore          mainSem;
        QSemaphore          childSem;
        QMutex              mut;
        RefineTreeNodeStatus* refineNodeStatuses;
        bool*               needRestart;
        unsigned            lastAcceptedIndex;
        unsigned*           currentNodeIndex;
        unsigned*           workerStartPos; 
        MSA*                msaIn;

        unsigned            uIters;
        unsigned            uRangeIndex;
        unsigned            uRangeCount;
        TaskStateInfo       *refineTI;

    };

#endif //_MUSCLEWORKPOOL_H_
} //namespace