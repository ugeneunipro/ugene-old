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

#include <U2Core/U2Region.h>

namespace U2 {

QVector<U2Region> U2Region::join(QVector<U2Region>& regions)  {
    QVector<U2Region> result = regions;
    qStableSort(result.begin(), result.end()); //sort by region start pos first
    for (int i = 0; i < result.size()-1;) {
        const U2Region& ri0 = result[i];
        const U2Region& ri1 = result[i+1];
        if (!ri0.intersects(ri1)) {
            i++;
            continue;
        }
        U2Region newRi = containingRegion(ri0, ri1);
        result[i] = newRi;
        result.remove(i+1);
    }
    return result;
}

void U2Region::bound(qint64 minPos, qint64 maxPos, QVector<U2Region>& regions) {
    for (int i = 0, n = regions.size(); i < n; i++) {
        U2Region& r = regions[i];
        int start = qBound(minPos, r.startPos, maxPos);
        int end = qBound(minPos, r.endPos(), maxPos);
        r.startPos = start;
        r.length = end - start;
    }
}

void U2Region::mirror(qint64 mirrorPos, QVector<U2Region>& regions) {
    for (int i = 0, n = regions.size(); i < n; i++) {
        U2Region& r = regions[i];
        assert(r.endPos() <= mirrorPos);
        r.startPos = mirrorPos - r.endPos();
    }
}

void U2Region::divide(qint64 div, QVector<U2Region>& regions) {
    for (int i = 0, n = regions.size(); i < n; i++) {
        U2Region& r = regions[i];
        r.startPos = r.startPos / div;
    }
}

void U2Region::multiply(qint64 mult, QVector<U2Region>& regions) {
    for (int i = 0, n = regions.size(); i < n; i++) {
        U2Region& r = regions[i];
        r.startPos = r.startPos * mult;
    }
}

void U2Region::reverse(QVector<U2Region>& regions) {
    QVector<U2Region> old = regions;
    regions.clear();
    foreach(const U2Region& r, old) {
        regions.prepend(r);
    }
}

void U2Region::shift(qint64 offset, QVector<U2Region>& regions) {
    QVector<U2Region> res;
    for (int i = 0, n = regions.size(); i < n; i++) {
        U2Region& r = regions[i];
        r.startPos += offset;
    }
}

int U2Region::findIntersectedRegion(const QVector<U2Region>& rs) const {
    for (int i = 0, n = rs.size(); i < n; i++) {
        const U2Region& r = rs[i];
        if (intersects(r)) {
            return i;
        }
    }
    return -1;
}


int U2Region::findOverlappingRegion(const QVector<U2Region>& rs) const {
    for (int i = 0, n = rs.size(); i < n; i++) {
        const U2Region& r = rs[i];
        if (r.contains(*this)) {
            return i;
        }
    }
    return -1;
}

void U2Region::removeAll(QVector<U2Region>& regionsToProcess, const QVector<U2Region>& regionsToRemove) {
    QVector<U2Region> result;
    foreach(const U2Region& pr, regionsToProcess) {
        if (regionsToRemove.indexOf(pr) == -1) {
            result.append(pr);
        }
    }
    regionsToProcess = result;
}



static bool _registerMeta() {
    qRegisterMetaType<U2Region>("U2Region");
    qRegisterMetaTypeStreamOperators<U2Region>("U2::U2Region");

    qRegisterMetaType<QVector<U2Region> >("QVector<U2::U2Region>");
    qRegisterMetaTypeStreamOperators< QVector<U2Region> >("QVector<U2::U2Region>");
    return true;
}

bool U2Region::registerMeta = _registerMeta();

QDataStream &operator<<(QDataStream &out, const U2Region &myObj) {
    out << myObj.startPos << myObj.length;
    return out;
}

QDataStream &operator>>(QDataStream &in, U2Region &myObj) {
    in >> myObj.startPos;
    in >> myObj.length;
    return in;
}


} //ns