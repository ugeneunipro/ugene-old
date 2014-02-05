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

#ifndef _U2_MEM_BUF_ADAPTER_H_
#define _U2_MEM_BUF_ADAPTER_H_

#include <U2Core/IOAdapter.h>

#include <QtCore/QBuffer>

namespace U2 {

class U2CORE_EXPORT VFSAdapterFactory: public IOAdapterFactory {
    Q_OBJECT
public:
    VFSAdapterFactory(QObject* p = NULL);

    virtual IOAdapter* createIOAdapter();

    virtual IOAdapterId getAdapterId() const {return BaseIOAdapters::VFS_FILE;}

    virtual const QString& getAdapterName() const {return name;}

    virtual bool isIOModeSupported(IOAdapterMode m)  const {Q_UNUSED(m); return true;} //files can be read and be written

    virtual TriState isResourceAvailable(const GUrl& url) const {
        assert(url.isVFSFile());
        Q_UNUSED(url);
        return TriState_Yes;
    }

protected:
    QString name;
};


class U2CORE_EXPORT VFSAdapter: public IOAdapter {
    Q_OBJECT
public:
    VFSAdapter(VFSAdapterFactory* f, QObject* o = NULL);
    ~VFSAdapter() {if (isOpen()) close();}

    virtual bool open(const GUrl& url, IOAdapterMode m);

    virtual bool isOpen() const {return buffer != NULL;}

    virtual void close();

    virtual qint64 readBlock(char* data, qint64 maxSize);

    virtual qint64 writeBlock(const char* data, qint64 size);

    virtual bool skip(qint64 nBytes);

    virtual qint64 left() const;

    virtual int getProgress() const;

    virtual qint64 bytesRead() const;

    virtual GUrl getURL() const {return url;}

    virtual QString errorString() const;

private:
    GUrl        url;
    QBuffer*    buffer;
};


}//namespace

#endif // _U2_MEM_BUF_ADAPTER_H_
