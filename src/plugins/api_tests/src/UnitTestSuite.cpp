#include "UnitTestSuite.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>

#include "core/dbi/assembly/AssemblyDbiUnitTests.h"
#include "core/dbi/attribute/AttributeDbiUnitTests.h"
#include "core/dbi/sequence/SequenceDbiUnitTests.h"
#include "core/format/genbank/LocationParserUnitTests.h"


namespace U2 {

class GTestBuilder {
public:
    bool addTestCase(const QString& testCase) {
        if (testCase.isEmpty() || tests.keys().contains(testCase)) {
            return false;
        }
        tests[testCase] = QStringList();
        return true;
    }

    bool addTest(const QString& testCase, const QString& test) {
        if (!tests.keys().contains(testCase)) {
            return false;
        }
        tests[testCase].append(test); 
        return true;
    }

	QMap<QString, QStringList> getTests() {
		return tests;
	}
private:
    QMap<QString, QStringList> tests;
};


void UnitTestSuite::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);
    GTestBuilder builder;

	passed = 0;
	ignored = 0;
	failed = 0;

    QString testList = el.text();
    QTextStream stream(&testList);
    QString line = stream.readLine();
    QString testCase;
    while (!line.isNull()) {
        line = line.trimmed();
        if (line.startsWith('+')) {
            line.remove(0,1);
            line = line.trimmed();
            builder.addTest(testCase, line);
        } else if (!line.startsWith('-') && !line.startsWith('#')) {
            testCase = line;
            builder.addTestCase(line);
		} else if (line.startsWith('-')) {
			ignored++;
		}
        line = stream.readLine();
    }
	tests = builder.getTests();
}

void UnitTestSuite::prepare() {
    QString dataDir = env->getVar("COMMON_DATA_DIR");
    AppContext::getAppSettings()->getTestRunnerSettings()->setVar("COMMON_DATA_DIR", dataDir);
	testsRun();
}
void UnitTestSuite::check_test(UnitTest *t){
	if (t != NULL) {
		t->Test();
		QString name = QString(typeid(*t).name());
		name.remove(0, name.indexOf("::") + 2);
		if (!t->GetError().isEmpty()){
			taskLog.error(name + ": " + t->GetError());
			failed++;
		} else {
			taskLog.info(name + ": OK");
			passed++;
		}
	}
}
void UnitTestSuite::testsRun(){

    foreach(const QString& suite, tests.keys()) {
        QStringList testList = tests.value(suite);
		
		if (suite == "SequenceDbiUnitTests") {
			foreach(const QString& testName, testList) {
				UnitTest* t = NULL;
				if (testName == "updateSequenceData") {
					t = new SequenceDbiUnitTests_updateSequenceData();
				} else if (testName == "getSequenceData") {
					t = new SequenceDbiUnitTests_getSequenceData();
				} else if (testName == "updateHugeSequenceData") {
					t = new SequenceDbiUnitTests_updateHugeSequenceData();
				} else if (testName == "updateSequencesData") {
					t = new SequenceDbiUnitTests_updateSequencesData();
				} else if (testName == "createSequenceObject") {
					t = new SequenceDbiUnitTests_createSequenceObject();
				} else if (testName == "getSequenceObject") {
					t = new SequenceDbiUnitTests_getSequenceObject();
				} else if (testName == "getSequenceObjectInvalid") {
					t = new SequenceDbiUnitTests_getSequenceObjectInvalid();
				} else if (testName == "updateSequenceObject") {
					t = new SequenceDbiUnitTests_updateSequenceData();
				} else if (testName == "updateAllSequenceObjects") {
					t = new SequenceDbiUnitTests_getAllSequenceObjects();
				}
				check_test(t);
			}
		} else if (suite == "AssemblyDbiUnitTests"){
			foreach(const QString& testName, testList) {
				UnitTest* t = NULL;
				if (testName == "getAssemblyObject") {
					t = new AssemblyDbiUnitTests_getAssemblyObject();
				} else if (testName == "getAssemblyObjectInvalid") {
					t = new AssemblyDbiUnitTests_getAssemblyObjectInvalid();
				} else if (testName == "countReads") {
					t = new AssemblyDbiUnitTests_countReads();
				} else if (testName == "countReadsInvalid") {
					t = new AssemblyDbiUnitTests_countReadsInvalid();
				} else if (testName == "getReads") {
					t = new AssemblyDbiUnitTests_getReads();
				} else if (testName == "getReadsInvalid") {
					t = new AssemblyDbiUnitTests_getReadsInvalid();
				} else if (testName == "getReadsByRow") {
					t = new AssemblyDbiUnitTests_getReadsByRow();
				} else if (testName == "getReadsByRowInvalid") {
					t = new AssemblyDbiUnitTests_getReadsByRowInvalid();
				} else if (testName == "getReadsByName") {
					t = new AssemblyDbiUnitTests_getReadsByName();
				} else if (testName == "getReadsByNameInvalid") {
					t = new AssemblyDbiUnitTests_getReadsByNameInvalid();
				} else if (testName == "getMaxPackedRow") {
					t = new AssemblyDbiUnitTests_getMaxPackedRow();
				} else if (testName == "getMaxPackedRowInvalid") {
					t = new AssemblyDbiUnitTests_getMaxPackedRowInvalid();
				} else if (testName == "getMaxEndPos") {
					t = new AssemblyDbiUnitTests_getMaxEndPos();
				} else if (testName == "getMaxEndPosInvalid") {
					t = new AssemblyDbiUnitTests_getMaxEndPosInvalid();
				} else if (testName == "createAssemblyObject") {
					t = new AssemblyDbiUnitTests_createAssemblyObject();
				} else if (testName == "removeReads") {
					t = new AssemblyDbiUnitTests_removeReads();
				} else if (testName == "removeReadsInvalid") {
					t = new AssemblyDbiUnitTests_removeReadsInvalid();
				} else if (testName == "addReads") {
					t = new AssemblyDbiUnitTests_addReads();
				} else if (testName == "addReadsInvalid") {
					t = new AssemblyDbiUnitTests_addReadsInvalid();
				} else if (testName == "pack") {
					t = new AssemblyDbiUnitTests_pack();
				} else if (testName == "packInvalid") {
					t = new AssemblyDbiUnitTests_packInvalid();
				} else if (testName == "calculateCoverage") {
					t = new AssemblyDbiUnitTests_calculateCoverage();
				} else if (testName == "calculateCoverageInvalid") {
					t = new AssemblyDbiUnitTests_calculateCoverageInvalid();
				}
				check_test(t);
			}
		} else if (suite == "AttributeDbiUnitTests"){
			foreach(const QString& testName, testList) {
				UnitTest* t = NULL;
				if (testName == "getAvailableAttributeNames") {
					t = new AttributeDbiUnitTests_getAvailableAttributeNames();
				} else if (testName == "getObjectAttributes") {
					t = new AttributeDbiUnitTests_getObjectAttributes();
				} else if ( testName == "getObjectAttributesByName") {
					t = new AttributeDbiUnitTests_getObjectAttributesByName();
				} else if ( testName == "getObjectPairAttributes") {
					t = new AttributeDbiUnitTests_getObjectPairAttributes();
				} else if ( testName == "getObjectPairAttributesByName") {
					t = new AttributeDbiUnitTests_getObjectPairAttributesByName();
				} else if ( testName == "removeAttributes") {
					t = new AttributeDbiUnitTests_removeAttributes();
				} else if ( testName == "removeObjectAttributes") {
					t = new AttributeDbiUnitTests_removeObjectAttributes();
				} else if ( testName == "IntegerAttribute") {
					t = new AttributeDbiUnitTests_IntegerAttribute();
				} else if ( testName == "RealAttribute") {
					t = new AttributeDbiUnitTests_RealAttribute();
				} else if ( testName == "StringAttribute") {
					t = new AttributeDbiUnitTests_StringAttribute();
				} else if ( testName == "ByteArrayAttribute") {
					t = new AttributeDbiUnitTests_ByteArrayAttribute();
				}
				check_test(t);
			}
		} else if (suite == "LocationParserTestData"){
			foreach(const QString& testName, testList) {
				UnitTest* t = NULL;
				if (testName == "locationParser") {
					t = new LocationParserTestData_locationParser();
				} else if ( testName == "locationParserInvalid") {
					t = new LocationParserTestData_locationParserInvalid();
				} else if ( testName == "hugeLocationParser") {
					t = new LocationParserTestData_hugeLocationParser();
				} else if ( testName == "buildLocationString") {
					t = new LocationParserTestData_buildLocationString();
				} else if ( testName == "buildLocationStringInvalid") {
					t = new LocationParserTestData_buildLocationStringInvalid();
				}
				check_test(t);
			}
		}
//		setError(taskLog.log())
	}
}

void UnitTestSuite::cleanup() {
    AppContext::getAppSettings()->getTestRunnerSettings()->removeVar("COMMON_DATA_DIR");

	AttributeTestData::shutdown();
	AssemblyTestData::shutdown();
	SequenceTestData::shutdown();

	if (passed){
		taskLog.info("Test passed: " + QString::number(passed));
	}
	if (ignored){
		taskLog.info("Test ignored: " + QString::number(ignored));
	}
	if (failed){
		setError("Test failed: " + QString::number(failed));
	}
}

} //namespace
