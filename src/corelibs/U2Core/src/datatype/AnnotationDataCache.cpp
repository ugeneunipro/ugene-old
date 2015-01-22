/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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
    SAFE_POINT(U2Feature::Annotation == feature.type, "Invalid feature type", );

    QMutexLocker locker(&guard);
    addFeature(feature);
    annotationDataId.insert(feature.id, data);
}

void AnnotationDataCache::addGroup(const U2Feature &feature) {
    SAFE_POINT(U2Feature::Group == feature.type, "Invalid feature type", );
    addFeature(feature);
}

void AnnotationDataCache::addFeature(const U2Feature &feature) {
    QMutexLocker locker(&guard);

    if (feature.rootFeatureId.isEmpty()) { // then it's a root feature itself
        SAFE_POINT(!rootSubfeatures.contains(feature.id), "Unexpected feature ID", );
        SAFE_POINT(!rootReferenceCount.contains(feature.id), "Unexpected feature ID", );

        rootSubfeatures.insert(feature.id, QList<U2DataId>());
        rootReferenceCount.insert(feature.id, 0);
        return;
    }

    if (!rootSubfeatures.contains(feature.rootFeatureId)) {
        rootSubfeatures.insert(feature.rootFeatureId, QList<U2DataId>());
    }
    rootSubfeatures[feature.rootFeatureId].append(feature.id);
    if (!rootReferenceCount.contains(feature.rootFeatureId)) {
        rootReferenceCount.insert(feature.rootFeatureId, 0);
    }
    feature2Id.insert(feature.id, feature);
}

void AnnotationDataCache::addAnnotationTable(const U2DataId &rootId) {
    QMutexLocker locker(&guard);

    SAFE_POINT(!rootSubfeatures.contains(rootId) && !rootReferenceCount.contains(rootId), "Annotation table is already cached", );
    rootSubfeatures.insert(rootId, QList<U2DataId>());
    rootReferenceCount.insert(rootId, 0);
}

bool AnnotationDataCache::containsAnnotationTable(const U2DataId &rootId) {
    QMutexLocker locker(&guard);
    return rootReferenceCount.contains(rootId);
}

bool AnnotationDataCache::containsAnnotation(const U2DataId &featureId) {
    QMutexLocker locker(&guard);
    return annotationDataId.contains(featureId);
}

bool AnnotationDataCache::containsGroup(const U2DataId &featureId) {
    QMutexLocker locker(&guard);
    return feature2Id.contains(featureId) && feature2Id[featureId].type == U2Feature::Group;
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

namespace {

bool annotationIntersectsRange(const AnnotationData &ad, const U2Region &range, bool contains) {
    if (!contains) {
        foreach(const U2Region &r, ad.getRegions()) {
            if (r.intersects(range)) {
                return true;
            }
        }
        return false;
    } else {
        foreach(const U2Region &r, ad.getRegions()) {
            if (!range.contains(r)) {
                return false;
            }
        }
        return true;
    }
}

bool featureLessThan(const U2Feature &first, const U2Feature &second) {
    return first.location.region.startPos < second.location.region.startPos;
}

}

QList<U2Feature> AnnotationDataCache::getTableFeatures(const U2DataId &rootId, const FeatureFlags &type) {
    QMutexLocker locker(&guard);

    QList<U2Feature> result;
    foreach (const U2DataId &featureId, rootSubfeatures.value(rootId)) {
        const U2Feature subfeature = feature2Id.value(featureId);
        if (type & subfeature.type) {
            result.append(subfeature);
        }
    }

    std::stable_sort(result.begin(), result.end(), featureLessThan);
    return result;
}

QList<U2Feature> AnnotationDataCache::getSubfeatures(const U2DataId &parentId, const FeatureFlags &type, OperationScope scope,
    SubfeatureSelectionMode mode)
{
    QList<U2Feature> result;

    QMutexLocker locker(&guard);
    SAFE_POINT(feature2Id.contains(parentId), "Unexpected parent feature provided", result);

    const U2Feature parentFeature = feature2Id.value(parentId);
    if (mode == SelectParentFeature) {
        result.append(parentFeature);
    }

    QList<U2Feature> subfeatures;
    foreach (const U2DataId &featureId, rootSubfeatures.value(parentFeature.rootFeatureId)) {
        const U2Feature subfeature = feature2Id.value(featureId);
        if (subfeature.parentFeatureId == parentId) {
            subfeatures.append(subfeature);
            if (type & subfeature.type) {
                result.append(subfeature);
            }
        }
    }

    if (Recursive == scope) {
        foreach (const U2Feature &feature, subfeatures) {
            result.append(getSubfeatures(feature.id, type, Recursive, NotSelectParentFeature));
        }
    }

    std::stable_sort(result.begin(), result.end(), featureLessThan);
    return result;
}

QList<U2Feature> AnnotationDataCache::getSubfeaturesByRegion(const U2DataId &rootId, const U2Region &range, bool contains) {
    QMutexLocker locker(&guard);

    QList<U2Feature> result;
    foreach(const U2DataId &featureId, rootSubfeatures.value(rootId)) {
        const AnnotationData a = annotationDataId.value(featureId);
        if (annotationIntersectsRange(a, range, contains)) {
            result << feature2Id.value(featureId);
        }
    }
    return result;
}

void AnnotationDataCache::removeAnnotationData(const U2DataId &featureId) {
    QMutexLocker locker(&guard);

    removeFeature(featureId);

    SAFE_POINT(annotationDataId.contains(featureId), "Unexpected annotation requested", );
    annotationDataId.remove(featureId);
}

void AnnotationDataCache::removeGroup(const U2DataId &featureId) {
    removeFeature(featureId);
}

void AnnotationDataCache::removeFeature(const U2DataId &featureId) {
    QMutexLocker locker(&guard);

    SAFE_POINT(feature2Id.contains(featureId) && rootSubfeatures.contains(feature2Id[featureId].rootFeatureId),
        "Unexpected annotation requested", );

    rootSubfeatures[feature2Id[featureId].rootFeatureId].removeAll(featureId);
    feature2Id.remove(featureId);
}

void AnnotationDataCache::removeAnnotationTableData(const U2DataId &rootId) {
    QMutexLocker locker(&guard);

    foreach (const U2DataId &featureId, rootSubfeatures.value(rootId)) {
        U2Feature::FeatureType type = feature2Id[featureId].type;
        if (U2Feature::Annotation == type) {
            removeAnnotationData(featureId);
        } else if (U2Feature::Group == type) {
            removeGroup(featureId);
        } else {
            assert(false);
            coreLog.error("Unexpected feature ID");
        }
    }
    rootSubfeatures.remove(rootId);
    rootReferenceCount.remove(rootId);
}

void AnnotationDataCache::refAnnotationTable(const U2DataId &rootId) {
    QMutexLocker locker(&guard);
    const int curRefCount = rootReferenceCount[rootId];
    SAFE_POINT(curRefCount >= 0, "Invalid annotation table ref count", );
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

void DbiAnnotationCache::addGroup(const U2DbiRef &dbiRef, const U2Feature &feature) {
    if (!dbiDataCache.contains(dbiRef)) {
        dbiDataCache.insert(dbiRef, new AnnotationDataCache());
    }
    dbiDataCache[dbiRef]->addGroup(feature);
}

bool DbiAnnotationCache::containsAnnotationTable(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    return dbiDataCache.contains(dbiRef) && dbiDataCache[dbiRef]->containsAnnotationTable(rootId);
}

bool DbiAnnotationCache::containsAnnotation(const U2DbiRef &dbiRef, const U2DataId &featureId) {
    return dbiDataCache.contains(dbiRef) && dbiDataCache[dbiRef]->containsAnnotation(featureId);
}

bool DbiAnnotationCache::containsGroup(const U2DbiRef &dbiRef, const U2DataId &featureId) {
    return dbiDataCache.contains(dbiRef) && dbiDataCache[dbiRef]->containsGroup(featureId);
}

int DbiAnnotationCache::getAnnotationTableSize(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    SAFE_POINT(containsAnnotationTable(dbiRef, rootId), "Unexpected annotation table requested from cache", -1);
    return dbiDataCache[dbiRef]->getAnnotationTableSize(rootId);
}

AnnotationData & DbiAnnotationCache::getAnnotationData(const U2DbiRef &dbiRef, const U2DataId &featureId) {
    SAFE_POINT(containsAnnotation(dbiRef, featureId), "Unexpected annotation requested from cache", defaultAnnotationReturnValue);
    return dbiDataCache[dbiRef]->getAnnotationData(featureId);
}

U2Feature & DbiAnnotationCache::getFeature(const U2DbiRef &dbiRef, const U2DataId &featureId) {
    SAFE_POINT(containsAnnotation(dbiRef, featureId) || containsGroup(dbiRef, featureId), "Unexpected feature requested from cache",
        defaultFeatureReturnValue);
    return dbiDataCache[dbiRef]->getFeature(featureId);
}

QList<U2Feature> DbiAnnotationCache::getTableFeatures(const U2DbiRef &dbiRef, const U2DataId &rootId, const FeatureFlags &type) {
    SAFE_POINT(containsAnnotationTable(dbiRef, rootId), "Unexpected annotation table requested from cache", QList<U2Feature>());
    return dbiDataCache[dbiRef]->getTableFeatures(rootId, type);
}

QList<U2Feature> DbiAnnotationCache::getSubfeatures(const U2DbiRef &dbiRef, const U2DataId &parentId, const FeatureFlags &type,
    OperationScope scope, SubfeatureSelectionMode mode)
{
    SAFE_POINT(containsGroup(dbiRef, parentId), "Unexpected annotation group requested from cache", QList<U2Feature>());
    return dbiDataCache[dbiRef]->getSubfeatures(parentId, type, scope, mode);
}

QList<U2Feature> DbiAnnotationCache::getSubfeaturesByRegion(const U2DbiRef &dbiRef, const U2DataId &rootId, const U2Region &range, bool contains) {
    SAFE_POINT(containsAnnotationTable(dbiRef, rootId), "Unexpected annotation table requested from cache", QList<U2Feature>());
    return dbiDataCache[dbiRef]->getSubfeaturesByRegion(rootId, range, contains);
}

void DbiAnnotationCache::removeAnnotationData(const U2DbiRef &dbiRef, const U2DataId &featureId) {
    SAFE_POINT(containsAnnotation(dbiRef, featureId), "Unexpected annotation requested from cache", );
    dbiDataCache[dbiRef]->removeAnnotationData(featureId);
}

void DbiAnnotationCache::removeAnnotationGroup(const U2DbiRef &dbiRef, const U2DataId &featureId) {
    SAFE_POINT(containsGroup(dbiRef, featureId), "Unexpected annotation group requested from cache", );
    dbiDataCache[dbiRef]->removeGroup(featureId);
}

void DbiAnnotationCache::removeAnnotationTableData(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    SAFE_POINT(containsAnnotationTable(dbiRef, rootId), "Unexpected annotation table requested from cache", );
    dbiDataCache[dbiRef]->removeAnnotationTableData(rootId);
}

void DbiAnnotationCache::refAnnotationTable(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    if (!containsAnnotationTable(dbiRef, rootId)) {
        if (!dbiDataCache.contains(dbiRef)) {
            dbiDataCache.insert(dbiRef, new AnnotationDataCache());
        }
        dbiDataCache[dbiRef]->addAnnotationTable(rootId);
    }
    dbiDataCache[dbiRef]->refAnnotationTable(rootId);
}

void DbiAnnotationCache::derefAnnotationTable(const U2DbiRef &dbiRef, const U2DataId &rootId) {
    CHECK(containsAnnotationTable(dbiRef, rootId), );
    dbiDataCache[dbiRef]->derefAnnotationTable(rootId);
}

void DbiAnnotationCache::onDbiRelease(const U2DbiRef &dbiRef) {
    if (dbiDataCache.contains(dbiRef)) {
        delete dbiDataCache[dbiRef];
        dbiDataCache.remove(dbiRef);
    }
}

}
