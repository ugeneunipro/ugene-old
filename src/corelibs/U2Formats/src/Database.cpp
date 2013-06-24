#include "Database.h"

#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/U2Type.h>
#include <U2Formats/SQLiteDbi.h>

#include <QtCore/QFileInfo>

namespace U2 {

#define S3_DATABASE_KEY "s3-database"

Database* Database::loadDatabase(const QString& url, U2OpStatus& os) {

    U2DbiRef dbiRef (SQLiteDbiFactory::ID, url);
    DbiConnection dbHandle (dbiRef, os);
    SAFE_POINT_OP(os, NULL);
  
    // ensure that DBI is a valid Session DBI
    QString val = dbHandle.dbi->getProperty(S3_DATABASE_KEY, "", os);
    SAFE_POINT_OP(os, NULL);

    if (val != U2_DBI_VALUE_ON) {
        os.setError(tr("Not a valid S3-database file: %1").arg(url));
        return NULL;
    }
    
    return new Database(dbHandle);
}

DbiConnection Database::createEmptyDbi(const QString& url, U2OpStatus& os) {
    QFileInfo fi(url);
    if (fi.exists()) {
        os.setError(tr("File already exists: %1").arg(url));
        return DbiConnection();
    }

    U2DbiRef dbiRef (SQLiteDbiFactory::ID, url);
    DbiConnection databaseDbi (dbiRef, true, os);
    SAFE_POINT_OP(os, DbiConnection());

    databaseDbi.dbi->setProperty(S3_DATABASE_KEY, U2_DBI_VALUE_ON, os);
    SAFE_POINT_OP(os, DbiConnection());
    
    return databaseDbi;
}

} //namespace
