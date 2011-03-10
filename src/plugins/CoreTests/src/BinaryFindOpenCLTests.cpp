#include "BinaryFindOpenCLTests.h"

#include <U2Core/AppContext.h>

#include <QtCore/QStringList>

namespace U2 {

#define NUMBERS "numbers"
#define FIND_NUMBERS "find_numbers"
#define EXPECTED_RESULTS "expected_res"

QList< XMLTestFactory* > BinaryFindOpenCLTests::createTestFactories(){
    QList< XMLTestFactory* > res;
    res.append( GTest_BinaryFindOpenCL::createFactory() );
    return res;
}

void GTest_BinaryFindOpenCL::init(XMLTestFormat *tf, const QDomElement& el){
    Q_UNUSED(tf);

    QString buf;

    buf = el.attribute(NUMBERS);
    if (!buf.isEmpty()){
        QStringList numsStr = buf.split(",");
        foreach(QString s, numsStr) {
            numbers.append(s.toLongLong());
        }
    } else {
        failMissingValue(NUMBERS);
    }

    buf = el.attribute(FIND_NUMBERS);
    if (!buf.isEmpty()){
        QStringList numsStr = buf.split(",");
        foreach(QString s, numsStr) {
            findNumbers.append(s.toLongLong());
        }
    } else {
        failMissingValue(FIND_NUMBERS);
    }

    buf = el.attribute(EXPECTED_RESULTS);
    if (!buf.isEmpty()){
        QStringList numsStr = buf.split(",");
        foreach(QString s, numsStr) {
            expectedResults.append(s.toInt());
        }
    } else {
        failMissingValue(EXPECTED_RESULTS);
    }
}

void GTest_BinaryFindOpenCL::prepare() {
    if (AppContext::getOpenCLGpuRegistry()->getAnyEnabledGpu() == 0) {
        stateInfo.setError(QString("No enabled gpu's found"));
    } else {
        BinaryFindOpenCL bf(numbers.constData(), numbers.size(), findNumbers.constData(), findNumbers.size());
        results = bf.launch();
    }
}

Task::ReportResult GTest_BinaryFindOpenCL::report(){
//    QString buf = "";
//    for (int i = 0; i < findNumbers.size(); i ++ ) {
//        buf += QString::number(results[i]) + " ";
//    }
//    coreLog.info("!!!!!!!!!!!!! result !!!!!!!!!!!!!");
//    coreLog.info(buf);
    for (int i = 0; i < findNumbers.size(); i ++ ) {
        if ( (expectedResults.at(i) == 0 && results[i] != -1) ||
             (expectedResults.at(i) != 0 && results[i] == -1) ||
             (results[i] != -1 && numbers[results[i]] != findNumbers.at(i))) {
            stateInfo.setError(QString("expectedIsContains: %1 value: %2 does not satisfy the resulting position: %3").arg(expectedResults.at(i)).arg(findNumbers.at(i)).arg(results[i]));
            break;
        }
    }
    delete[] results;
    return ReportResult_Finished;
}

} //namespace

