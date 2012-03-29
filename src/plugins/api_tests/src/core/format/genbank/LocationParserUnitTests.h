#ifndef _U2_GENBANK_LOCATION_PARSER_UNIT_TESTS_H_
#define _U2_GENBANK_LOCATION_PARSER_UNIT_TESTS_H_

#include <unittest.h>
#include <U2Core/U2OpStatusUtils.h>

namespace U2 {

class LocationParserTestData {
public:
	void init() {};
	static void setError(const QString& err) {
		os->setError(err);
	}
protected:
	static U2OpStatus2Log* os;
};

class LocationParserTestData_locationParser : public UnitTest {
	void Test();
};

class LocationParserTestData_locationParserInvalid : public UnitTest {
	void Test();
};

class LocationParserTestData_hugeLocationParser : public UnitTest {
	void Test();
};

class LocationParserTestData_buildLocationString : public UnitTest {
	void Test();
};

class LocationParserTestData_buildLocationStringInvalid : public UnitTest {
	void Test();
};

}
#endif