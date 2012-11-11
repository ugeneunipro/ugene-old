#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "ComparingAlgorithm.h"

namespace SPB {

ComparingAlgorithm * ComparingAlgorithmFactory::createAlgorithm(const QString &algoId) {
    if (DEFAULT == algoId) {
        return new DefaultComparingAlgorithm();
    }

    return NULL;
}

const QString ComparingAlgorithmFactory::DEFAULT("Default");

ComparingAlgorithm::ComparingAlgorithm() {

}

ComparingAlgorithm::~ComparingAlgorithm() {

}

double ComparingAlgorithm::compare(const U2SequenceObject *seq1,
    const U2SequenceObject *seq2) {
    U2OpStatusImpl os;

    QByteArray array1 = seq1->getSequenceData(U2_REGION_MAX, os);
    CHECK_OP(os, 0);
    QByteArray array2 = seq2->getSequenceData(U2_REGION_MAX, os);
    CHECK_OP(os, 0);
    
    return compare(array1, array2);
}

/************************************************************************/
/* DefaultComparingAlgorithm */
/************************************************************************/
const int DefaultComparingAlgorithm::MIN_INDEX_WINDOW_SIZE = 8;

DefaultComparingAlgorithm::DefaultComparingAlgorithm()
: aligner(NULL), accuracy(0.0), index(NULL)
{

}

DefaultComparingAlgorithm::~DefaultComparingAlgorithm() {
    delete aligner;
    delete index;
}

double DefaultComparingAlgorithm::compare(const QByteArray &seq1,
    const QByteArray &seq2) {
    MAlignment ma = align(seq1, seq2);

    int mismatches = 0;
    U2OpStatus2Log os;
    QByteArray row1 = ma.getRow(0).toByteArray(ma.getLength(), os);
    QByteArray row2 = ma.getRow(1).toByteArray(ma.getLength(), os);
    for (int i=0; i<ma.getLength(); i++) {
        mismatches += (int)(!symbolsEqual(row1[i], row2[i]));
    }

    double result = double(ma.getLength() - mismatches) / ma.getLength();

    return result * 100;
}

MAlignment DefaultComparingAlgorithm::align(const QByteArray &seq1, const QByteArray &seq2) {
    if (NULL == aligner) {
        aligner = PairwiseAlignerFactory::createAligner(PairwiseAlignerFactory::NEEDLEMAN_WUNSCH, seq1, seq2);
        CHECK(NULL != aligner, MAlignment());
    } else {
        aligner->setSeqs(seq1, seq2);
    }

    return aligner->align();
}

void DefaultComparingAlgorithm::setSeq1(const QByteArray &value, double _accuracy) {
    seq1 = value;
    accuracy = _accuracy;
    createIndex();
}

void DefaultComparingAlgorithm::setSeq2(const QByteArray &value) {
    seq2 = value;
}

double DefaultComparingAlgorithm::compare() {
    bool found = true;
    if (NULL != index) {
        int found = false;
        SArrayIndex::SAISearchContext context;
        int w = index->getPrefixSize();
        for (int i=0; (i <= seq2.size() - w); i++) {
            const char *s = seq2.constData() + i;
            if (index->find(&context, s)) {
                found = true;
                break;
            }
        }
    }
    if (found) {
        return compare(seq1, seq2);
    }
    return accuracy - 0.01;
}

void DefaultComparingAlgorithm::createIndex() {
    delete index; index = NULL;
    double misCount = seq1.size() * (100.0 - accuracy) / 100.0;
    int w = int( seq1.size() / (misCount + 1) );
    if (w >= MIN_INDEX_WINDOW_SIZE) {
        TaskStateInfo os;
        index = new SArrayIndex(seq1.constData(), seq1.size(), w, os);
    }
}

inline bool DefaultComparingAlgorithm::symbolsEqual(char c1, char c2) {
    if (MAlignment_GapChar == c1 || MAlignment_GapChar == c2) {
        return true;
    }
    return (c1 == c2);
}

} // SPB
