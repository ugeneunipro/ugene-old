#ifndef _SPB_COMPARING_ALGORITHM_H_
#define _SPB_COMPARING_ALGORITHM_H_

#include <U2Algorithm/PairwiseAligner.h>
#include <U2Algorithm/SArrayIndex.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignment.h>
#include <U2Core/U2OpStatus.h>

#include <U2Lang/DbiDataHandler.h>
#include <U2Lang/DbiDataStorage.h>

using namespace U2;
using namespace U2::Workflow;

namespace SPB {

class ComparingAlgorithm;

class ComparingAlgorithmFactory {
public:
    static ComparingAlgorithm * createAlgorithm(const QString &algoId);

    static const QString DEFAULT;
};

class ComparingAlgorithm {
public:
    ComparingAlgorithm();
    virtual ~ComparingAlgorithm();

    /** Returns the percent of sequences identity: 0.0 .. 100.0 */
    virtual double compare(const U2SequenceObject *seq1,
        const U2SequenceObject *seq2);
    virtual double compare(const QByteArray &seq1, const QByteArray &seq2) = 0;
    virtual double compare() = 0;
};

class DefaultComparingAlgorithm : public ComparingAlgorithm {
public:
    DefaultComparingAlgorithm();
    virtual ~DefaultComparingAlgorithm();
    virtual double compare(const QByteArray &seq1, const QByteArray &seq2);
    virtual double compare();

    virtual void setSeq1(const QByteArray &seq1, double accuracy);
    virtual void setSeq2(const QByteArray &seq1);

private:
    MAlignment align(const QByteArray &seq1, const QByteArray &seq2);
    bool symbolsEqual(char c1, char c2);

    void createIndex();

private:
    PairwiseAligner *aligner;
    QByteArray seq1;
    QByteArray seq2;
    double accuracy;
    SArrayIndex *index;

    static const int MIN_INDEX_WINDOW_SIZE;
};

} // SPB

#endif // _SPB_COMPARING_ALGORITHM_H_
