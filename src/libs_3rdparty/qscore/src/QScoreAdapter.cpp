#include "QScoreAdapter.h"
#include "qscore/qscore.h"

extern double QScore(MSA_QScore* _msaTest, MSA_QScore* _msaRef);

namespace U2 {
  
  static void convertMAlignment2MSA(MSA_QScore& muscleMSA, const MAlignment& ma){

    for (int i=0, n = ma.getNumRows(); i<n; i++) {
      const MAlignmentRow& aseq = ma.getRow(i);

      char* seq  = new char[aseq.getCoreLength()+1];
      memcpy(seq, aseq.getCore().constData(), aseq.getCoreLength());
      seq[aseq.getCoreLength()] = '\0';

      char* name = new char[aseq.getName().length() + 1];
      memcpy(name, aseq.getName().toLocal8Bit().data(), aseq.getName().length());
      name[aseq.getName().length()] = '\0';

      muscleMSA.AppendSeq(seq, aseq.getCoreLength(), name);
    }
  }

  double QScore(const MAlignment& maTest, const MAlignment& maRef, TaskStateInfo& ti) {
    MSA_QScore msaTest, msaRef;
    try {
      convertMAlignment2MSA(msaTest, maTest);
      convertMAlignment2MSA(msaRef, maRef);
      return QScore(&msaTest, &msaRef);
    } catch(...) {
      ti.setError(QString("QScore throws exception: "));
      return 0;
    }

  }
} // namespace U2 
