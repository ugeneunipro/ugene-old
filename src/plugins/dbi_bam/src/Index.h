#ifndef _U2_BAM_INDEX_H_
#define _U2_BAM_INDEX_H_

#include <QtCore/QList>
#include "VirtualOffset.h"

namespace U2 {
namespace BAM {

class Index
{
public:
    class ReferenceIndex {
    public:
        class Chunk {
        public:
            Chunk(VirtualOffset start, VirtualOffset end);
            VirtualOffset getStart()const;
            VirtualOffset getEnd()const;
        private:
            VirtualOffset start;
            VirtualOffset end;
        };
        ReferenceIndex(const QList<QList<Chunk> > &bins, const QList<VirtualOffset> &intervals);
        const QList<QList<Chunk> > &getBins()const;
        const QList<VirtualOffset> &getIntervals()const;
    private:
        QList<QList<Chunk> > bins;
        QList<VirtualOffset> intervals;
    };
    Index(const QList<ReferenceIndex> &referenceIndices);
    const QList<ReferenceIndex> &getReferenceIndices()const;
private:
    QList<ReferenceIndex> referenceIndices;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_INDEX_H_
