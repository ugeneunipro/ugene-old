#include "FastqUnitTests.h"

#include <U2Core/AnnotationData.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2Region.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Formats/FastqFormat.h>
#include <U2Core/AppSettings.h>
#include <U2Test/TestRunnerSettings.h>

namespace U2 {

IOAdapter* FastqFormatTestData::ioAdapter = NULL;
FastqFormat* FastqFormatTestData::format = NULL;

void FastqFormatTestData::init() {
    TestRunnerSettings* trs = AppContext::getAppSettings()->getTestRunnerSettings();
    QString originalFile = trs->getVar("COMMON_DATA_DIR") + "/tmp.fastq";
    QString tmpFile = QDir::temp().absoluteFilePath(QFileInfo(originalFile).fileName());
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    ioAdapter = iof->createIOAdapter();
    bool open = ioAdapter->open(tmpFile, IOAdapterMode_Append);
    //CHECK_EQUAL(true, open, "ioAdapter is not opened");
    format = (FastqFormat*)AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::FASTQ);
    //CHECK_NOT_EQUAL(NULL, format, "Format is NULL");
}

IMPLEMENT_TEST(FasqUnitTests, checkRawData) {
    if (FastqFormatTestData::format == NULL) {
        FastqFormatTestData::init();
    }
    QByteArray rawData = "@SEQ_ID\nGATTTGGGGTTCAAAGCAGTATCGATCAAATAGTAAATCCATTTGTTCAACTCACAGTTT\n+\n!''*((((***+))%%%++)(%%%%).1***-+*''))**55CCF>>>>>>CCCCCCC65\n";
    FormatCheckResult res = FastqFormatTestData::format->checkRawData(rawData);

    CHECK_NOT_EQUAL(FormatDetection_NotMatched, res.score,  "data is not sequence");
    bool result = res.properties[RawDataCheckResult_Sequence].toBool();
    CHECK_TRUE(result, "data is not sequence");
    result = res.properties[RawDataCheckResult_MultipleSequences].toBool();
    CHECK_FALSE(result, "sequence is multiple");
    result = res.properties[RawDataCheckResult_SequenceWithGaps].toBool();
    CHECK_FALSE(result, "sequence with gap");
}

IMPLEMENT_TEST(FasqUnitTests, checkRawDataMultiple) {
    if (FastqFormatTestData::format == NULL) {
        FastqFormatTestData::init();
    }
    QByteArray rawData = "@SEQ_ID\nGATTTGGGGTTCAAAGCAGTATCGATCAAATAGTAAATCCATTTGTTCAACTCACAGTTT\n+\n!''*((((***+))%%%++)(%%%%).1***-+*''))**55CCF>>>>>>CCCCCCC65\n";
    QByteArray rawData1 = "@SEQ_ID1\nGATTTGGGGTTCAAAGCAGTATCGATCAAATAGTAAATCCATTTGTTCAACTCACAGTTT\n+\n!''*((((***+))%%%++)(%%%%).1***-+*''))**55CCF>>>>>>CCCCCCC65\n";

    FormatCheckResult res = FastqFormatTestData::format->checkRawData(rawData + rawData1);

    CHECK_NOT_EQUAL(FormatDetection_NotMatched, res.score, "data is not sequence");
    bool result = res.properties[RawDataCheckResult_Sequence].toBool();
    CHECK_TRUE(result, "data is not sequence");
    result = res.properties[RawDataCheckResult_MultipleSequences].toBool();
    CHECK_TRUE(result , "sequence is not multiple");
    result = res.properties[RawDataCheckResult_SequenceWithGaps].toBool();
    CHECK_FALSE(result, "sequence with gap");
}

IMPLEMENT_TEST(FasqUnitTests, checkRawDataInvalidHeaderStartWith) {
    if (FastqFormatTestData::format == NULL) {
        FastqFormatTestData::init();
    }
    QByteArray rawData = "SEQ_ID\nGATTTGGGGTTCAAAGCAGTATCGATCAAATAGTAAATCCATTTGTTCAACTCACAGTTT\n+\n!''*((((***+))%%%++)(%%%%).1***-+*''))**55CCF>>>>>>CCCCCCC65\n";
    FormatCheckResult res = FastqFormatTestData::format->checkRawData(rawData);
    CHECK_EQUAL(FormatDetection_NotMatched, res.score, "format is not matched");
}

IMPLEMENT_TEST(FasqUnitTests, checkRawDataInvalidQualityHeaderStartWith) {
    if (FastqFormatTestData::format == NULL) {
        FastqFormatTestData::init();
    }
    QByteArray rawData = "@SEQ_ID\nGGGTGATGGCCGCTGCCGATGGCGTCAAATCCCACC\n-\nIIIIIIIIIIIIIIIIIIIIIIIIIIIIII9IG9IC\n";
    FormatCheckResult res = FastqFormatTestData::format->checkRawData(rawData);
    CHECK_EQUAL(FormatDetection_NotMatched, res.score, "format is not matched");
}

} //namespace
