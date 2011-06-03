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

#include <QtCore/QFileInfo>
#include <QtCore/QBuffer>

#include "Base64File.h"

#include <assert.h>

namespace U2 {


#define BUFSIZE 8192

Base64File::Base64File( const QString& url )
: file(url)
{
    base64_init_encodestate(&encodeState);
    buf.reserve(BUFSIZE*2);
    bufLen = 0;
    bufOffset = 0;
}

bool Base64File::open( OpenMode mode )
{
    bool ok = file.open(mode);
    if (ok) {
        setOpenMode(mode);
    }

    return ok;
}

qint64 Base64File::size() const
{
    return QFileInfo(file).size() * 4/3;
}


qint64 Base64File::readData( char *data, qint64 maxlen )
{
    qint64 bytesRead = 0;
    
    if (file.atEnd() && bufLen == 0) {
        return -1;
    }

    forever {
        if (bufLen == 0) {
            // fill buffer
            QByteArray fileData;
            fileData.reserve(BUFSIZE);
            bufOffset = 0;
            qint64 len = file.read(fileData.data(), BUFSIZE);
            if (len <= 0) {
                // EOF is reached
                break;
            }
            qint64 codelen = base64_encode_block(fileData.constData(), len, buf.data(), &encodeState);
            bufLen += codelen;
            if (file.atEnd()) {
                bufLen += base64_encode_blockend(buf.data() + codelen, &encodeState);
            }
        } 

        qint64 bytesToRead = maxlen - bytesRead;
        qint64 len = bufLen > bytesToRead ? bytesToRead : bufLen;
        qMemCopy(data + bytesRead, buf.data() + bufOffset, len );
        bufOffset += len;
        bytesRead += len;
        bufLen -= len;
        if (bytesRead == maxlen) {
            break;
        }
        assert(bufLen == 0);
    }

    return bytesRead;  
    
}

qint64 Base64File::writeData( const char* , qint64 )
{
    return -1;
}


} //namespace
