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

#include "HttpFileAdapter.h"
#include "ZlibAdapter.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QUrl>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

#include <QtNetwork/QHttp>
#include <QtNetwork/QNetworkProxy>

namespace U2 {

#define MAX_GET_LENGTH 3000

HttpFileAdapterFactory::HttpFileAdapterFactory(QObject* o) : IOAdapterFactory(o) {
    name = tr("HTTP file");
}

IOAdapter* HttpFileAdapterFactory::createIOAdapter() {
    return new HttpFileAdapter(this);
}

GzippedHttpFileAdapterFactory::GzippedHttpFileAdapterFactory( QObject * obj /* = 0 */ )  :
HttpFileAdapterFactory( obj ) {    
    name = tr("HTTP GZIP adaptor");
}

IOAdapter * GzippedHttpFileAdapterFactory::createIOAdapter() {
    return new ZlibAdapter( new HttpFileAdapter(this) );
}

QNetworkProxy HttpFileAdapterFactory::getProxyByUrl( const QUrl & url ) const { 
    NetworkConfiguration* nc = AppContext::getAppSettings()->getNetworkConfiguration();
    return nc->getProxyByUrl(url); 
}



HttpFileAdapter::HttpFileAdapter(HttpFileAdapterFactory* factory, QObject* o) 
: IOAdapter(factory, o), is_cached(false), begin_ptr(-1), end_ptr(0), 
http(0), badstate(false), is_downloaded(false), downloaded(0), total(0)
{
    chunk_list.push_back( QByteArray(CHUNKSIZE, char(0)) );
}

bool HttpFileAdapter::open(const GUrl& url_, IOAdapterMode m) {
    SAFE_POINT (m == IOAdapterMode_Read, QString("Illegal IO mode: %1").arg(m), false);

    QUrl url( url_.getURLString().trimmed() );
    if( !url.isValid() ) {
        return false;
    }
    bool https = url.scheme() == "https";
    if( !url.scheme().isEmpty() && !https && url.scheme() != "http" ) {
        return false;
    }
    gurl = url_;
    init();

    HttpFileAdapterFactory * f = qobject_cast<HttpFileAdapterFactory*> ( getFactory() ); 
    QNetworkProxy proxy = f->getProxyByUrl(url);

    return open( url.host(), url.path() + "?" + QString(url.encodedQuery()), proxy, 
        -1 == url.port() ? (https ? 443:80) : url.port(), https );
}

bool HttpFileAdapter::open( const QString& host, const QString & what, const QNetworkProxy & p, quint16 port, bool https )
{
    SAFE_POINT(!isOpen(), "Adapter is already opened!", false);

    if( http ) {
        close();
    }
    http = new QHttp( host,  https ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp, port );

    //printf( "using proxy: %s on port %d\n", p.hostName().toStdString().c_str(), p.port() );
    http->setProxy( p );
    
    connect( http, SIGNAL(dataReadProgress(int,int)), this, SLOT(progress(int,int)), Qt::DirectConnection );
//    connect( http, SIGNAL(stateChanged(int)), this, SLOT(state(int)), Qt::DirectConnection );
    connect( http, SIGNAL(done(bool)), this, SLOT(done(bool)), Qt::DirectConnection );
    connect( http, SIGNAL(readyRead(const QHttpResponseHeader&)), this, SLOT(add_data(const QHttpResponseHeader&)), Qt::DirectConnection );
    
    if(what.length()>MAX_GET_LENGTH) {
        QString path = what.split("?").first();
        QByteArray params = what.split("?").last().toAscii();
        http->post(path,params);
    }
    else {
        http->get( what );
    }

    return true;
}

void HttpFileAdapter::close() {
    SAFE_POINT(isOpen(), "Adapter is not opened!", );

    if (!isOpen()) {
        return;
    }
    assert( http );
    http->abort();
    delete http;
    http = 0;
    gurl = GUrl();
    init();
}

qint64 HttpFileAdapter::readBlock(char* data, qint64 size) 
{
    SAFE_POINT(isOpen(), "Adapter is not opened!", 0);
    if( badstate ) {
        return -1;
    }
    assert( data );
    assert( CHUNKSIZE != end_ptr );
    size = waitData( size );

    if( badstate ) {
        return -1;
    }

    rwmut.lock();
    qint64 write_offs = 0;
    while( write_offs < size ) {
        int howmuch = qMin( size - write_offs, (qint64)firstChunkContains() );
        readFromChunk( data + write_offs, howmuch );
        write_offs += howmuch;
    }

    rwmut.unlock();
    return size;
}

qint64 HttpFileAdapter::writeBlock(const char* , qint64) {
    SAFE_POINT(0, "Operation is not supported!",0);
    return 0;
}

bool HttpFileAdapter::skip(qint64 nBytes) {
    SAFE_POINT(isOpen(), "Adapter is not opened!", false);

    nBytes = waitData( nBytes );
    rwmut.lock();

    if( nBytes < 0 ) {
        assert( -nBytes <= CHUNKSIZE );
        if( begin_ptr >= -nBytes ) {
            begin_ptr += nBytes;
        } else {
            if( !is_cached ) {
                rwmut.unlock();
                return false;
            }
            chunk_list.prepend( cache );
            begin_ptr = CHUNKSIZE - (-nBytes - begin_ptr);
            is_cached = false;
        }
    } else {
        rwmut.unlock();
        return nBytes == skipAhead( nBytes );
    }
    rwmut.unlock();
    return true;
}

qint64 HttpFileAdapter::left() const {
    return (is_downloaded ? stored() : -1);
}

void HttpFileAdapter::init() {
    badstate = false;
    is_downloaded = false;
    is_cached= false;
    chunk_list.clear();
    chunk_list.append( QByteArray(CHUNKSIZE, 0) );
    loop.exit();
}

void HttpFileAdapter::add_data( const QHttpResponseHeader & )
{
    rwmut.lock();
    if( isEmpty() ) {
        begin_ptr = 0;
    }
    int howmuch = CHUNKSIZE - end_ptr; 
    int got = http->read( chunk_list.last().data() + end_ptr, howmuch );

    if( got < 0 ) {
        badstate = true;
    }
    else if( got < howmuch ) {
        end_ptr += got;
    }
    else {
        do {
            QByteArray newchunck( CHUNKSIZE, (char)0 );    
            got = http->read( newchunck.data(), CHUNKSIZE );

            if( got < 0  ) {
                badstate = true;
                break;
            }
            chunk_list.append( newchunck );
        } while( got == CHUNKSIZE );
        end_ptr = got;
    }
//    cond.wakeOne();
    loop.exit();
    rwmut.unlock();
}

qint64 HttpFileAdapter::stored() const
{
    return (singleChunk() ? firstChunkContains() : 
           firstChunkContains() + end_ptr + (chunk_list.size()-2) * CHUNKSIZE );
}

void HttpFileAdapter::readFromChunk( char * data, int size )
{
    assert( size <= firstChunkContains() );
    assert( !isEmpty() );
    
    memcpy( data, chunk_list.first().data() + begin_ptr, size );
    if( begin_ptr + size == CHUNKSIZE) {
        popFront();
    } else {
        begin_ptr += size;
    }
}

void HttpFileAdapter::skipFromChunk( qint64 nbytes )
{
    assert( nbytes <= firstChunkContains() );
    assert( !isEmpty() );

    if( begin_ptr + nbytes == CHUNKSIZE) {
        popFront();
    } else {
        begin_ptr += nbytes;
    }
}

void HttpFileAdapter::popFront()
{
    if( singleChunk() ) {
        cache = chunk_list.first();
        Empty();
    } else {
        begin_ptr = 0;
        cache = chunk_list.takeFirst();
        is_cached = true;
    }
}

qint64 HttpFileAdapter::skipAhead( qint64 nBytes )
{
    assert( nBytes >= 0 );
    nBytes = qMin( nBytes, stored() );
    qint64 skip_offs = 0;
    while( skip_offs < nBytes ) {
        int howmuch = qMin( nBytes - skip_offs, (qint64)firstChunkContains() );
        skipFromChunk( howmuch );
        skip_offs += howmuch;
    }
    return nBytes;
}

qint64 HttpFileAdapter::waitData( qint64 until )
{
    while (!is_downloaded && (until > stored()) ) {
        loop.exec();
    }
    return qMin( until, stored() );
}

void HttpFileAdapter::done( bool error )
{
    is_downloaded = true;
    badstate = error;
    loop.exit();
}

int HttpFileAdapter::getProgress() const
{
    return (total ? (int)( 100 * (float)downloaded / total ) : -1);
}

void HttpFileAdapter::state( int state ) {
    Q_UNUSED(state);
//    printf("downloading state: %d\n", state);
}

void HttpFileAdapter::progress( int done_, int total_ )
{
    downloaded = done_;
    total = total_;
}

}//namespace
