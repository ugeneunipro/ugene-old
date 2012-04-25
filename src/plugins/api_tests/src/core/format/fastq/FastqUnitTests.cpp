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
	SAFE_POINT(open, "ioAdapter is not opened", );
	format = (FastqFormat*)AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::FASTQ);
}

IMPLEMENT_TEST(FasqUnitTests, checkRawData) {
	if (FastqFormatTestData::format == NULL) {
		FastqFormatTestData::init();
	}
	QByteArray rawData = "@SEQ_ID\nGATTTGGGGTTCAAAGCAGTATCGATCAAATAGTAAATCCATTTGTTCAACTCACAGTTT\n+\n!''*((((***+))%%%++)(%%%%).1***-+*''))**55CCF>>>>>>CCCCCCC65\n";
	FormatCheckResult res = FastqFormatTestData::format->checkRawData(rawData);
	SAFE_POINT(res.properties[RawDataCheckResult_Sequence] == true, "data is not sequence", );
	SAFE_POINT(res.properties[RawDataCheckResult_MultipleSequences] == false, "sequence is multiple", );
	SAFE_POINT(res.properties[RawDataCheckResult_SequenceWithGaps] == false, "sequence with gap", );
}

IMPLEMENT_TEST(FasqUnitTests, checkRawDataMultiple) {
	if (FastqFormatTestData::format == NULL) {
		FastqFormatTestData::init();
	}
	QByteArray rawData = "@SEQ_ID\nGATTTGGGGTTCAAAGCAGTATCGATCAAATAGTAAATCCATTTGTTCAACTCACAGTTT\n+\n!''*((((***+))%%%++)(%%%%).1***-+*''))**55CCF>>>>>>CCCCCCC65\n";
	QByteArray rawData1 = "@SEQ_ID1\nGATTTGGGGTTCAAAGCAGTATCGATCAAATAGTAAATCCATTTGTTCAACTCACAGTTT\n+\n!''*((((***+))%%%++)(%%%%).1***-+*''))**55CCF>>>>>>CCCCCCC65\n";

	FormatCheckResult res = FastqFormatTestData::format->checkRawData(rawData + rawData1);
	SAFE_POINT(res.properties[RawDataCheckResult_Sequence] == true, "data is not sequence", );
	SAFE_POINT(res.properties[RawDataCheckResult_MultipleSequences] == true, "sequence is multiple", );
	SAFE_POINT(res.properties[RawDataCheckResult_SequenceWithGaps] == false, "sequence with gap", );
}

IMPLEMENT_TEST(FasqUnitTests, checkRawDataInvalidHeaderStartWith) {
	if (FastqFormatTestData::format == NULL) {
		FastqFormatTestData::init();
	}
	QByteArray rawData = "SEQ_ID\nGATTTGGGGTTCAAAGCAGTATCGATCAAATAGTAAATCCATTTGTTCAACTCACAGTTT\n+\n!''*((((***+))%%%++)(%%%%).1***-+*''))**55CCF>>>>>>CCCCCCC65\n";
	FormatCheckResult res = FastqFormatTestData::format->checkRawData(rawData);
	SAFE_POINT(res.score == FormatDetection_NotMatched, "format is not matched", );
}

IMPLEMENT_TEST(FasqUnitTests, checkRawDataInvalidQualityHeaderStartWith) {
	if (FastqFormatTestData::format == NULL) {
		FastqFormatTestData::init();
	}
	QByteArray rawData = "@SEQ_ID\nGGGTGATGGCCGCTGCCGATGGCGTCAAATCCCACC\n-\nIIIIIIIIIIIIIIIIIIIIIIIIIIIIII9IG9IC\n";
	FormatCheckResult res = FastqFormatTestData::format->checkRawData(rawData);
	SAFE_POINT(res.score == FormatDetection_NotMatched, "format is not matched", );
}

} //namespace
