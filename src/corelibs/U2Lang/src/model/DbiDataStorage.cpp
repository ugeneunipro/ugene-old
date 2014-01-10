/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <QtCore/QFile>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2FeatureDbi.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2VariantDbi.h>
#include <U2Core/UserApplicationsSettings.h>

#include "DbiDataStorage.h"

namespace U2 {

namespace Workflow {

DbiDataStorage::DbiDataStorage()
: dbiHandle(NULL)
{

}

DbiDataStorage::~DbiDataStorage() {
    foreach (const U2DbiId &id, connections.keys()) {
        DbiConnection *connection = connections[id];
        delete connection;
    }
    foreach (const U2DbiId &dbiId, dbiList.keys()) {
        bool temporary = dbiList.value(dbiId);
        if (temporary) {
            if (QFile::exists(dbiId)) {
                QFile::remove(dbiId);
            }
        }
    }
    delete dbiHandle;
}

bool DbiDataStorage::init() {
    U2OpStatusImpl os;
    dbiHandle = new TmpDbiHandle(WORKFLOW_SESSION_TMP_DBI_ALIAS, os);
    CHECK_OP(os, false);

    QScopedPointer<DbiConnection> connection(new DbiConnection(dbiHandle->getDbiRef(), os));
    CHECK_OP(os, false);

    connections[dbiHandle->getDbiRef().dbiId] = connection.take();
    return true;
}

U2DbiRef DbiDataStorage::getDbiRef() {
    assert(NULL != dbiHandle);
    return dbiHandle->getDbiRef();
}

U2Object *DbiDataStorage::getObject(const SharedDbiDataHandler &handler, const U2DataType &type) {
    assert(NULL != dbiHandle);
    U2OpStatusImpl os;
    const U2DataId &objectId = handler->entRef.entityId;
    DbiConnection *connection = this->getConnection(handler->getDbiRef(), os);
    CHECK_OP(os, NULL);

    //if (U2Type::Sequence == type) {
    if (1 == type) {
        U2SequenceDbi *dbi = connection->dbi->getSequenceDbi();
        U2Sequence seq = dbi->getSequenceObject(objectId, os);
        SAFE_POINT_OP(os, NULL);

        return new U2Sequence(seq);
    } else if (2 == type) {
        U2MsaDbi *dbi = connection->dbi->getMsaDbi();
        U2Msa msa = dbi->getMsaObject(objectId, os);
        SAFE_POINT_OP(os, NULL);

        return new U2Msa(msa);
    //} else if (U2Type::VariantTrack == type) {
    } else if (5 == type) {
        U2VariantDbi *dbi = connection->dbi->getVariantDbi();
        U2VariantTrack track = dbi->getVariantTrack(objectId, os);
        SAFE_POINT_OP(os, NULL);

        return new U2VariantTrack(track);
    //} else if (U2Type::Assembly == type) {
    } else if (4 == type) {
        U2AssemblyDbi *dbi = connection->dbi->getAssemblyDbi();
        U2Assembly assembly = dbi->getAssemblyObject(objectId, os);
        SAFE_POINT_OP(os, NULL);

        return new U2Assembly(assembly);
    //} else if (U2Type::AnnotationTable == type) {
    } else if ( 10 == type ) {
        U2FeatureDbi *dbi = connection->dbi->getFeatureDbi();
        U2AnnotationTable annTable = dbi->getAnnotationTableObject(objectId, os);
        SAFE_POINT_OP(os, NULL);

        return new U2AnnotationTable(annTable);
    } else {
        assert(0);
    }
    return NULL;
}

SharedDbiDataHandler DbiDataStorage::putSequence(const DNASequence &dnaSeq) {
    assert(NULL != dbiHandle);
    
    U2OpStatusImpl os;
    U2EntityRef ent = U2SequenceUtils::import(dbiHandle->getDbiRef(), dnaSeq, os);
    CHECK_OP(os, SharedDbiDataHandler());

    DbiConnection *connection = this->getConnection(dbiHandle->getDbiRef(), os);
    CHECK_OP(os, SharedDbiDataHandler());

    SharedDbiDataHandler handler(new DbiDataHandler(ent, connection->dbi->getObjectDbi(), true));
    
    return handler;
}

SharedDbiDataHandler DbiDataStorage::putAlignment(const MAlignment &al) {
    assert(NULL != dbiHandle);

    U2OpStatus2Log os;
    U2EntityRef ent = MAlignmentImporter::createAlignment(dbiHandle->getDbiRef(), al, os);
    CHECK_OP(os, SharedDbiDataHandler());

    DbiConnection *connection = this->getConnection(dbiHandle->getDbiRef(), os);
    CHECK_OP(os, SharedDbiDataHandler());

    SharedDbiDataHandler handler(new DbiDataHandler(ent, connection->dbi->getObjectDbi(), true));

    return handler;
}

SharedDbiDataHandler DbiDataStorage::putAnnotationTable( const QList<AnnotationData> &anns ) {
    SAFE_POINT( NULL != dbiHandle, "Invalid DBI handle!", SharedDbiDataHandler( ) );

    AnnotationTableObject obj( "Annotations", dbiHandle->getDbiRef( ) );
    U2EntityRef ent = obj.getEntityRef( );

    U2OpStatusImpl os;
    DbiConnection *connection = this->getConnection( dbiHandle->getDbiRef( ), os );
    SAFE_POINT_OP( os, SharedDbiDataHandler( ) );

    SharedDbiDataHandler handler( new DbiDataHandler( ent, connection->dbi->getObjectDbi( ),
        true ) );

    return handler;
}

bool DbiDataStorage::deleteObject(const U2DataId &, const U2DataType &) {
    assert(NULL != dbiHandle);
    return true;
}

SharedDbiDataHandler DbiDataStorage::getDataHandler(const U2EntityRef &entRef, bool useGC) {
    U2OpStatusImpl os;
    DbiConnection *connection = this->getConnection(entRef.dbiRef, os);
    CHECK_OP(os, SharedDbiDataHandler());

    DbiDataHandler *handler = new DbiDataHandler(entRef, connection->dbi->getObjectDbi(), useGC);

    return SharedDbiDataHandler(handler);
}

DbiConnection *DbiDataStorage::getConnection(const U2DbiRef &dbiRef, U2OpStatus &os) {
    // TODO: mutex
    if (connections.contains(dbiRef.dbiId)) {
        return connections[dbiRef.dbiId];
    } else {
        QScopedPointer<DbiConnection> connection(new DbiConnection(dbiRef, os));
        CHECK_OP(os, NULL);

        connections[dbiRef.dbiId] = connection.data();
        return connection.take();
    }
}

U2DbiRef DbiDataStorage::createTmpDbi(U2OpStatus &os) {
    // TODO: mutex
    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath();

    U2DbiRef dbiRef;
    dbiRef.dbiId = GUrlUtils::prepareTmpFileLocation(tmpDirPath, WORKFLOW_SESSION_TMP_DBI_ALIAS, "ugenedb", os);
    dbiRef.dbiFactoryId = DEFAULT_DBI_ID;
    SAFE_POINT_OP(os, U2DbiRef());

    SAFE_POINT(!dbiList.contains(dbiRef.dbiId),
               QString("Temp dbi already exists: %1").arg(dbiRef.dbiId), dbiRef);

    QScopedPointer<DbiConnection> con(new DbiConnection(dbiRef, true, os));
    SAFE_POINT_OP(os, U2DbiRef());

    dbiList[dbiRef.dbiId] = true;
    connections[dbiRef.dbiId] = con.take();

    return dbiRef;
}

void DbiDataStorage::openDbi(const U2DbiRef &dbiRef, U2OpStatus &os) {
    QScopedPointer<DbiConnection> con(new DbiConnection(dbiRef, false, os));
    CHECK_OP(os, );

    dbiList[dbiRef.dbiId] = false;
    connections[dbiRef.dbiId] = con.take();
}

/************************************************************************/
/* StorageUtils */
/************************************************************************/
U2SequenceObject *StorageUtils::getSequenceObject(DbiDataStorage *storage, const SharedDbiDataHandler &handler) {
    CHECK(NULL != handler.constData(), NULL);
    //QScopedPointer<U2Sequence> seqDbi(dynamic_cast<U2Sequence*>(storage->getObject(handler, U2Type::Sequence)));
    QScopedPointer<U2Sequence> seqDbi(dynamic_cast<U2Sequence*>(storage->getObject(handler, 1)));
    CHECK(NULL != seqDbi.data(), NULL);

    U2EntityRef ent(handler->getDbiRef(), seqDbi->id);
    return new U2SequenceObject(seqDbi->visualName, ent);
}

VariantTrackObject *StorageUtils::getVariantTrackObject(DbiDataStorage *storage, const SharedDbiDataHandler &handler) {
    CHECK(NULL != handler.constData(), NULL);
    //QScopedPointer<U2VariantTrack> track(dynamic_cast<U2VariantTrack*>(storage->getObject(handler, U2Type::VariantTrack)));
    QScopedPointer<U2VariantTrack> track(dynamic_cast<U2VariantTrack*>(storage->getObject(handler, 5)));
    CHECK(NULL != track.data(), NULL);

    U2EntityRef trackRef(handler->getDbiRef(), track->id);
    QString objName = track->sequenceName;

    return new VariantTrackObject(objName, trackRef);
}

AssemblyObject *StorageUtils::getAssemblyObject(DbiDataStorage *storage, const SharedDbiDataHandler &handler) {
    CHECK(NULL != handler.constData(), NULL);
    QScopedPointer<U2Assembly> assembly(dynamic_cast<U2Assembly*>(storage->getObject(handler, 4)));
    CHECK(NULL != assembly.data(), NULL);

    U2EntityRef assemblyRef(handler->getDbiRef(), assembly->id);
    QString objName = assembly->visualName;

    QVariantMap hints;
    return new AssemblyObject(assemblyRef, objName, hints);
}

MAlignmentObject *StorageUtils::getMsaObject(DbiDataStorage *storage, const SharedDbiDataHandler &handler) {
    CHECK(NULL != handler.constData(), NULL);
    QScopedPointer<U2Msa> msa(dynamic_cast<U2Msa*>(storage->getObject(handler, 2)));
    CHECK(NULL != msa.data(), NULL);

    U2EntityRef msaRef(handler->getDbiRef(), msa->id);
    QString objName = msa->visualName;

    return new MAlignmentObject(objName, msaRef);
}

AnnotationTableObject * StorageUtils::getAnnotationTableObject( DbiDataStorage *storage,
    const SharedDbiDataHandler &handler )
{
    CHECK( NULL != handler.constData( ), NULL );
    U2Object *dbObject = storage->getObject( handler, 10 );
    QScopedPointer<U2AnnotationTable> table( dynamic_cast<U2AnnotationTable *>( dbObject ) );
    CHECK( NULL != table.data( ), NULL );

    U2EntityRef tableRef( handler->getDbiRef( ), table->id );
    QString objName = table->visualName;

    return new AnnotationTableObject( objName, tableRef );
}

} // Workflow
} // U2
