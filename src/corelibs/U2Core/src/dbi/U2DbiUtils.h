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

#ifndef _U2_DBI_UTILS_H_
#define _U2_DBI_UTILS_H_

#include <U2Core/U2Dbi.h>

namespace U2 {

class U2OpStatus;

/** 
    Helper class that allocates connection in constructor and automatically releases it in the destructor 
    It uses app-global connection pool.

    Note: DbiConnection caches U2OpStatus and reuses it in destructor on DBI release. Ensure that 
    U2OpStatus live range contains DbiHandle live range
*/
class U2CORE_EXPORT DbiConnection {
public:
    /** Opens connection to existing DBI */
    DbiConnection(const U2DbiRef& ref,  U2OpStatus& os);
    
    /** Opens connection to existing DBI or create news DBI*/
    DbiConnection(const U2DbiRef& ref,  bool create, U2OpStatus& os);

    DbiConnection(const DbiConnection & dbiConnection_);

    /** Constructs not opened dbi connection */
    DbiConnection();

    ~DbiConnection();

    /** Opens connection to existing DBI */
    void open(const U2DbiRef& ref,  U2OpStatus& os);

    /** Opens connection to existing DBI or create news DBI*/
    void open(const U2DbiRef& ref,  bool create, U2OpStatus& os);
    
    void close(U2OpStatus& os);

    bool isOpen() const {return dbi != NULL;}
    
    U2Dbi*          dbi;

private: //TODO
    DbiConnection & operator=(const DbiConnection & dbiHandle);
};


/** 
    Helper class to track DBI instance live range
*/
class U2CORE_EXPORT TmpDbiHandle {
public:
    TmpDbiHandle(const QString& alias, U2OpStatus& os);
    
    TmpDbiHandle(const U2DbiRef& dbi, bool deallocate);
    
    ~TmpDbiHandle();

    bool isValid() const {return dbiRef.isValid();}
    
    /** DBI reference */
    U2DbiRef            dbiRef;

    /** If true will close all DBI connection in destructor and remove DBI file */
    bool                deallocate;

private: //TODO
    TmpDbiHandle & operator=(const TmpDbiHandle & dbiHandle);
};

class U2CORE_EXPORT TmpDbiObjects {
public:
    TmpDbiObjects(const U2DbiRef& _dbiRef, U2OpStatus& _os) : dbiRef(_dbiRef), os(_os){}
    ~TmpDbiObjects();
    
    U2DbiRef            dbiRef;
    QList<U2DataId>     objects;
    U2OpStatus&         os;
};

/**
    Iterator over buffered data set
*/
template<class T> class BufferedDbiIterator : public U2DbiIterator<T> {
public:
    BufferedDbiIterator(const QList<T>& _buffer, const T& _errValue = T()) : buffer(_buffer), pos(0), errValue(_errValue) {}

    /** returns true if there are more reads to iterate*/
    virtual bool hasNext() {
        return pos < buffer.size();
    }

    /** returns next read and shifts one element*/
    virtual T next() {
        if (!hasNext()) {
            return errValue;
        }
        const T& res = buffer.at(pos);
        pos++;
        return res;
    }

    /** returns next read without shifting*/
    virtual T peek() {
        if (!hasNext()) {
            return errValue;
        }
        return buffer.at(pos);

    }

private:
    QList<T>    buffer;
    int         pos;
    T           errValue;
};


class U2CORE_EXPORT U2DbiUtils : public QObject{
    Q_OBJECT
public:
    /** 
        Logs that operation called is not supported by DBI 
        If U2OpStatus has no error set, sets the error message
    */
    static void logNotSupported(U2DbiFeature f, U2Dbi* dbi, U2OpStatus& os);

    template<class T> static QList<T> toList(U2DbiIterator<T>* it);

    static U2DbiRef toRef(U2Dbi* dbi);
};


template<class T> QList<T> U2DbiUtils::toList(U2DbiIterator<T>* it) {
    QList<T> result;
    while (it->hasNext()) {
        result << it->next();
    }
    return result;
}

}// namespace

#endif
