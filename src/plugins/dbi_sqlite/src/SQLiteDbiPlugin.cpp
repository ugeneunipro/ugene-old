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

#include "SQLiteDbiPlugin.h"
#include "SQLiteDbi.h"

#include <U2Core/Timer.h>
#include <U2Core/Log.h>
#include <U2Core/AppContext.h>
#include <U2Core/DbiDocumentFormat.h>

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2AssemblyUtils.h>

#include <QtCore/QFile>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    SQLiteDbiPlugin* plug = new SQLiteDbiPlugin();
    return plug;
}

static QList<U2AssemblyRead> generateReads(int n, int rlen, int slen) {
    static const char* acgt = "ACGT";
    QList<U2AssemblyRead> res;
    for (int i = 0; i < n ; i++) {
        U2AssemblyRead r(new U2AssemblyReadData());
        r->cigar << U2CigarToken(U2CigarOp_M, rlen-2) << U2CigarToken(U2CigarOp_I, 1) << U2CigarToken(U2CigarOp_M, 1);
        r->leftmostPos = 0;
        r->readSequence.resize(rlen);
        for (int j = 0; j < rlen; j++) {
            char c =  acgt[qrand() % 4];
            //char c =  'A';
            r->readSequence[j] = c;
        }
        r->name = r->readSequence.mid(0, 15);
        res.append(r);
    }
    return res;
}

//#define TEST_SQLITE_DBI

SQLiteDbiPlugin::SQLiteDbiPlugin() : Plugin(tr("SQLite format support"), tr("Adds support for SQLite format to UGENE")) {
    AppContext::getDbiRegistry()->registerDbiFactory(new SQLiteDbiFactory());
    DbiDocumentFormat* f = new DbiDocumentFormat(SQLiteDbiFactory::ID, "usqlite", tr("UGENE Database"), QStringList()<<"ugenedb" );
    AppContext::getDocumentFormatRegistry()->registerFormat(f);

#ifdef TEST_SQLITE_DBI
    SQLiteDbi dbi;
    U2OpStatusImpl os;
    QHash<QString, QString> props;
    QString url = "c:/test.sqlite";
    QFile::remove(url);
    props[U2_DBI_OPTION_URL] = url;
    //props[U2_DBI_OPTION_URL] = SQLITE_DBI_VALUE_MEMORY_DB_URL;
    //props[SQLITE_DBI_OPTION_ASSEMBLY_READ_COMPRESSION1_FLAG] = U2_DBI_VALUE_ON;
    props[U2_DBI_OPTION_CREATE] = U2_DBI_VALUE_ON;
    //props[SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_KEY] = SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_RTREE;
    //props[SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_KEY] = SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_SINGLE_TABLE;
    props[SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_KEY] = SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_MULTITABLE_V1;
    dbi.init(props, QVariantMap(), os);
    dbi.getDbRef()->useTransaction = true;

    int nReads = 100*1000;
    int seqLen = 1000*1000, rowLen = 50;
    QList<U2AssemblyRead> rows = generateReads(nReads, rowLen, seqLen);
    dbi.getObjectDbi()->createFolder("/", os);
    qint64 t0 = GTimer::currentTimeMicros();
    {
        U2Assembly as;

    //    dbi.getAssemblyRWDbi()->createAssemblyObject(as, "/", NULL, os);
    //    dbi.getAssemblyRWDbi()->addReads(as.id, rows, os);

        BufferedDbiIterator<U2AssemblyRead> it(rows);
        dbi.getAssemblyDbi()->createAssemblyObject(as, "/", &it, os);
    }
    qint64 t1 = GTimer::currentTimeMicros();
    float nSeconds = float((t1-t0)/(1000*1000.0));
    perfLog.info(QString("Rate: %1/second").arg(int(nReads/nSeconds)));
    if (os.hasError()) {
        coreLog.error(os.getError());
    }
   
    dbi.shutdown(os);
#endif

}

}//namespace
