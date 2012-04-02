#ifndef _U2_UNIT_TEST_SUITE_H_
#define _U2_UNIT_TEST_SUITE_H_

#include <U2Test/XMLTestUtils.h>

namespace U2 {

class UnitTestSuite : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(UnitTestSuite, "unittest", TaskFlags_NR_FOSCOE);

	void prepare();
    void cleanup();

protected:
	void tests_run();
	void test_run(const QString& testName);

    QMap<QString, QStringList> tests;
private:
	int passed;
	int ignored;
	int failed;
};

} // namespace

#endif
