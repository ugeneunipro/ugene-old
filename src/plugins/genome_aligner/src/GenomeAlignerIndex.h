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
#include "GenomeAlignerIndexPart.h"
#include "GenomeAlignerSettingsWidget.h"
#include "GenomeAlignerFindTask.h"

namespace U2 {

#define ResType qint64
class SearchContext;
class SearchQuery;

class GenomeAlignerIndex {
    friend class GenomeAlignerIndexTask;
    friend class GenomeAlignerSettingsWidget;
    friend class GenomeAlignerFindTask;
public:
    GenomeAlignerIndex();
    ~GenomeAlignerIndex();

    BMType getBitValue(const char *seq, int length) const;
    void loadPart(int part);
    void findInPart(int startPos, ResType firstResult, BMType bitValue, SearchQuery *qu, SearchContext *settings);
    ResType findBit(BMType bitValue, BMType bitFilter);
    ResType *findBitOpenCL(BMType *bitValues, int size, quint64 BMType);
    ResType *findBitValuesUsingCUDA(BMType *bitValues, int size, BMType filter);
    QString getSeqName() const {return seqObjName;}
    int getPartCount() const {return indexPart.partCount;}
    SAType getSArraySize() const {return indexPart.saLengths[currentPart];}
    IndexPart& getLoadedPart() { return indexPart; }

private:
    quint32         seqLength;      //reference sequence's length
    int             seqPartSize;    //in Mb
    int             w;              //window size
    QString         baseFileName;   //base of the file name
    quint32         *memIdx;
    quint64         *memBM;
    BitsTable       bt;
    const quint32*  bitTable;
    int             bitCharLen;
    BMType          bitFilter;
    int             partsInMemCache;
    quint32         *objLens;
    int             objCount;
    QString         seqObjName;
    int             currentPart;
    IndexPart       indexPart;
    bool            build;
    char            unknownChar;

    void serialize(const QString &refFileName);
    bool deserialize(QByteArray &error);
    bool openIndexFiles();
    inline bool isValidPos(SAType offset, int startPos, int length, SAType &fisrtSymbol, SearchQuery *qu, SAType &loadedSeqStart);
    inline bool compare(const char *sourceSeq, const char *querySeq, int startPos, int w, int &c, int CMAX, int length);
    inline void fullBitMaskOptimization(int CMAX, BMType bitValue, BMType bitMaskValue, int restBits, int w, int &bits, int &c);
    inline bool find(SAType &offset, SAType &firstSymbol, int &startPos, SearchQuery *qu, bool &bestMode, int &CMAX, bool valid);

    static const QString HEADER;
    static const QString PARAMETERS;

    /*build*/
    SAType          *sArray;
    BMType          *bitMask;
    void buildPart(SAType start, SAType length, SAType &arrLen);
    void initSArray(SAType start, SAType length, SAType &arrLen);
    void sort(BMType *x, int off, int len);
    inline qint64 compare(const BMType *x1, const BMType *x2) const;
    inline void swap(BMType *x1, BMType *x2) const;
    inline quint32 med3(BMType *x, quint32 a, quint32 b, quint32 c);
    inline void vecswap(BMType *x1, BMType *x2, quint32 n);

public:
    static const QString HEADER_EXTENSION;
    static const QString SARRAY_EXTENSION;
    static const QString REF_INDEX_EXTENSION;
    static const int charsInMask;
    static const int overlapSize;
};

} //U2

#endif
