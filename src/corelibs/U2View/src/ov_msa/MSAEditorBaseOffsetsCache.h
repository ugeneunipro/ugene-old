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

#ifndef _U2_MSA_EDITOR_BASEOFFSET_CACHE_H_
#define _U2_MSA_EDITOR_BASEOFFSET_CACHE_H_

#include <QtCore/QObject>
#include <QtCore/QVector>

namespace U2 {

class MAlignmentObject;
class MAlignment;
class MAlignmentModInfo;

//TODO: optimize this cache example -> use sparse cache for 1 of 100 positions in every sequence

class MSAEditorBaseOffsetCache : public QObject {
    Q_OBJECT
public:
    MSAEditorBaseOffsetCache(QObject* p, MAlignmentObject* aliObj);

    int getBaseCounts(int seqNum, int aliPos, bool inclAliPos);
    
    MAlignmentObject* getMSAObject() const {return aliObj;}
private slots:
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&);

private:
    void updateCacheSize();
    void updateCacheRow(int seqNum);
    int  _getBaseCounts(int seqNum, int aliPos, int& cachedEndPos);

    class RowCache {
    public:
        RowCache() : cacheVersion(0){}
        int          cacheVersion;
        QVector<int> rowOffsets;
    };

    
    MAlignmentObject*       aliObj;
    QVector<RowCache>       cache;
    int                     objVersion;
    int                     globVersion;
};

}//namespace;

#endif
