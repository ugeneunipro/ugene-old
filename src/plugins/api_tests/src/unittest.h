#ifndef _U2_UNIT_TEST_H_
#define _U2_UNIT_TEST_H_

#include <QString>

#define _STR(c) #c
#define TEST_CLASS(suite, name)          suite##_##name
#define TEST_CLASS_STR(suite, name) _STR(suite##_##name)

/** Macros for writing tests without repeating too much */

/** For each test place this in header file, inside namespace U2 to declare test class */
#define DECLARE_TEST(suite, name) \
    class TEST_CLASS(suite, name) : public UnitTest { \
    public: \
        virtual void Test(); \
    }

/** For each test place this in header file, outside (!) namespace U2 to register Qt meta type */
#define DECLARE_METATYPE(suite, name) \
    Q_DECLARE_METATYPE(U2::suite##_##name)

/** Place this in cpp file and define test body in subsequent {} block */
#define IMPLEMENT_TEST(suite, name) \
    static int _##suite##_##name##_type = qRegisterMetaType<U2::TEST_CLASS(suite, name)>(TEST_CLASS_STR(suite, name)); \
    void TEST_CLASS(suite, name)::Test()

/** Macros to be used in test body: they all check for some condition,
    and if check fails then set test error and break its execution */

#define CHECK_NO_ERROR(os) CHECK_OP_EXT(os, SetError(os.getError()),)
#define CHECK_TRUE(condition, error) CHECK_EXT(condition, SetError(error),)
#define CHECK_EQUAL(expected, actual, what) CHECK_TRUE(expected == actual, QString("%1: expected '%2', got '%3'").arg(what).arg(QString(expected)).arg(QString(actual)))
#define CHECK_NOT_EQUAL(notExpected, actual, what) CHECK_TRUE(notExpected != actual, QString("%1: expected not to be'%2', but got '%3'").arg(what).arg(QString(notExpected)).arg(QString(actual)))

namespace U2 {

class UnitTest {
public:
    virtual void Test() = 0;
	virtual void SetUp() {};
	virtual void TearDown() {};
	virtual void SetError(const QString& err) {	error = err; }
	virtual QString GetError() { return error; }
protected:
	QString error;
};

} // namespace

#endif
