#include "SMatrixTests.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Algorithm/SubstMatrixRegistry.h>

#include <U2Core/SMatrix.h>


namespace U2 {

#define FILE_ATTR "file"
#define COL_ATTR "column"
#define ROW_ATTR "row"
#define VALUE_ATTR "val"
#define ALPHABET_ATTR "alphabet"


QList< XMLTestFactory* > SMatrixTests::createTestFactories(){
    QList< XMLTestFactory* > res;
    res.append( GTest_SubstMatrix::createFactory() );
    return res;
}

void GTest_SubstMatrix::init(XMLTestFormat *tf, const QDomElement& el){
    Q_UNUSED(tf);
    QString buf;
    bool isOk;
    buf = el.attribute(FILE_ATTR);
    if (!buf.isEmpty()){
        file=buf;
    }

    buf = el.attribute(COL_ATTR);
    if (!buf.isEmpty()){
        col = buf.at(0).toAscii();
    }

    buf = el.attribute(ROW_ATTR);
    if (!buf.isEmpty()){
        row = buf.at(0).toAscii();
    }
    
    buf = el.attribute(VALUE_ATTR);
    if (!buf.isEmpty()){
        float tmp = buf.toFloat(&isOk);
        if(!isOk){
            return;
        }
        expectedVal = tmp;
    }

    buf = el.attribute(ALPHABET_ATTR);
    if (!buf.isEmpty()){
        expectedAlphabetId = buf;
    }
}

Task::ReportResult GTest_SubstMatrix::report(){
    SubstMatrixRegistry *r = AppContext::getSubstMatrixRegistry();
    if((r->getMatrixNames()).indexOf(file) == -1){
        stateInfo.setError(tr("Matrix with %1 name not found").arg(file));
        return ReportResult_Finished;
    }
    SMatrix m = r->getMatrix(file);
    float actualScore = m.getScore(row, col);
    if(expectedVal != actualScore){
        stateInfo.setError(tr("Actual score %1 not equal with expected %2").arg(actualScore).arg(expectedVal));
        return ReportResult_Finished;
    }
    if(m.getAlphabet()->getId() != expectedAlphabetId){
        stateInfo.setError(tr("Actual alphabet id %1 not equal with expected %2").arg(m.getAlphabet()->getId()).arg(expectedAlphabetId));
    }    
    return ReportResult_Finished;
}

}