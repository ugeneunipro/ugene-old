#include <memory>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>
#include "IOException.h"
#include "Reader.h"
#include "Dbi.h"
#include "BAMDbiPlugin.h"
#include "ConvertToSQLiteTask.h"

namespace U2 {
namespace BAM {

ConvertToSQLiteTask::ConvertToSQLiteTask(const GUrl &sourceUrl, const QList<GUrl> referenceUrls, const GUrl &destinationUrl):
    Task("Convert BAM to SQLite", TaskFlag_None),
    sourceUrl(sourceUrl),
    referenceUrls(referenceUrls),
    destinationUrl(destinationUrl)
{
    tpm = Progress_Manual;
}

void ConvertToSQLiteTask::run() {
    try {
        if(!destinationUrl.isLocalFile()) {
            throw Exception(BAMDbiPlugin::tr("Non-local files are not supported"));
        }
        std::auto_ptr<IOAdapter> ioAdapter;
        {
            IOAdapterFactory *factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(sourceUrl));
            ioAdapter.reset(factory->createIOAdapter());
        }
        if(!ioAdapter->open(sourceUrl, IOAdapterMode_Read)) {
            throw IOException(BAMDbiPlugin::tr("Can't open file '%1'").arg(sourceUrl.getURLString()));
        }
        std::auto_ptr<Reader> reader(new Reader(*ioAdapter));

        assert(destinationUrl.isLocalFile());
        if(QFile::exists(destinationUrl.getURLString())) {
            QFile::remove(destinationUrl.getURLString());
        }
        std::auto_ptr<U2Dbi> sqliteDbi(AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi")->createDbi());
        {
            QHash<QString, QString> properties;
            properties["url"] = destinationUrl.getURLString();
            properties["create"] = "1";
            U2OpStatusImpl opStatus;
            sqliteDbi->init(properties, QVariantMap(), opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        {
            U2OpStatusImpl opStatus;
            sqliteDbi->getFolderDbi()->createFolder("/", opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        QList<U2Assembly> assemblies;
        foreach(const Header::Reference &reference, reader->getHeader().getReferences()) {
            Q_UNUSED(reference);
            U2Assembly assembly;
            {
                U2OpStatusImpl opStatus;
                sqliteDbi->getAssemblyRWDbi()->createAssemblyObject(assembly, "/", NULL, opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
            }
            assemblies.append(assembly);
        }
        static const int FIRST_STAGE_PERCENT = 60;
        static const int SECOND_STAGE_PERCENT = 40;
        while(!reader->isEof()) {
            QList<QList<U2AssemblyRead> > reads;
            for(int index = 0;index < assemblies.size();index++) {
                reads.append(QList<U2AssemblyRead>());
            }
            {
                int readsCount = 0;
                while(!reader->isEof()) {
                    Alignment alignment = reader->readAlignment();
                    if(-1 != alignment.getReferenceId()) {
                        reads[alignment.getReferenceId()].append(AssemblyRDbi::alignmentToRead(alignment));
                        readsCount++;
                    }
                    if(readsCount >= 16384) {
                        break;
                    }
                }
            }
            for(int index = 0;index < assemblies.size();index++) {
                if(!reads[index].isEmpty()) {
                    U2OpStatusImpl opStatus;
                    sqliteDbi->getAssemblyRWDbi()->addReads(assemblies[index].id, reads[index], opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
                }
            }
            if(isCanceled()) {
                throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
            }
            stateInfo.progress = ioAdapter->getProgress()*FIRST_STAGE_PERCENT/100;
        }
        for(int index = 0;index < assemblies.size();index++) {
            {
                U2OpStatusImpl opStatus;
                sqliteDbi->getAssemblyRWDbi()->pack(assemblies[index].id, opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
            }
            if(isCanceled()) {
                throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
            }
            stateInfo.progress = FIRST_STAGE_PERCENT + (index + 1)*SECOND_STAGE_PERCENT/assemblies.size();
        }
        {
            U2OpStatusImpl opStatus;
            sqliteDbi->shutdown(opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
    } catch(const Exception &e) {
        setError(e.getMessage());
        assert(destinationUrl.isLocalFile());
        QFile::remove(destinationUrl.getURLString());
    }
}

} // namespace BAM
} // namespace U2
