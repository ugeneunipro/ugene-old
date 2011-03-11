#ifndef _U2_GENOME_ALIGNER_INDEX_H_
#define _U2_GENOME_ALIGNER_INDEX_H_

#include <U2Core/Task.h>
#include <U2Algorithm/BitsTable.h>
#include <QtCore/QFile>

namespace U2 {

#define ResType qint64
class SearchContext;

class GenomeAlignerIndex {
    friend class GenomeAlignerIndexTask;
public:
    GenomeAlignerIndex();
    ~GenomeAlignerIndex();

    quint64 getBitValue(const char *seq, int length) const;
    int getPrefixSize() const;
    int findInCache(quint64 bitValue, quint64 bitFilter) const;
    void loadPart(int part);
    void findInPart(QFile *refFile, const QByteArray &seq, int startPos, ResType firstResult,
                    quint64 bitValue, QList<quint32> &results, SearchContext *settings);
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

    static const QString HEADER;
    static const QString PARAMETERS;

public:
    static const QString HEADER_EXTENSION;
    static const QString SARRAY_EXTENSION;
    static const QString REF_INDEX_EXTENSION;
    static const int charsInMask = 31;
};

} //U2

#endif
