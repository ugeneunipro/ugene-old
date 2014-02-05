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

#ifndef _U2_DBI_REGISTRY_H_
#define _U2_DBI_REGISTRY_H_

#include <U2Core/U2Dbi.h>

#include <QtCore/QHash>
#include <QtCore/QMutex>

namespace U2 {

class U2DbiPool;
class DbiConnection;

#define SQLITE_DBI_ID "SQLiteDbi"
#define BAM_DBI_ID "SamtoolsBasedDbi"
#define DEFAULT_DBI_ID SQLITE_DBI_ID
#define WORKFLOW_SESSION_TMP_DBI_ALIAS "workflow_session"

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

    /**
    * Increases the "number of users"-counter for the dbi, if it exists.
    * Otherwise, allocates the dbi and sets the counter to 1.
    */
    U2DbiRef attachTmpDbi(const QString& alias, U2OpStatus& os);

    /**
    * Decreases the "number of users"-counter.
    * Deallocates the dbi, if it becomes equal to 0.
    */
    void detachTmpDbi(const QString& alias, U2OpStatus& os);

    QList<U2DbiRef> listTmpDbis() const;

    /**
    * Returns the reference to the tmp session dbi.
    * If the last is not created yet then it would be created.
    */
    U2DbiRef getSessionTmpDbiRef(U2OpStatus &os);

    /**
    * WARNING: must be used only in crash handler.
    * Closes the session database connection and returns the path to the database file
    */
    QString shutdownSessionDbi(U2OpStatus &os);


private:
    /** Creates the session connection and increases the counter for the dbi */
    void initSessionDbi(TmpDbiRef& tmpDbiRef);

    U2DbiRef allocateTmpDbi(const QString& alias, U2OpStatus& os);

    void deallocateTmpDbi(const TmpDbiRef& ref, U2OpStatus& os);

    QHash<U2DbiFactoryId, U2DbiFactory *>   factories;
    U2DbiPool*                              pool;
    QList<TmpDbiRef>                        tmpDbis;
    QMutex                                  lock;

    /** this connection is opened during the whole ugene session*/
    DbiConnection*                          sessionDbiConnection;
    bool                                    sessionDbiInitDone;
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
