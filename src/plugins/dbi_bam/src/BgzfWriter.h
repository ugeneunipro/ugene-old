#ifndef _U2_BAM_BGZF_WRITER_H_
#define _U2_BAM_BGZF_WRITER_H_

//using 3rd-party zlib (not included in ugene bundle) on *nix
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#include <zlib.h>
#else
#include "zlib.h"
#endif

#include <U2Core/IOAdapter.h>
#include "VirtualOffset.h"

namespace U2 {
namespace BAM {

class BgzfWriter
{
public:
    BgzfWriter(IOAdapter &ioAdapter);
    ~BgzfWriter();

    void write(const char *buff, qint64 size);
    void finish();

    VirtualOffset getOffset()const;
private:
    void finishBlock();

    static const int BUFFER_SIZE = 16384;
    static const int BLOCK_SIZE = 65536;
    IOAdapter &ioAdapter;
    z_stream stream;
    char buffer[BUFFER_SIZE];
    quint64 headerOffset;
    bool blockEnd;
    bool finished;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_BGZF_WRITER_H_
