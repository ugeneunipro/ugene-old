/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _BASE64_FILE_H_ 
#define _BASE64_FILE_H_

#include <QtCore/QFile>

extern "C" 
{
#include "base64/cencode.h"
} 

namespace U2 {

class Base64File : public QIODevice 
{
public:
    Base64File(const QString& url); 
    virtual bool isSequential() const { return true; }
    virtual bool open(OpenMode mode);
    virtual qint64 size() const;
    QString fileName() const { return file.fileName(); }
protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char* data, qint64 maxlen);

private:
    QByteArray          buf;
    qint64              bufLen;
    qint64              bufOffset;
    QFile               file;
    base64_encodestate  encodeState; 
};


} // namespace


#endif // _BASE64_FILE_H_
