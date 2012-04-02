#ifndef _U2_GENBANK_LOCATION_PARSER_UNIT_TESTS_H_
#define _U2_GENBANK_LOCATION_PARSER_UNIT_TESTS_H_

#include <unittest.h>
#include <U2Core/U2OpStatusUtils.h>
#include <QMetaType>
namespace U2 {

class LocationParserTestData {
public:
	void init() {};
protected:
	static bool registerTest;
};

class LocationParserTestData_locationParser : public UnitTest {
public: 
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

Q_DECLARE_METATYPE(U2::LocationParserTestData_locationParser);
Q_DECLARE_METATYPE(U2::LocationParserTestData_locationParserInvalid);
Q_DECLARE_METATYPE(U2::LocationParserTestData_hugeLocationParser);
Q_DECLARE_METATYPE(U2::LocationParserTestData_buildLocationString);
Q_DECLARE_METATYPE(U2::LocationParserTestData_buildLocationStringInvalid);

#endif