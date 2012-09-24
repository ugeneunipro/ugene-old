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

#include "MSAEditorBaseOffsetsCache.h"

#include <U2Core/MAlignmentObject.h>

namespace U2 {

//one of 256 positions contains offset info
#define CACHE_FREQ 256

MSAEditorBaseOffsetCache::MSAEditorBaseOffsetCache(QObject* p, MAlignmentObject* obj) 
: QObject(p), aliObj(obj)
{
    connect(aliObj, SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)), 
        SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
    
    objVersion = 1;
    globVersion = 0;
}


void MSAEditorBaseOffsetCache::sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo& modInfo) {
    if (modInfo.sequenceContentChanged) {
        objVersion++;
    }
}

int MSAEditorBaseOffsetCache::getBaseCounts(int seqNum, int aliPos, bool inclAliPos) {
    const MAlignment& ma = aliObj->getMAlignment();
    const MAlignmentRow& row = ma.getRow(seqNum);
    int endPos = inclAliPos ? aliPos + 1 : aliPos;
    if (endPos < row.getCoreStart()) {
        return 0;
    }
    int i = 0;
    int cnt = _getBaseCounts(seqNum, aliPos, i);
    for (; i < endPos; i++) {
        if (row.charAt(i) != MAlignment_GapChar) {
            cnt++;
        }
    }
    return cnt;
}

int  MSAEditorBaseOffsetCache::_getBaseCounts(int seqNum, int aliPos, int& cachedEndPos) {
    updateCacheSize();
    updateCacheRow(seqNum);
    const RowCache& r = cache[seqNum];
    assert(globVersion = objVersion);
    assert(r.cacheVersion == objVersion);
    int cacheIdx = aliPos / CACHE_FREQ  - 1;
    if (cacheIdx < 0) {
        cachedEndPos = 0;
        return 0;
    }
    cachedEndPos = (cacheIdx + 1) * CACHE_FREQ;
    int res = r.rowOffsets[cacheIdx];
    return res;
}

void MSAEditorBaseOffsetCache::updateCacheSize() {
    if (objVersion == globVersion) {
        return;
    }
    int nSeq = aliObj->getMAlignment().getNumRows();
    cache.resize(nSeq);
    globVersion = objVersion;
}

void MSAEditorBaseOffsetCache::updateCacheRow(int seqNum) {
    assert(globVersion == objVersion);
    RowCache& r = cache[seqNum];
    if (r.cacheVersion == objVersion) {
        return;
    }
    const MAlignment& ma = aliObj->getMAlignment();
    int aliLen = ma.getLength();
    int nPoints = aliLen / CACHE_FREQ;
    r.rowOffsets.resize(nPoints);
    const MAlignmentRow& row = ma.getRow(seqNum);
    const char* seq = row.getCore().constData();
    int firstCharOffset = row.getCoreStart();
    int lastCharOffset = row.getCoreEnd();
    for (int i = 0, cnt = 0; i < aliLen; i++) {
        if (i != 0 && i % CACHE_FREQ == 0) {
            r.rowOffsets[i / CACHE_FREQ - 1] = cnt;
        }
        if (i < firstCharOffset || i >= lastCharOffset || seq[i - firstCharOffset] != MAlignment_GapChar) {
            cnt++;
        }
    }
    r.cacheVersion = objVersion;
}

}//namespace

