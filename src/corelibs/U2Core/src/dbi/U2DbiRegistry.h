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

#ifndef _U2_DBI_REGISTRY_H_
#define _U2_DBI_REGISTRY_H_

#include <U2Core/U2Dbi.h>

#include <QtCore/QHash>
#include <QtCore/QMutex>

namespace U2 {

class U2DbiPool;

#define SQLITE_DBI_ID "SQLiteDbi"
#define DEFAULT_DBI_ID SQLITE_DBI_ID
#define SESSION_TMP_DBI_ALIAS "session"

/** 
    Keep all DBI types registered in the system
*/
class U2CORE_EXPORT U2DbiRegistry : public QObject {
    Q_OBJECT
    class TmpDbiRef {
    public:
        TmpDbiRef(const QString& _alias = QString(), const U2DbiRef& _dbiRef = U2DbiRef(), int _nUsers = 0)
            : alias(_alias), dbiRef(_dbiRef), nUsers(_nUsers){}

        QString     alias;
        U2DbiRef    dbiRef;
        int         nUsers;
    };
public:
    U2DbiRegistry(QObject *parent = NULL);
    ~U2DbiRegistry();

    virtual bool registerDbiFactory(U2DbiFactory *factory);

    virtual QList<U2DbiFactoryId> getRegisteredDbiFactories() const;

    virtual U2DbiFactory *getDbiFactoryById(U2DbiFactoryId id) const;

    U2DbiPool* getGlobalDbiPool() const {return pool;}

    U2DbiRef allocateTmpDbi(const QString& alias, U2OpStatus& os);

    void deallocateTmpDbi(const U2DbiRef& ref, U2OpStatus& os);

    QList<U2DbiRef> listTmpDbis() const;

private:
    
    QHash<U2DbiFactoryId, U2DbiFactory *>   factories;
    U2DbiPool*                              pool;
    QList<TmpDbiRef>                        tmpDbis;
    QMutex                                  lock;
};

/** 
    Class to access to DBI connections
    Roles: 
        creates new DBIs on user request
        track connection live range (using ref counters)
        closes unused DBIs
*/
class U2CORE_EXPORT U2DbiPool : public QObject {
    Q_OBJECT
public:
    U2DbiPool(QObject* p = NULL);
    
    U2Dbi* openDbi(const U2DbiRef& ref, bool create, U2OpStatus& os);
    void addRef(U2Dbi * dbi, U2OpStatus & os);
    void releaseDbi(U2Dbi* dbi, U2OpStatus& os);
    void closeAllConnections(const QString& url, U2OpStatus& os);

private:
    QHash<QString, U2Dbi*> dbiByUrl;
    QHash<QString, int> dbiCountersByUrl;
    QMutex lock;
};

} // namespace U2

#endif // _U2_DBI_REGISTRY_H_
