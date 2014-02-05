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

#include "BufferedDataReader.h"

#include <assert.h>

namespace U2 {

static QList<QByteArray> splitBuffers(const QByteArray& request, const QByteArray& splitMarker) {
    QList<QByteArray> result;
    static int len = QByteArray(splitMarker).length() ;
    int start = 0;

    for (; ;) {
        int pos = request.indexOf(splitMarker,start);
        if (pos == -1 ) {
            QByteArray buf = request.mid(start);
            assert(buf.startsWith("</data>"));
            result.append(buf);
            break;
        }
        QByteArray buf = request.mid(start, pos - start );
        assert(buf.endsWith(">"));
        result.append(buf);
        start = pos + len;
    }
    
    return result;

}


BufferedDataReader::BufferedDataReader( const QStringList& inputUrls, const QByteArray& requestTemplate, const QByteArray& splitMarker ) 
    : curIdx(0), hasErrors(false)
{
    buffersData = splitBuffers(requestTemplate, splitMarker);

    foreach (const QString& url, inputUrls) {
        inputFiles.append(new Base64File(url));
    }

    for (int i = 0; i < buffersData.count(); ++i) {
        inputBufs.append( new QBuffer(&buffersData[i]));
    }

    assert(buffersData.count() == inputFiles.count() + 1);
    setErrorString("");

}

bool BufferedDataReader::open( OpenMode mode )
{
    if (mode != QIODevice::ReadOnly) {
        setError("Only ReadOnly open mode is supported for device.");
        return false;
    }
  

    if (buffersData.count() != inputFiles.count() + 1) {
        setError("Incorrect buffer splitting.");
        return false;
    }
    
    foreach (Base64File* file, inputFiles) {
        if (!file->open(QIODevice::ReadOnly)) {
            setError(QString("Failed to open file  %").arg(file->fileName()));
            return false;
        }
    }
    
    foreach (QBuffer* buf, inputBufs) {
        if (!buf->open(QIODevice::ReadOnly)) {
            setError(QString("Failed to open buffer of RunRemoteTaskRequest data."));
            return false;
        }
    }

    
    int numBufs = inputBufs.count(); 
    for (int i = 0; i < numBufs; ++i) {
        inputDevs.append(inputBufs[i]);
        if (i + 1 < numBufs ) {
            inputDevs.append(inputFiles[i]);
        }
    }

    setOpenMode(QIODevice::ReadOnly);
    return true;

}

/*
    Each time this function is called, buffer of size=maxlen is prepared.
    The buffer can include data from fragments of request template or from actual
    file data.
    
    After the buffer is prepared it's contents is copied into data.

*/

qint64 BufferedDataReader::readData( char *data, qint64 maxlen )
{
    assert(isOpen());

    QByteArray preparedBuf;
    preparedBuf.reserve(maxlen);
    
    qint64 len = 0;
    char* preparedData = preparedBuf.data();
    const int MAX_INDEX = inputDevs.count() - 1;
    
    if (hasErrors || curIdx > MAX_INDEX) {
        return -1;
    }

    forever {
        QIODevice* dev = inputDevs[curIdx];
        assert(dev->isOpen());
        qint64 bytesToRead = maxlen - len;
        //TODO: support base64 encoding
        qint64 bytesRead = dev->read(preparedData, bytesToRead);
        if (-1 == bytesRead) {
            setError(QString("Failed to read from buffered device."));
            return -1;
        }
        len += bytesRead;
        preparedData += bytesRead;
        
        if (dev->bytesAvailable() == 0 && curIdx == MAX_INDEX ) {
            curIdx++;
            break;
        }
        if (len == maxlen ) {
            break;
        } 
        curIdx++;
    }
    
    qMemCopy(data, preparedBuf.constData(), len);

    return len;
}

qint64 BufferedDataReader::writeData( const char* , qint64 )
{
    assert(0);
    return -1;
}



BufferedDataReader::~BufferedDataReader()
{
    qDeleteAll(inputBufs);
    qDeleteAll(inputFiles);
}

qint64 BufferedDataReader::size() const
{
    qint64 dataLen = 0;

    foreach(QBuffer* buf, inputBufs) {
        dataLen += buf->size();
    }

    foreach(Base64File* file, inputFiles) {
        dataLen += file->size();        
    }
    
    return dataLen + 100;
}

void BufferedDataReader::setError( const QString& errMsg )
{
    hasErrors = true;
    setErrorString(errMsg);
}



} //namespace
