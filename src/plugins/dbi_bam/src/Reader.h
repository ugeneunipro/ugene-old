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

#ifndef _U2_BAM_READER_H_
#define _U2_BAM_READER_H_

#include "Header.h"
#include "Alignment.h"
#include "BgzfReader.h"

namespace U2 {
namespace BAM {

class Reader
{
public:

    class AlignmentReader {
    public:
        AlignmentReader(Reader* reader, int id, int blockSize);
        int getId();
        Alignment read();
        void skip();
    private:
        int id;
        int blockSize;
        Reader* r;
    };

    Reader(IOAdapter &ioAdapter);
    const Header &getHeader()const;
    Alignment readAlignment();
    AlignmentReader getAlignmentReader();
    bool isEof()const;
    VirtualOffset getOffset()const;
    void seek(VirtualOffset offset);
private:
    void readBytes(char *buffer, qint64 size);
    QByteArray readBytes(qint64 size);
    qint32 readInt32();
    quint32 readUint32();
    qint16 readInt16();
    quint16 readUint16();
    qint8 readInt8();
    quint8 readUint8();
    float readFloat32();
    char readChar();
    QByteArray readString();
    void readHeader();

    Header header;
    QHash<QByteArray, int> referencesMap;
    QHash<QByteArray, int> readGroupsMap;
    QHash<QByteArray, int> programsMap;
    IOAdapter &ioAdapter;
    BgzfReader reader;
friend class AlignmentReader;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_READER_H_
