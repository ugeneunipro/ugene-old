#include "MSADistanceAlgorithmHammingRevCompl.h"

#include <U2Core/MAlignment.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/TextUtils.h>

namespace U2 {

MSADistanceAlgorithmFactoryHammingRevCompl::MSADistanceAlgorithmFactoryHammingRevCompl(QObject* p) 
: MSADistanceAlgorithmFactory(BuiltInDistanceAlgorithms::HAMMING_REVCOMPL_ALGO, DistanceAlgorithmFlag_Nucleic, p)
{

}


QString MSADistanceAlgorithmFactoryHammingRevCompl::getDescription() const {
    return tr("Based on Hamming distance between two sequences");
}

QString MSADistanceAlgorithmFactoryHammingRevCompl::getName() const {
    return tr("Hamming reverse-complement");
}


MSADistanceAlgorithm* MSADistanceAlgorithmFactoryHammingRevCompl::createAlgorithm(const MAlignment& ma, QObject* p) {
    return new MSADistanceAlgorithmHammingRevCompl(this, ma);
}

//////////////////////////////////////////////////////////////////////////
// Algorithm

void MSADistanceAlgorithmHammingRevCompl::run() {
    QList<DNATranslation*> compTTs = AppContext::getDNATranslationRegistry()->
        lookupTranslation(ma.getAlphabet(), DNATranslationType_NUCL_2_COMPLNUCL);

    assert (!compTTs.isEmpty());

    DNATranslation* trans = compTTs.first();
    int nSeq = ma.getNumRows();
    MAlignment revtransl;
    revtransl.setAlphabet(ma.getAlphabet());
    for (int i = 0; i < nSeq; i++) {
        QByteArray arr = ma.getRow(i).toByteArray(ma.getLength());
        trans->translate(arr.data(), arr.length());
        TextUtils::reverse(arr.data(), arr.length());
        revtransl.addRow(MAlignmentRow(ma.getRow(i).getName(), arr));
    }

    for (int i = 0; i < nSeq; i++) {
        for (int j = i; j < nSeq; j++) {
            int sim = 0;
            for (int k = 0; k < ma.getLength(); k++) {
                if (ma.charAt(i, k) == revtransl.charAt(j, k)) sim++;
            }
            lock.lock();
            distanceTable[i][j] = distanceTable[j][i] = sim;
            lock.unlock();
        }
    }
}

} //namespace
