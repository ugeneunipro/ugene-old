#ifndef _U2_FASTQ_UNIT_TESTS_H_
#define _U2_FASTQ_UNIT_TESTS_H_

#include <unittest.h>
#include <U2Core/U2OpStatusUtils.h>

namespace U2 {
class IOAdapter;
class FastqFormat;

class FastqFormatTestData {
public:
	static void init();
	static void shutdown();

	static IOAdapter* ioAdapter;
	static FastqFormat* format;
};

DECLARE_TEST(FasqUnitTests, checkRawData);
DECLARE_TEST(FasqUnitTests, checkRawDataInvalidHeaderStartWith);
DECLARE_TEST(FasqUnitTests, checkRawDataInvalidQualityHeaderStartWith);
DECLARE_TEST(FasqUnitTests, checkRawDataMultiple);

}

DECLARE_METATYPE(FasqUnitTests, checkRawData);
DECLARE_METATYPE(FasqUnitTests, checkRawDataInvalidHeaderStartWith);
DECLARE_METATYPE(FasqUnitTests, checkRawDataInvalidQualityHeaderStartWith);
DECLARE_METATYPE(FasqUnitTests, checkRawDataMultiple);

#endif

