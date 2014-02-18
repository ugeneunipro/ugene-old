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

#ifndef _U2_MSA_EDITOR_CONSENSUS_CACHE_H_
#define _U2_MSA_EDITOR_CONSENSUS_CACHE_H_

#include <QtCore/QObject>
#include <QtCore/QVector>

namespace U2 {

class MAlignmentObject;
class MAlignment;
class MAlignmentModInfo;
class MSAConsensusAlgorithm;
class MSAConsensusAlgorithmFactory;

class MSAEditorConsensusCache : public QObject {
    Q_OBJECT
public:
    MSAEditorConsensusCache(QObject* p, MAlignmentObject* aliObj, MSAConsensusAlgorithmFactory* algo);
    ~MSAEditorConsensusCache();

    char getConsensusChar(int pos);
    
    int getConsensusCharPercent(int pos);

    int getConsensusLength() const { return cache.size(); }

    void setConsensusAlgorithm(MSAConsensusAlgorithmFactory* algo);

    MSAConsensusAlgorithm* getConsensusAlgorithm() const {return algorithm;}

    QByteArray getConsensusLine(bool withGaps);

private slots:
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&);
    void sl_thresholdChanged(int newValue);

private:
    struct CacheItem {
        CacheItem(int v = 0, char c = '-', int tc = 0) : version(v), topChar(c), topPercent(tc){}
        int     version;
        char    topChar;
        char    topPercent;
    };

    void updateCacheItem(int pos);

    int                     currentVersion;
    QVector<CacheItem>      cache;
    MAlignmentObject*       aliObj;
    MSAConsensusAlgorithm*  algorithm;
};

}//namespace;

#endif
