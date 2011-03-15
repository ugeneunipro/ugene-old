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

#include "LocalFileAdapter.h"
#include "ZlibAdapter.h"

namespace U2 {

LocalFileAdapterFactory::LocalFileAdapterFactory(QObject* o) : IOAdapterFactory(o) {
    name = tr("Local file");
}

IOAdapter* LocalFileAdapterFactory::createIOAdapter() {
    return new LocalFileAdapter(this);
}

GzippedLocalFileAdapterFactory::GzippedLocalFileAdapterFactory(QObject* o) 
: LocalFileAdapterFactory(o) {
    name = tr("GZIP file");
}

IOAdapter* GzippedLocalFileAdapterFactory::createIOAdapter() {
    return new ZlibAdapter(new LocalFileAdapter(this));
}

LocalFileAdapter::LocalFileAdapter(LocalFileAdapterFactory* factory, QObject* o) : IOAdapter(factory, o), f(NULL) {
}


bool LocalFileAdapter::open(const GUrl& url, IOAdapterMode m) {
    assert(!isOpen());
    assert(f==NULL);
    if (url.isEmpty()) {
        return false;
    }
    f = new QFile(url.getURLString());
    QIODevice::OpenMode iomode;
    switch (m) {
        case IOAdapterMode_Read: iomode = QIODevice::ReadOnly; break;
        case IOAdapterMode_Write: iomode = QIODevice::WriteOnly | QIODevice::Truncate; break;
        case IOAdapterMode_Append: iomode = QIODevice::WriteOnly | QIODevice::Append; break;
    }
    bool res = f->open(iomode);
    if (!res) {
        delete f;
        f = NULL;
        return false;
    }
    return true;
}

void LocalFileAdapter::close() {
    assert(isOpen());
    if (!isOpen()) {
        return;
    }
    f->close();
    delete f;
    f = NULL;
}

qint64 LocalFileAdapter::readBlock(char* data, qint64 size) {
    assert(isOpen());
    qint64 l = f->read(data, size);
    return l;
}

qint64 LocalFileAdapter::writeBlock(const char* data, qint64 size) {
    assert(isOpen());
    qint64 l = f->write(data, size);
    return l;
}

bool LocalFileAdapter::skip(qint64 nBytes) {
    assert(isOpen());
    if (!isOpen()) {
        return false;
    }
    qint64 p = f->pos();
    return f->seek(p+nBytes);
}

qint64 LocalFileAdapter::left() const {
    assert(isOpen());
    qint64 p = f->pos();
    qint64 len = f->size();
    return len - p;
}

int LocalFileAdapter::getProgress() const {
    assert(isOpen());
    return int(100 * float(f->pos()) / f->size());
}

qint64 LocalFileAdapter::bytesRead() const {
    return f->pos();
}

GUrl LocalFileAdapter::getURL() const {
    return GUrl(f->fileName(), GUrl_File);
}

};//namespace
