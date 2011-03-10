#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>

#include <U2View/SecStructPredictUtils.h>
#include <U2Core/BioStruct3D.h>
#include <U2Core/Counter.h>
#include "PsipredAlgTask.h"
#include "sspred_avpred.h"
#include "sspred_hmulti.h"
#include "sspred_utils.h"

namespace U2 {

const QString PsipredAlgTask::taskName(tr("PsiPred"));
QMutex PsipredAlgTask::runLock;

PsipredAlgTask::PsipredAlgTask(const QByteArray& inputSeq) : SecStructPredictTask(inputSeq)
{
    GCOUNTER( cvar, tvar, "PsipredAlgTask" );
}

#define PSIPRED_ANNOTATION_NAME "psipred_results"

void PsipredAlgTask::run() 
{
    QMutexLocker runLocker( &runLock ); //TODO: BUG-0000808
    //TODO: get rid of this limit
    const int MAXSIZE = 10000;
    if (sequence.size() > MAXSIZE) {
        stateInfo.setError(SecStructPredictTask::tr("psipred: sequence is too long, max seq size is 10000"));
        return;
    }

    QTemporaryFile matrixFile; 
    
    seq2mtx(sequence.constData(), sequence.length(), &matrixFile);
    matrixFile.reset();
    
    {
        QStringList weightFileNames;
        weightFileNames << ":psipred/datafiles/weights_s.dat";
        weightFileNames << ":psipred/datafiles/weights_s.dat2";
        weightFileNames << ":psipred/datafiles/weights_s.dat3";

        PsiPassOne pass1 (&matrixFile, weightFileNames);
        pass1.runPsiPass();
    }

    const char* psipass2_args[] = 
    { 
        "empty", 
        ":psipred/datafiles/weights_p2.dat", "1", "1.0", "1.0", 
        "output.ss2", 
        "output.ss"
    };
    
    {
        PsiPassTwo pass2;
        pass2.runPsiPass(7, psipass2_args, output);
    }

    results = SecStructPredictUtils::saveAlgorithmResultsAsAnnotations(output, PSIPRED_ANNOTATION_NAME);
    
    QDir curDir;
    curDir.remove("output.ss");
    curDir.remove("output.ss2");
}



} //namespace



