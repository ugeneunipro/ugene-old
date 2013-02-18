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

#include "PanViewRows.h"

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/U2SafePoints.h>
#include <QtCore/QVarLengthArray>

namespace U2 {

PVRowsManager::~PVRowsManager() {
    clear();
}

void PVRowsManager::clear() {
    qDeleteAll(rows);
    rows.clear();
    rowByAnnotation.clear();
}

typedef QVector<U2Region>::const_iterator LRIter;

bool PVRowData::fitToRow(const QVector<U2Region>& location) {
    //assume locations are always in ascending order
    //usually annotations come in sorted by location 
    //first check the most frequent way
    if (!ranges.isEmpty()) {
        const U2Region& l = location.first();
        const U2Region& r = ranges.last();
        if (l.startPos > r.endPos()) {
            ranges << location;
            return true;
        } else if (l.startPos >= r.startPos || l.endPos() >= r.startPos) {
            //got intersection
            return false;
        }
    }
    //bad luck, full search required
    QVarLengthArray<int,16> pos;
    LRIter zero = ranges.constBegin();
    LRIter end = ranges.constEnd();
    foreach(const U2Region& l, location) {
        LRIter it = qLowerBound(ranges, l);
        if (it != end && 
                (it->startPos <= l.endPos() || (it != zero && (it-1)->endPos() >= l.startPos))
           ) {
            //got intersection
            return false;
        }
        pos.append(it-zero);
    }
    //ok this feature can be added to row;
    //keep the ranges in ascending order
    for(int i = location.size()-1; i>=0; i--) {
        ranges.insert(pos[i], location.at(i));
    }
    
    return true;
}

inline bool compare_rows(PVRowData* x, PVRowData* y) { 
    return  x->key.compare(y->key) > 0;
}    

void PVRowsManager::addAnnotation(Annotation* a, const QString& key) {
    SAFE_POINT(!rowByAnnotation.contains(a), "Annoation has been already added", );
    QVector<U2Region> location = a->getRegions();
    foreach(PVRowData* row, rows) {
        if (row->key == key && row->fitToRow(location)) {
            row->annotations.append(a);
            rowByAnnotation[a] = row;
            return;
        }
    }
    PVRowData* row = new PVRowData(key);
    row->ranges << location;
    row->annotations.append(a);

    rowByAnnotation[a] = row;
    rows.push_back(row);
    qStableSort(rows.begin(), rows.end(), compare_rows);
}

void PVRowsManager::removeAnnotation(Annotation* a) {
    PVRowData* row = rowByAnnotation.value(a, NULL);
    SAFE_POINT(row != NULL, "Now row by annotation", );
    rowByAnnotation.remove(a);
    row->annotations.removeOne(a);
    U2Region::removeAll(row->ranges, a->getRegions());
    if (row->annotations.empty()) {
        rows.removeOne(row);
        delete row;
    }
}

int PVRowsManager::getAnnotationRowIdx(Annotation* a) const {
    PVRowData* row = rowByAnnotation.value(a, NULL);
    if (NULL == row) {
        return -1;
    } else {
        return rows.indexOf(row);
    }
}

int PVRowsManager::getNumAnnotationsInRow(int rowNum) const {
    SAFE_POINT(rowNum >= 0 && rowNum < rows.size(), "Row number out of range", 0);
    PVRowData* r = rows[rowNum];
    return r->annotations.size();
}


bool PVRowsManager::contains(const QString& key) const {
    foreach(PVRowData* r, rows) {
        if (r->key == key) {
            return true;
        }
    }
    return false;
}

PVRowData* PVRowsManager::getRow( int row ) const
{
    if (row>= 0 && row<rows.size()) {
        return rows.at(row);
    }
    return NULL;
}

} // namespace
