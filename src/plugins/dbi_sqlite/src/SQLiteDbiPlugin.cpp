#include "SQLiteDbiPlugin.h"
#include "SQLiteDbi.h"

#include <U2Core/Timer.h>
#include <U2Core/Log.h>
#include <U2Core/AppContext.h>
#include <U2Core/DbiDocumentFormat.h>

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2DbiUtils.h>
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
        U2AssemblyRead r;
        r.cigar << U2CigarToken(U2CigarOp_M, rlen-2) << U2CigarToken(U2CigarOp_I, 1) << U2CigarToken(U2CigarOp_M, 1);
        r.leftmostPos = 0;
        r.readSequence.resize(rlen);
        for (int j = 0; j < rlen; j++) {
            char c =  acgt[qrand() % 4];
            r.readSequence[j] = c;
        }
        
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
    props[SQLITE_DBI_OPTION_URL] = url;
    //props[SQLITE_DBI_OPTION_URL] = SQLITE_DBI_VALUE_MEMORY_DB_URL;
    //props[SQLITE_DBI_OPTION_ASSEMBLY_READ_COMPRESSION1_FLAG] = SQLITE_DBI_VALUE_ON;
    props[SQLITE_DBI_OPTION_CREATE] = SQLITE_DBI_VALUE_ON;
    dbi.init(props, QVariantMap(), os);
    dbi.getDbRef()->useTransaction = true;

    int nReads = 100*1000;
    int seqLen = 1000*1000, rowLen = 50;
    QList<U2AssemblyRead> rows = generateReads(nReads, rowLen, seqLen);
    dbi.getFolderDbi()->createFolder("/", os);
    qint64 t0 = GTimer::currentTimeMicros();
    {
        U2Assembly as;

    //    dbi.getAssemblyRWDbi()->createAssemblyObject(as, "/", NULL, os);
    //    dbi.getAssemblyRWDbi()->addReads(as.id, rows, os);

        U2AssemblyReadsIteratorImpl it(rows);
        dbi.getAssemblyRWDbi()->createAssemblyObject(as, "/", &it, os);
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
