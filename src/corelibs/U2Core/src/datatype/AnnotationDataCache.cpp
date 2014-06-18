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

#include <QtCore/QMutexLocker>

#include <U2Core/U2Feature.h>
#include <U2Core/U2SafePoints.h>

#include "AnnotationDataCache.h"

namespace U2 {

static AnnotationData defaultAnnotationReturnValue = AnnotationData();
static U2Feature defaultFeatureReturnValue = U2Feature();

AnnotationDataCache::AnnotationDataCache()
    : guard(QMutex::Recursive)
{

}

void AnnotationDataCache::addData(const U2Feature &feature, const AnnotationData &data) {
    QMutexLocker locker(&guard);

    if (!rootSubfeatures.contains(feature.rootFeatureId)) {
        rootSubfeatures.insert(feature.rootFeatureId, QSet<U2DataId>());
    }
    rootSubfeatures[feature.rootFeatureId].insert(feature.id);
    if (!rootReferenceCount.contains(feature.rootFeatureId)) {
        rootReferenceCount.insert(feature.rootFeatureId, 1);
    }
    annotationDataId.insert(feature.id, data);
    feature2Id.insert(feature.id, feature);
}

bool AnnotationDataCache::containsAnnotationTable(const U2DataId &rootId) {
    QMutexLocker locker(&guard);
    return rootReferenceCount.contains(rootId);
}

bool AnnotationDataCache::contains(const U2DataId &featureId) {
    QMutexLocker locker(&guard);

    return annotationDataId.contains(featureId);
}

int AnnotationDataCache::getAnnotationTableSize(const U2DataId &rootId) {
    QMutexLocker locker(&guard);
    return rootSubfeatures.contains(rootId) ? rootSubfeatures[rootId].size() : 0;
}

AnnotationData & AnnotationDataCache::getAnnotationData(const U2DataId &featureId) {
    QMutexLocker locker(&guard);

    return annotationDataId[featureId];
}

U2Feature & AnnotationDataCache::getFeature(const U2DataId &featureId) {
    QMutexLocker locker(&guard);

    return feature2Id[featureId];
}

QList<U2Feature> AnnotationDataCache::getSubfeatures(const U2DataId &rootId) {
    QMutexLocker locker(&guard);

    QList<U2Feature> result;
    foreach (const U2DataId &featureId, rootSubfeatures.value(rootId)) {
        result.append(feature2Id.value(featureId));
    }
    return result;
}

void AnnotationDataCache::removeAnnotationData(const U2DataId &featureId) {
    QMutexLocker locker(&guard);

    SAFE_POINT(feature2Id.contains(featureId) && annotationDataId.contains(featureId)
        && rootSubfeatures.contains(feature2Id[featureId].rootFeatureId), "Unexpected annotation requested", );

    rootSubfeatures[feature2Id[featureId].rootFeatureId].remove(featureId);
    annotationDataId.remove(featureId);
    feature2Id.remove(featureId);
}

void AnnotationDataCache::removeAnnotationTableData(const U2DataId &rootId) {
    QMutexLocker locker(&guard);

    foreach (const U2DataId &featureId, rootSubfeatures.value(rootId)) {
        removeAnnotationData(featureId);
    }
    rootSubfeatures.remove(rootId);
    rootReferenceCount.remove(rootId);
}

void AnnotationDataCache::refAnnotationTable(const U2DataId &rootId) {
    QMutexLocker locker(&guard);
    const int curRefCount = rootReferenceCount[rootId];
    SAFE_POINT(curRefCount > 0, "Invalid annotation table ref count", );
    rootReferenceCount[rootId] = curRefCount + 1;
}

void AnnotationDataCache::derefAnnotationTable(const U2DataId &rootId) {
    QMutexLocker locker(&guard);
    const int curRefCount = rootReferenceCount[rootId];
    SAFE_POINT(curRefCount > 0, "Invalid annotation table ref count", );
    if (curRefCount > 1) {
        rootReferenceCount[rootId] = curRefCount - 1;
    } else {
        removeAnnotationTableData(rootId);
    }
}

//////////////////////////////////////////////////////////////////////////

DbiAnnotationCache::~DbiAnnotationCache() {
    qDeleteAll(dbiDataCache);
}

void DbiAnnotationCache::addData(const U2DbiRef &dbiRef, const U2Feature &feature, const AnnotationData &data) {
    if (!dbiDataCache.contains(dbiRef)) {
        dbiDataCache.insert(dbiRef, new AnnotationDataCache());
    }

    dbiDataCache[dbiRef]->addData(feature, data);
}

bool DbiAnnotationCache::containsAnnotationTable(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    return dbiDataCache.contains(dbiRef) && dbiDataCache[dbiRef]->containsAnnotationTable(rootId);
}

bool DbiAnnotationCache::contains(const U2DbiRef &dbiRef, const U2DataId &featureId) {
    return dbiDataCache.contains(dbiRef) && dbiDataCache[dbiRef]->contains(featureId);
}

int DbiAnnotationCache::getAnnotationTableSize(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    SAFE_POINT(containsAnnotationTable(dbiRef, rootId), "Unexpected annotation table requested from cache", -1);
    return dbiDataCache[dbiRef]->getAnnotationTableSize(rootId);
}

AnnotationData & DbiAnnotationCache::getAnnotationData(const U2DbiRef &dbiRef, const U2DataId &featureId) {
    SAFE_POINT(contains(dbiRef, featureId), "Unexpected annotation requested from cache", defaultAnnotationReturnValue);
    return dbiDataCache[dbiRef]->getAnnotationData(featureId);
}

U2Feature & DbiAnnotationCache::getFeature(const U2DbiRef &dbiRef, const U2DataId &featureId) {
    SAFE_POINT(contains(dbiRef, featureId), "Unexpected feature requested from cache", defaultFeatureReturnValue);
    return dbiDataCache[dbiRef]->getFeature(featureId);
}

QList<U2Feature> DbiAnnotationCache::getSubfeatures(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    SAFE_POINT(containsAnnotationTable(dbiRef, rootId), "Unexpected annotation table requested from cache", QList<U2Feature>());
    return dbiDataCache[dbiRef]->getSubfeatures(rootId);
}

void DbiAnnotationCache::removeAnnotationData(const U2DbiRef &dbiRef, const U2DataId &featureId) {
    SAFE_POINT(contains(dbiRef, featureId), "Unexpected annotation requested from cache", );
    dbiDataCache[dbiRef]->removeAnnotationData(featureId);
}

void DbiAnnotationCache::removeAnnotationTableData(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    SAFE_POINT(containsAnnotationTable(dbiRef, rootId), "Unexpected annotation table requested from cache", );
    dbiDataCache[dbiRef]->removeAnnotationTableData(rootId);
}

void DbiAnnotationCache::refAnnotationTable(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    SAFE_POINT(containsAnnotationTable(dbiRef, rootId), "Unexpected annotation table requested from cache", );
    dbiDataCache[dbiRef]->refAnnotationTable(rootId);
}

void DbiAnnotationCache::derefAnnotationTable(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    CHECK(containsAnnotationTable(dbiRef, rootId), ); // this is not safe point because empty annotation tables does not have entries in cache
    dbiDataCache[dbiRef]->derefAnnotationTable(rootId);
}

void DbiAnnotationCache::onDbiRelease(const U2DbiRef &dbiRef) {
    if (dbiDataCache.contains(dbiRef)) {
        delete dbiDataCache[dbiRef];
        dbiDataCache.remove(dbiRef);
    }
}

}
