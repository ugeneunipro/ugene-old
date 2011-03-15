/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_BAM_WRITER_H_
#define _U2_BAM_WRITER_H_

#include <U2Core/IOAdapter.h>
#include "Header.h"
#include "Alignment.h"
#include "BgzfWriter.h"

namespace U2 {
namespace BAM {

class Writer
{
public:
    Writer(IOAdapter &ioAdapter);
    void writeHeader(const Header &header);
    void writeRead(const Alignment &alignment);
    void finish();
private:
    void writeBytes(const char *buffer, qint64 size);
    void writeBytes(const QByteArray &buffer);
    void writeInt32(qint32 value);
    void writeUint32(quint32 value);
    void writeInt16(qint16 value);
    void writeUint16(quint16 value);
    void writeInt8(qint8 value);
    void writeUint8(quint8 value);
    void writeFloat32(float value);
    void writeChar(char value);
    void writeString(const QByteArray &string);

    IOAdapter &ioAdapter;
    BgzfWriter writer;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_WRITER_H_
