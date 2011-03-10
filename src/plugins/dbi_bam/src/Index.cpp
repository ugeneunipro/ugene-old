#include "Index.h"

namespace U2 {
namespace BAM {

// Index::ReferenceIndex::Chunk

Index::ReferenceIndex::Chunk::Chunk(VirtualOffset start, VirtualOffset end):
    start(start),
    end(end)
{
}

VirtualOffset Index::ReferenceIndex::Chunk::getStart()const {
    return start;
}
VirtualOffset Index::ReferenceIndex::Chunk::getEnd()const {
    return end;
}

// Index::ReferenceIndex

Index::ReferenceIndex::ReferenceIndex(const QList<QList<Chunk> > &bins, const QList<VirtualOffset> &intervals):
    bins(bins),
    intervals(intervals)
{
}

const QList<QList<Index::ReferenceIndex::Chunk> > &Index::ReferenceIndex::getBins()const {
    return bins;
}

const QList<VirtualOffset> &Index::ReferenceIndex::getIntervals()const {
    return intervals;
}

// Index

Index::Index(const QList<ReferenceIndex> &referenceIndices):
    referenceIndices(referenceIndices)
{
}

const QList<Index::ReferenceIndex> &Index::getReferenceIndices()const {
    return referenceIndices;
}

} // namespace BAM
} // namespace U2
