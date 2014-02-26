/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_BAM_BGZF_WRITER_H_
#define _U2_BAM_BGZF_WRITER_H_

//using 3rd-party zlib (not included in ugene bundle) on *nix
#if defined(Q_OS_UNIX)
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
