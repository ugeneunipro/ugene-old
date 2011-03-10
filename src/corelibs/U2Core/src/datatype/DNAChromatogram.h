#ifndef _U2_DNA_CHROMATOGRAM_H_
#define _U2_DNA_CHROMATOGRAM_H_

#include <QtCore/QVector>
#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT DNAChromatogram {
public:
    DNAChromatogram() : traceLength(0), seqLength(0), hasQV(false) {}
    int traceLength;
    int seqLength;
    QVector<ushort> baseCalls;
    QVector<ushort> A;
    QVector<ushort> C;
    QVector<ushort> G;
    QVector<ushort> T;
    QVector<char> prob_A;
    QVector<char> prob_C;
    QVector<char> prob_G;
    QVector<char> prob_T;
    bool hasQV;
};

} //namespace

#endif
