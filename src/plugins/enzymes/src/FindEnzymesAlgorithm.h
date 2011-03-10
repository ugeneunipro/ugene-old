#ifndef _U2_FIND_ENZYMES_ALGO_H_
#define _U2_FIND_ENZYMES_ALGO_H_

#include "EnzymeModel.h"

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequence.h>

#include <QtCore/QObject>
#include <QtCore/QList>

namespace U2 {

class FindEnzymesAlgListener {
public:
    ~FindEnzymesAlgListener(){}
    virtual void onResult(int pos, const SEnzymeData& enzyme) = 0;
};


template <typename CompareFN>
class FindEnzymesAlgorithm {
public:
    void run(const DNASequence& sequence, const U2Region& range, const SEnzymeData& enzyme, FindEnzymesAlgListener* l, TaskStateInfo& ti) {
        CompareFN fn(sequence.alphabet, enzyme->alphabet);
        const char* seq = sequence.constData();
        const char* pattern = enzyme->seq.constData();
        char unknownChar = sequence.alphabet->getDefaultSymbol();
        int plen = enzyme->seq.length();
        for (int s=range.startPos, n = range.endPos() - plen + 1; s < n && !ti.cancelFlag; s++) {
            bool match = true;
            for (int p=0; p < plen && match; p++) {
                char c1 = seq[s + p];
                char c2 = pattern[p];
                match = c1!=unknownChar && fn.equals(c1, c2);
            }
            if (match) {
                l->onResult(s, enzyme);
            }
        }
        if (sequence.circular) {
            if ( range.startPos + range.length == sequence.length() ) {
                QByteArray buf;
                const QByteArray& dnaseq = sequence.seq;
                int size = enzyme->seq.size() - 1;
                int startPos = dnaseq.length() - size;
                buf.append(dnaseq.mid(startPos));
                buf.append(dnaseq.mid(0, size));
                for (int s = 0; s < size; s++) {
                    bool match = true;
                    for (int p = 0; p < plen && match; p++) {
                        char c1 = buf[s + p];
                        char c2 = pattern[p];
                        match = c1!=unknownChar && fn.equals(c1,c2);
                    }
                    if (match) {
                        l->onResult(s + startPos, enzyme);
                    }
                }
                
            } 
        }
    }
};

} //namespace

#endif
