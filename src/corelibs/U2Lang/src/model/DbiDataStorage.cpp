/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2VariantDbi.h>

#include "DbiDataStorage.h"

namespace U2 {

namespace Workflow {

DbiDataStorage::DbiDataStorage()
: dbiHandle(NULL) , connection(NULL) {

}

DbiDataStorage::~DbiDataStorage() {
    delete connection;
    delete dbiHandle;
}

bool DbiDataStorage::init() {
    U2OpStatusImpl os;
    dbiHandle = new TmpDbiHandle(WORKFLOW_SESSION_TMP_DBI_ALIAS, os);
    CHECK_OP(os, false);

    connection = new DbiConnection(dbiHandle->getDbiRef(), os);
    CHECK_OP(os, false);

    return true;
}

U2DbiRef DbiDataStorage::getDbiRef() {
    assert(NULL != dbiHandle);
    return dbiHandle->getDbiRef();
}

U2Object *DbiDataStorage::getObject(const SharedDbiDataHandler &handler, const U2DataType &type) {
    assert(NULL != dbiHandle);
    U2OpStatusImpl os;
    const U2DataId &objectId = handler->id;

    //if (U2Type::Sequence == type) {
    if (1 == type) {
        U2SequenceDbi *dbi = connection->dbi->getSequenceDbi();
        U2Sequence seq = dbi->getSequenceObject(objectId, os);
        SAFE_POINT_OP(os, NULL);

        return new U2Sequence(seq);
    //} else if (U2Type::VariantTrack == type) {
    } else if (5 == type) {
        U2VariantDbi *dbi = connection->dbi->getVariantDbi();
        U2VariantTrack track = dbi->getVariantTrack(objectId, os);
        SAFE_POINT_OP(os, NULL);

        return new U2VariantTrack(track);
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

    SharedDbiDataHandler handler(new DbiDataHandler(ent.entityId, connection->dbi->getObjectDbi()));
    
    return handler;
}

bool DbiDataStorage::deleteObject(const U2DataId &, const U2DataType &) {
    assert(NULL != dbiHandle);
    return true;
}

SharedDbiDataHandler DbiDataStorage::getDataHandler(const U2DataId &id) {
    DbiDataHandler *handler = new DbiDataHandler(id, connection->dbi->getObjectDbi());

    return SharedDbiDataHandler(handler);
}

U2SequenceObject *StorageUtils::getSequenceObject(DbiDataStorage *storage, const SharedDbiDataHandler &handler) {
    if(handler.constData() == NULL){
        return NULL;
    }
    //QScopedPointer<U2Sequence> seqDbi(dynamic_cast<U2Sequence*>(storage->getObject(handler, U2Type::Sequence)));
    QScopedPointer<U2Sequence> seqDbi(dynamic_cast<U2Sequence*>(storage->getObject(handler, 1)));
    CHECK(NULL != seqDbi.data(), NULL);

    U2EntityRef ent(storage->getDbiRef(), seqDbi->id);
    return new U2SequenceObject(seqDbi->visualName, ent);
}

VariantTrackObject *StorageUtils::getVariantTrackObject(DbiDataStorage *storage, const SharedDbiDataHandler &handler) {
    if(handler.constData() == NULL){
        return NULL;
    }
    //QScopedPointer<U2VariantTrack> track(dynamic_cast<U2VariantTrack*>(storage->getObject(handler, U2Type::VariantTrack)));
    QScopedPointer<U2VariantTrack> track(dynamic_cast<U2VariantTrack*>(storage->getObject(handler, 5)));
    CHECK(NULL != track.data(), NULL);

    U2EntityRef trackRef(storage->getDbiRef(), track->id);
    QString objName = track->sequenceName;

    return new VariantTrackObject(objName, trackRef);
}

} // Workflow
} // U2
