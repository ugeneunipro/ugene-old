#include "LocationParserUnitTests.h"

#include <U2Core/AnnotationData.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2Region.h>
#include <U2Formats/GenbankLocationParser.h>

namespace U2 {

IMPLEMENT_TEST(LocationParserTestData, locationParser) {
	QString regionStr = "0..0";
    U2Location location;
    Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(regions.size(), 1, "regions size should be 1");

	regionStr = "0..10,10..12";
    Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    regions = location->regions;

	CHECK_EQUAL(regions.size(), 2, "regions size should be 2");
}
IMPLEMENT_TEST(LocationParserTestData, locationParserEmpty) {
	QString regionStr;
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(regions.size(), 0, "regions size should be 1");
}


IMPLEMENT_TEST(LocationParserTestData, locationParserCompare) {
	QString original = "0..10";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(original),original.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(regions.size(), 1, "regions size should be 1");
	QString expected = Genbank::LocationParser::buildLocationString(regions);
	CHECK_TRUE(original == expected, "regions should be the same");
}

IMPLEMENT_TEST(LocationParserTestData, locationParserDuplicate) {
	QString regionStr = "0..10,0..10,0..10";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(3, regions.size(), "regions size should be 1");
}


IMPLEMENT_TEST(LocationParserTestData, locationParserInvalid) {
	QString regionStr = "-10..9";
    U2Location location;
    Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "regions size should be 0");

	regionStr = "0.10,15-20,30..0xFF";
    Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    regions = location->regions;
	CHECK_EQUAL(regions.size(), 0, "regions size should be 0");
}

IMPLEMENT_TEST(LocationParserTestData, hugeLocationParser) {
    U2Location location;
	QString regionStr = "";
	qint64 i;
	for (i = 0; i < 10000000; i ++) {
		regionStr += QString::number(i*10) + ".." + QString::number(10*(i+1));
		if ( i < 10000000 - 1) {
			regionStr += + ",";
		}
	}

    Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(regions.size(), i, "regions size should be " + QString::number(i));

	AnnotationData ad;
	ad.location->regions = regions;
	QString expectedStr = Genbank::LocationParser::buildLocationString(&ad);
	CHECK_TRUE(expectedStr.length() > 0, "regions string should not be empty");
	QStringList expected = expectedStr.split(",");
	CHECK_EQUAL(expected.size(), ad.location->regions.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, locationParserComplement) {	
	QString regionStr = "complement(0..0)";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(regions.size(), 1, "regions size should be 1");

	regionStr = "complement(0..10,10..12)";
	Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
	regions = location->regions;

	CHECK_EQUAL(regions.size(), 2, "regions size should be 2");
}

IMPLEMENT_TEST(LocationParserTestData, locationParserComplementInvalid) {	
	QString regionStr = "complement{0..0)";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "regions size should be 1");

	regionStr = "complement(0..10,10..12}";
	Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
	regions = location->regions;

	CHECK_EQUAL(0, regions.size(), "regions size should be 2");
}

IMPLEMENT_TEST(LocationParserTestData, buildLocationString) {
    AnnotationData ad;
	qint64 region_length = 100;
	for (int i = 0; i < 1000; i++){
		ad.location->regions << U2Region((region_length -1) * i, region_length);
	}
    QString regionStr = Genbank::LocationParser::buildLocationString(&ad);
	CHECK_TRUE(regionStr.length() > 0, "regions string should not be empty");

    QStringList regions = regionStr.split(",");
	CHECK_EQUAL(regions.size(), ad.location->regions.size(), "incorrect expected regions size");

	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
	QVector<U2Region> expected = location->regions;
	CHECK_EQUAL(regions.size(), expected.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, buildLocationStringDuplicate) {
	AnnotationData ad;
	qint64 region_length = 100;
	for (int i = 0; i < 10; i++){
		ad.location->regions << U2Region(1, region_length);
	}
	QString regionStr = Genbank::LocationParser::buildLocationString(&ad);
	CHECK_TRUE(regionStr.length() > 0, "regions string should not be empty");

	QStringList regions = regionStr.split(",");
	CHECK_EQUAL(regions.size(), ad.location->regions.size(), "incorrect expected regions size");

	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
	QVector<U2Region> expected = location->regions;
	CHECK_EQUAL(regions.size(), expected.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, buildLocationStringInvalid) {
    AnnotationData ad;
	qint64 region_length = 10;
	for (int i = 0; i < 10; i++){
		ad.location->regions << U2Region(-region_length * i, -region_length);
	}
    QString regionStr = Genbank::LocationParser::buildLocationString(&ad);
    QStringList regions = regionStr.split(",");

    U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    QVector<U2Region> expected = location->regions;
	CHECK_EQUAL(expected.size(), regions.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, locationOperatorJoin) {
	AnnotationData ad;
	ad.setLocationOperator(U2LocationOperator_Join);

	qint64 region_length = 10;
	for (int i = 0; i < 10; i++){
		ad.location->regions << U2Region((region_length -1) * i, region_length);
	}
	QString regionStr = Genbank::LocationParser::buildLocationString(&ad);
	CHECK_TRUE(regionStr.length() > 0, "regions string should not be empty");
	CHECK_TRUE(regionStr.startsWith("join"), "regions join string must start with <join>");
}


IMPLEMENT_TEST(LocationParserTestData, locationOperatorJoinInvalid) {
	QString str = "join 1..10,11..10)";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, locationOperatorOrder) {
	AnnotationData ad;
	ad.setLocationOperator(U2LocationOperator_Order);

	qint64 region_length = 100;
	for (int i = 0; i < 10; i++){
		ad.location->regions << U2Region((region_length -1) * i, region_length);
	}
	QString regionStr = Genbank::LocationParser::buildLocationString(&ad);
	CHECK_TRUE(regionStr.length() > 0, "regions string should not be empty");
	CHECK_TRUE(regionStr.startsWith("order"), "regions join string must start with order");
}

IMPLEMENT_TEST(LocationParserTestData, locationOperatorOrderInvalid) {
	QString str = "order(1..10,11..10";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, locationParserParenthesis) {
	QString str = "join(1..10,11..10)";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(2, regions.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, locationParserParenthesisInvalid) {
	QString str = "join((1..10,11..10),(31..10))";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, locationParserLeftParenthesisMissed) {
	QString str = "join 1..10)";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");

	str = "join(1..10))";
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");

	str = "join(1..10))))))))))";
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");

}

IMPLEMENT_TEST(LocationParserTestData, locationParserRightParenthesisMissed) {
	QString str = "oreder(1..10";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");

	str = "order((1..10)";
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");

	str = "order((((((((((1..10)";
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, locationParserPeriodInvalid) {
	QString str = "join(.,.,.)";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, locationParserDoublePeriodInvalid) {
	QString str = "join(..,..,..)";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");
}

IMPLEMENT_TEST(LocationParserTestData, locationParserCommaInvalid) {
	QString str = "join(,,,)";
	U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(str),str.length(), location);
	QVector<U2Region> regions = location->regions;
	CHECK_EQUAL(0, regions.size(), "incorrect expected regions size");
}

} //namespace