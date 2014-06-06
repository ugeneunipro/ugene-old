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

#ifndef _U2_ANNOTATION_DATA_CACHE_H_
#define _U2_ANNOTATION_DATA_CACHE_H_

#include <QtCore/QMutex>
#include <QtCore/QSet>

#include <U2Core/AnnotationData.h>

namespace U2 {

/**
 * The classes below are intended for caching annotation data fetched from a DBI
 */

class U2Feature;

/**
 * This class represents a storage for annotations belonging to a particular database.
 * Annotation tables are distinguished here by their root features and annotations, in turn,
 * are associated with appropriate features in DB.
 */

class AnnotationDataCache {
public:
    AnnotationDataCache();

    void addData(const U2Feature &feature, const AnnotationData &data);

    bool containsAnnotationTable(const U2DataId &rootId);
    bool contains(const U2DataId &featureId);
    int getAnnotationTableSize(const U2DataId &rootId);

    AnnotationData & getAnnotationData(const U2DataId &featureId);
    U2Feature & getFeature(const U2DataId &featureId);
    QList<U2Feature> getSubfeatures(const U2DataId &rootId);

    void removeAnnotationData(const U2DataId &featureId);
    void removeAnnotationTableData(const U2DataId &rootId);

    void refAnnotationTable(const U2DataId &rootId);
    void derefAnnotationTable(const U2DataId &rootId);

private:
    QMutex guard;

    QHash<U2DataId, AnnotationData> annotationDataId;
    QHash<U2DataId, U2Feature> feature2Id;
    QHash<U2DataId, QSet<U2DataId> > rootSubfeatures;
    QHash<U2DataId, int> rootReferenceCount;
};

/**
 * This class represents a set of storages defined above and associated with different databases.
 * So the instance of this class delegates all queries to annotation caches stored in it.
 */

class DbiAnnotationCache {
public:
    ~DbiAnnotationCache();

    void addData(const U2DbiRef &dbiRef, const U2Feature &feature, const AnnotationData &data);

    bool containsAnnotationTable(const U2DbiRef &dbiRef, const U2DataId &rootId);
    bool contains(const U2DbiRef &dbiRef, const U2DataId &featureId);
    int getAnnotationTableSize(const U2DbiRef &dbiRef, const U2DataId &rootId);

    AnnotationData & getAnnotationData(const U2DbiRef &dbiRef, const U2DataId &featureId);
    U2Feature & getFeature(const U2DbiRef &dbiRef, const U2DataId &featureId);
    QList<U2Feature> getSubfeatures(const U2DbiRef &dbiRef, const U2DataId &rootId);

    void removeAnnotationData(const U2DbiRef &dbiRef, const U2DataId &featureId);
    void removeAnnotationTableData(const U2DbiRef &dbiRef, const U2DataId &rootId);

    void refAnnotationTable(const U2DbiRef &dbiRef, const U2DataId &rootId);
    void derefAnnotationTable(const U2DbiRef &dbiRef, const U2DataId &rootId);

    void onDbiRelease(const U2DbiRef &dbiRef);

private:
    QMap<U2DbiRef, AnnotationDataCache *> dbiDataCache;
};

}

#endif // _U2_ANNOTATION_DATA_CACHE_H_
