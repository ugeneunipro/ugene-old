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

#include "IOAdapterUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/GUrl.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/L10n.h>

namespace U2 {

IOAdapterId IOAdapterUtils::url2io(const GUrl& url) {
    if( url.isVFSFile() ) {
        return BaseIOAdapters::VFS_FILE;
    }
    if( url.isHyperLink() ) {
        if( url.lastFileSuffix() == "gz") {
            return BaseIOAdapters::GZIPPED_HTTP_FILE;
        }
        return BaseIOAdapters::HTTP_FILE;
    }
    if( url.lastFileSuffix() == "gz") {
        return BaseIOAdapters::GZIPPED_LOCAL_FILE;
    }
    return BaseIOAdapters::LOCAL_FILE;
}

QByteArray IOAdapterUtils::readFileHeader(const GUrl& url, int size) {
    QByteArray data;
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    QScopedPointer<IOAdapter> adapter(iof->createIOAdapter());
    bool res = adapter->open(url, IOAdapterMode_Read);
    if (!res) {
        return data;//BUG:420: report error
    }

    data.resize(size);

    int s = adapter->readBlock(data.data(), data.size());
    if (s == -1) {
        data.resize(0);
        return data;//BUG:420: report error
    }

    if (s != data.size()) {
        data.resize(s);
    }
    return data;
}

QByteArray IOAdapterUtils::readFileHeader( IOAdapter* io, int sz ) {
    QByteArray data;
    if( NULL == io || !io->isOpen() ) {
        return data;
    }
    data.resize( sz );
    int ret = io->readBlock( data.data(), sz );
    if( -1 == ret ) {
        data.resize( 0 );
        return data;
    }
    if( ret != sz ) {
        data.resize( ret );
    }
    io->skip( -ret );
    return data;
}

IOAdapter* IOAdapterUtils::open(const GUrl& url, U2OpStatus& os, IOAdapterMode mode) {
    IOAdapterId  ioId = IOAdapterUtils::url2io(url);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(ioId);
    if (iof == NULL) {
        os.setError(L10N::tr("Failed to detect IO adapter for %1").arg(url.getURLString()));
        return NULL;
    }
    IOAdapter* io = iof->createIOAdapter();
    SAFE_POINT(io != NULL, "IO adapter is NULL!", NULL);
    
    bool ok = io->open(url, mode);
    if (!ok) {
        os.setError(L10N::tr("Failed to detect IO adapter for %1").arg(url.getURLString()));
        delete io;
        return NULL;
    }
    return io;
}

IOAdapterFactory* IOAdapterUtils::get(const IOAdapterId& id) {
    return AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(id);
}


} //namespace
