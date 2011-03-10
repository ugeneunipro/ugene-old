#ifndef _SMITHWATERMANALGORITHM_H
#define _SMITHWATERMANALGORITHM_H

#include "PairAlignSequences.h"

#include <U2Core/SMatrix.h>

#include <QtCore/QVector>
#include <QtCore/QByteArray>

namespace U2 {

class SmithWatermanAlgorithm {

public:
    SmithWatermanAlgorithm();
    virtual ~SmithWatermanAlgorithm() {};

    virtual void launch(const SMatrix& m, QByteArray const & _patternSeq,
        QByteArray const & _searchSeq, int _gapOpen, int _gapExtension, int _minScore);
    
    QList<PairAlignSequences> getResults();
    static void sortByScore(QList<PairAlignSequences> & pairAlignmentStrings);

protected:
    bool calculateMatrixLength();
    void printMatrix() const;
    int getRow(int row) const;
    bool isValidParams();
    int maximum(int var1, int var2, int var3, int var4 = 0);
    void setValues(const SMatrix& _substitutionMatrix, 
        QByteArray const & _patternSeq, QByteArray const & _searchSeq, 
        int _gapOpen, int _gapExtension, int _minScore);

    QList<PairAlignSequences> pairAlignmentStrings;

    SMatrix substitutionMatrix;

    QByteArray patternSeq;
    QByteArray searchSeq;

    int gapOpen;
    int gapExtension;    
    int minScore;
    int matrixLength;
    int storedResults;

    QVector<QVector<char> > directionMatrix;

    void backtrace(int row, int col, int score);

    static const char STOP;
    static const char UP;
    static const char LEFT;
    static const char DIAG;

    struct KeyOfPairAlignSeq
    {
        KeyOfPairAlignSeq(int _score, U2Region const & _intervalSeq1) {
            setValues(_score, _intervalSeq1);
        }
//         KeyOfPairAlignSeq(int _score, U2Region const & _intervalSeq1, U2Region const & _intervalSeq2) {
//             setValues(_score, _intervalSeq1, _intervalSeq2);
//         }

//         KeyOfPairAlignSeq() {
//             score = 0;
// 
//             intervalSeq1.startPos = 0;
//             intervalSeq1.len = 0;
// 
//             intervalSeq2.startPos = 0;;
//             intervalSeq2.len = 0;
//         };

        KeyOfPairAlignSeq() {
            score = 0;

            intervalSeq1.startPos = 0;
            intervalSeq1.length = 0;
        };


//         static void exchange(KeyOfPairAlignSeq & a, int & pos1, KeyOfPairAlignSeq & b, int & pos2) {
//             KeyOfPairAlignSeq bufKey;
//             int buf;
// 
//             buf = pos1;
//             pos1 = pos2;
//             pos2 = buf;
// 
//             bufKey = a;
//             a = b;
//             b = bufKey;
//         }
        static void exchange(PairAlignSequences & a, PairAlignSequences & b) {
            PairAlignSequences bufKey;

            bufKey = a;
            a = b;
            b = bufKey;
        }

//         void setValues(int _score, U2Region const & _intervalSeq1, U2Region const & _intervalSeq2) {
//             score = _score;
//             intervalSeq1 = _intervalSeq1;
//             intervalSeq2 = _intervalSeq2;
//         }

        void setValues(int _score, U2Region const & _intervalSeq1) {
            score = _score;
            intervalSeq1 = _intervalSeq1;            
        }

        int score;
        U2Region intervalSeq1;
        
    };


private:

    QVector<QVector<int> > matrix;
    QVector<int> EMatrix;
    QVector<int> FMatrix;    

    void calculateMatrix();    
    
};

}//namespace

#endif
