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

#include "MSAEditorConsensusCache.h"

#include <U2Core/MAlignmentObject.h>
#include <U2Algorithm/MSAConsensusAlgorithm.h>

namespace U2 {

MSAEditorConsensusCache::MSAEditorConsensusCache(QObject* p, MAlignmentObject* o, MSAConsensusAlgorithmFactory* factory) 
: QObject(p), currentVersion(1), aliObj(o), algorithm(NULL)
{
    setConsensusAlgorithm(factory);
    
    connect(aliObj, SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)),
        SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));

    cache.resize(aliObj->getLength());
}

MSAEditorConsensusCache::~MSAEditorConsensusCache() {
    delete algorithm;
    algorithm = NULL;
}

void MSAEditorConsensusCache::setConsensusAlgorithm(MSAConsensusAlgorithmFactory* factory) {
    delete algorithm;
    algorithm = NULL;
    algorithm = factory->createAlgorithm(aliObj->getMAlignment());
    connect(algorithm, SIGNAL(si_thresholdChanged(int)), SLOT(sl_thresholdChanged(int)));
    currentVersion++;
}

void MSAEditorConsensusCache::sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&) {
    cache.resize(aliObj->getLength());
    currentVersion++;
}

void MSAEditorConsensusCache::updateCacheItem(int pos) {
    const MAlignment& ma = aliObj->getMAlignment();
    assert(pos >= 0 && pos < ma.getLength());
    assert(pos < cache.size() && cache.size() == ma.getLength());
    const CacheItem& cci = cache[pos];
    if (cci.version == currentVersion) {
        return;
    }
    CacheItem& ci = cache[pos];
    int count = 0;
    int nSeq = ma.getNumRows();

    ci.topChar = algorithm->getConsensusCharAndScore(ma, pos, count);
    ci.topPercent = (char)qRound(count * 100 / nSeq);
    assert(ci.topPercent >=0 && ci.topPercent<=100);
    ci.version = currentVersion;
}

char MSAEditorConsensusCache::getConsensusChar(int pos) {
    updateCacheItem(pos);
    const CacheItem& ci = cache[pos];
    return ci.topChar;
}

int MSAEditorConsensusCache::getConsensusCharPercent(int pos) {
    updateCacheItem(pos);
    const CacheItem& ci = cache[pos];
    return ci.topPercent;
}

QByteArray MSAEditorConsensusCache::getConsensusLine(bool withGaps) {
    QByteArray res;
    const MAlignment& ma = aliObj->getMAlignment();
    for (int i=0, n = ma.getLength(); i<n; i++) {
        char c = getConsensusChar(i);
        if (c!=MAlignment_GapChar || withGaps) {
            res.append(c);
        }
    }
    return res;
}

void MSAEditorConsensusCache::sl_thresholdChanged(int newValue) {
    Q_UNUSED(newValue);
    currentVersion++;
}

}//namespace

