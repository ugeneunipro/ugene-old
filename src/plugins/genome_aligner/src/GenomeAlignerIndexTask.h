#ifndef _U2_GENOME_ALIGNER_INDEX_TASK_H_
#define _U2_GENOME_ALIGNER_INDEX_TASK_H_

#include <U2Core/Task.h>
#include <U2Algorithm/BitsTable.h>
#include <QList>
#include <QFile>

namespace U2 {

class GenomeAlignerIndex;
class DNASequenceObject;

class GenomeAlignerIndexSettings {
public:
    QString indexFileName;
    QString refFileName;
    bool    deserializeFromFile;
    bool    openCL;
};

class GenomeAlignerIndexTask: public Task {
    Q_OBJECT
public:
    GenomeAlignerIndexTask(const GenomeAlignerIndexSettings &settings);
    ~GenomeAlignerIndexTask();
    void run();
    qint64 getFreeMemSize() {return memFreeSize;}
    qint64 getFreeGPUSize() {return gpuFreeSize;}

    GenomeAlignerIndex    *index;

private:
    char            *seq;           //reference sequence
    quint32         seqLength;      //reference sequence's length
    int             w;              //window size
    QString         baseFileName;   //base of the index's file name
    quint32         indexLength;    //count of index's elements
    quint32         *sArray;
    quint64         *bitMask;
    BitsTable       bt;
    const quint32*  bitTable;
    int             bitCharLen;
    QList<QFile*>   tempFiles;
    QFile           *newRefFile;
    quint64         bitFilter;
    char            *buffer;
    quint32         *objLens;
    int             objCount;
    char            unknownChar;

    qint64           memFreeSize;
    qint64           gpuFreeSize;

    GenomeAlignerIndexSettings settings;

    quint32 PART_SIZE; //*12 = ~500 Mb
    static const int BUFF_SIZE = 6291456; //6Mb. Must be divided by 12
    static double MEMORY_DIVISION;


private:
    void buildPart(quint32 *idx, int *curObj, quint32 *arrLen);
    void sort(quint64* x, int off, int len);
    inline qint64 compare(const quint64 *x1, const quint64 *x2) const;
    inline void swap(quint64 *x1, quint64 *x2) const;
    inline quint32 med3(quint64 *x, quint32 a, quint32 b, quint32 c);
    inline void vecswap(quint64 *x1, quint64 *x2, quint32 n);
    void writePart(QFile *file, quint32 arrLen);
    void mergeSort();
    void reformatSequence();
    quint32 initPart(quint32 *idx, int *curObj, quint32 *arrLen, QByteArray &refPart);
    void calculateMemForAligning();
};

} //U2

#endif // _U2_GENOME_ALIGNER_INDEX_TASK_H_
