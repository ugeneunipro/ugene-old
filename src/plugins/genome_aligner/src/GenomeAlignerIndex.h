/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GENOME_ALIGNER_INDEX_H_
#define _U2_GENOME_ALIGNER_INDEX_H_

#include <U2Core/Task.h>
#include <U2Algorithm/BitsTable.h>
#include <QtCore/QFile>

namespace U2 {

#define ResType qint64
class SearchContext;
class SearchQuery;

class GenomeAlignerIndex {
    friend class GenomeAlignerIndexTask;
public:
    GenomeAlignerIndex();
    ~GenomeAlignerIndex();

    quint64 getBitValue(const char *seq, int length) const;
    int getPrefixSize() const;
    int findInCache(quint64 bitValue, quint64 bitFilter) const;
    void loadPart(int part);
    void findInPart(QFile *refFile, int startPos, ResType firstResult,
                    quint64 bitValue, SearchQuery *qu, SearchContext *settings);
    int findBit(quint64 bitValue, quint64 bitFilter);
    ResType *findBitOpenCL(quint64 *bitValues, int size, quint64 bitFilter);
    QFile *openRefFile();
    QString getSeqName() const {return seqObjName;}
    int getPartsInMemCache() const {return partsInMemCache;}

private:
    const char      *seq;           //reference sequence
    quint32         seqLength;      //reference sequence's length
    int             w;              //window size
    QString         baseFileName;   //base of the file name
    quint32         indexLength;    //count of index's elements
    quint32         *sArray;
    quint64         *bitMask;
    quint32         loadedPartSize;
    quint32         *memIdx;
    quint64         *memBM;
    BitsTable       bt;
    const quint32*  bitTable;
    int             bitCharLen;
    QFile           *indexFile;
    QFile           *refFile;
    int             partsInMemCache;
    quint32         *objLens;
    int             objCount;
    QString         seqObjName;
    int             currentPart;

    void serialize(const QString &refFileName, TaskStateInfo &ti);
    void deserialize(TaskStateInfo &ti);
    bool openIndexFile();
    void createMemCache();
    inline bool isValidPos(quint32 offset, int startPos, int length, quint32 &fisrtSymbol, const QList<quint32> &results);
    inline bool compare(const char *sourceSeq, const QByteArray &querySeq, int startPos,
        int w, int bits, int &c, int CMAX, int restBits);
    inline void fullBitMaskOptimization(int CMAX, quint64 bitValue, quint64 bitMaskValue, int restBits, int w, int &bits, int &c);

    static const QString HEADER;
    static const QString PARAMETERS;

public:
    static const QString HEADER_EXTENSION;
    static const QString SARRAY_EXTENSION;
    static const QString REF_INDEX_EXTENSION;
    static const int charsInMask;
};

} //U2

#endif
