#ifndef _U2_UNIT_TEST_H_
#define _U2_UNIT_TEST_H_

#include <QString.h>

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
