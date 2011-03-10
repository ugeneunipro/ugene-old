#ifndef _U2_BAM_VIRTUAL_OFFSET_H_
#define _U2_BAM_VIRTUAL_OFFSET_H_

#include <QtGlobal>

namespace U2 {
namespace BAM {

class VirtualOffset
{
public:
    VirtualOffset(quint64 packedOffset);
    VirtualOffset(quint64 coffset, int uoffset);
    quint64 getPackedOffset()const;
    quint64 getCoffset()const;
    int getUoffset()const;
    bool operator==(VirtualOffset right)const;
    bool operator!=(VirtualOffset right)const;
    bool operator<(VirtualOffset right)const;
    bool operator>(VirtualOffset right)const;
    bool operator<=(VirtualOffset right)const;
    bool operator>=(VirtualOffset right)const;
private:
    quint64 packedOffset;
};

} // namespace BAM
} // namespace U2

#endif // _VIRTUAL_OFFSET_H_
