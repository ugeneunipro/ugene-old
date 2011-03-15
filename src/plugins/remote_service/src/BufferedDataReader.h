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

#ifndef _BUFFERED_DATA_READER_H_ 
#define _BUFFERED_DATA_READER_H_

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QBuffer>

#include "Base64File.h"

namespace U2 {

class BufferedDataReader : public QIODevice 
{
public:
    BufferedDataReader(const QStringList& inputUrls, const QByteArray& requestTemplate, const QByteArray& splitMarker); 
    ~BufferedDataReader();
    virtual bool isSequential() const { return true; }
    virtual bool open(OpenMode mode);
    virtual qint64 size() const;
    void setError(const QString& errMsg);
protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char* data, qint64 maxlen);

private:
    QList<QIODevice*>   inputDevs;
    QList<Base64File*>       inputFiles;
    QList<QBuffer*>     inputBufs;
    QList<QByteArray>   buffersData;
    int                 curIdx;
    bool                hasErrors;
    
};

} //namespace

#endif // _BUFFERED_DATA_READER_H_

