#include "LocationParserUnitTests.h"

#include <U2Formats/GenbankLocationParser.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2Region.h>

#include <QStringList>

namespace U2 {

U2OpStatus2Log*	LocationParserTestData::os = new U2OpStatus2Log();

void LocationParserTestData_locationParser::Test() {
	QString regionStr = "0..0";
    U2Location location;
    Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    QVector<U2Region> regions = location->regions;
	CHECK_EXT(regions.size() == 1, SetError(QString("regions size should be 1")), );

	regionStr = "0..10,10..12";
    Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    regions = location->regions;

	CHECK_EXT(regions.size() == 2, SetError(QString("regions size should be 2")), );
}

void LocationParserTestData_locationParserInvalid::Test() {
	QString regionStr = "-1..-10";
    U2Location location;
    Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    QVector<U2Region> regions = location->regions;
	CHECK_EXT(regions.size() == 0, SetError(QString("regions size should be 0")), );

	regionStr = "0.10,15-20,30..0xFF";
    Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    regions = location->regions;
	CHECK_EXT(regions.size() == 0, SetError(QString("regions size should be 0")), );
}

void LocationParserTestData_hugeLocationParser::Test() {
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

	CHECK_EXT(regions.size() == i, SetError(QString("regions size should be " + QString::number(i))), );
}

void LocationParserTestData_buildLocationString::Test() {
    AnnotationData ad;
	qint64 region_length = 100;
	for (int i = 0; i < 1000; i++){
		ad.location->regions << U2Region((region_length -1) * i, region_length);
	}
    QString regionStr = Genbank::LocationParser::buildLocationString(&ad);
	CHECK_EXT(regionStr.length() > 0, SetError(QString("regions string should not be empty")), );

    QStringList regions = regionStr.split(",");
	CHECK_EXT(regions.size() == ad.location->regions.size(), SetError(QString("incorrect expected regions size")), );
}

void LocationParserTestData_buildLocationStringInvalid::Test() {
    AnnotationData ad;
	qint64 region_length = 100;
	for (int i = 0; i < 10; i++){
		ad.location->regions << U2Region(-(region_length -1) * i, -region_length);
	}
    QString regionStr = Genbank::LocationParser::buildLocationString(&ad);
    QStringList regions = regionStr.split(",");

    U2Location location;
	Genbank::LocationParser::parseLocation(qPrintable(regionStr),regionStr.length(), location);
    QVector<U2Region> expected = location->regions;
	CHECK_EXT(regions.size() == expected.size(), SetError(QString("incorrect expected regions size")), );
}

} //namespace