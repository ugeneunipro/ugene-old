#include <cassert>
#include "VirtualOffset.h"

namespace U2 {
namespace BAM {

VirtualOffset::VirtualOffset(quint64 packedOffset):
        packedOffset(packedOffset)
{
}

VirtualOffset::VirtualOffset(quint64 coffset, int uoffset):
        packedOffset((coffset << 16) | uoffset)
{
    assert(coffset <= 0xffffffffffff);
    assert(uoffset >= 0);
    assert(uoffset <= 0xffff);
}

quint64 VirtualOffset::getPackedOffset()const {
    return packedOffset;
}

quint64 VirtualOffset::getCoffset()const {
    return packedOffset >> 16;
}

int VirtualOffset::getUoffset()const {
    return (int)packedOffset & 0xffff;
}

bool VirtualOffset::operator==(VirtualOffset right)const {
    return packedOffset == right.packedOffset;
}

bool VirtualOffset::operator!=(VirtualOffset right)const {
    return packedOffset != right.packedOffset;
}

bool VirtualOffset::operator<(VirtualOffset right)const {
    return packedOffset < right.packedOffset;
}

bool VirtualOffset::operator>(VirtualOffset right)const {
    return packedOffset > right.packedOffset;
}

bool VirtualOffset::operator<=(VirtualOffset right)const {
    return packedOffset <= right.packedOffset;
}

bool VirtualOffset::operator>=(VirtualOffset right)const {
    return packedOffset >= right.packedOffset;
}

} // namespace BAM
} // namespace U2
