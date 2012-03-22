#ifndef _U2_UNIT_TEST_H_
#define _U2_UNIT_TEST_H_

namespace U2 {

class UnitTest {
public:
    virtual void Test() = 0;
	virtual void SetUp() {};
	virtual void TearDown() {};
};

} // namespace

#endif
