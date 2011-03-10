#ifndef _U2_UMUSCLE_ADAPTER_H_
#define _U2_UMUSCLE_ADAPTER_H_

#include <QtCore/QObject>

namespace U2 {

class MAlignment;
class TaskStateInfo;

class MuscleAdapter : public QObject {
Q_OBJECT
public:
    static void align(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti, bool mhack = true);

    static void refine(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti);

    static void align2Profiles(const MAlignment& ma1, const MAlignment& ma2, MAlignment& res, TaskStateInfo& ti);
    
    static void addUnalignedSequencesToProfile( const MAlignment& ma, const MAlignment& unalignedSeqs, MAlignment& res, TaskStateInfo& ti);

private:
    static void alignUnsafe(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti, bool mhack);
    
    static void refineUnsafe(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti);

    static void align2ProfilesUnsafe(const MAlignment& ma1, const MAlignment& ma2, MAlignment& res, TaskStateInfo& ti);

    static void addUnalignedSequencesToProfileUnsafe(const MAlignment& ma, const MAlignment& unalignedSeqs, MAlignment& res, TaskStateInfo& ti);
};

}//namespace

#endif
