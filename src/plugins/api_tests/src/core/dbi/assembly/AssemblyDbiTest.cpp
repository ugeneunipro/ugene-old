#include <U2Core/U2OpStatusUtils.h>
#include "AssemblyDbiTest.h"

#include <U2Core/U2AssemblyDbi.h>


namespace U2 {

template<class DBI_TYPE_PARAM>
class Test_U2AssemblyDbi_getAssemblyObject : public AssemblyDbiTest<DBI_TYPE_PARAM> {
public:
    typedef DBI_TYPE_PARAM DBI_TYPE;
protected:
    virtual void SetUp() {
        ASSERT_NO_FATAL_FAILURE(AssemblyDbiTest<DBI_TYPE>::SetUp());
        assemblyDbi = this->dbi->getAssemblyDbi();
        ASSERT_NE((U2AssemblyDbi *)NULL, this->assemblyDbi);
    }
    virtual void TearDown() {
        EXPECT_NO_FATAL_FAILURE(AssemblyDbiTest<DBI_TYPE>::TearDown());
    }
protected:
    U2AssemblyDbi *assemblyDbi;
};
typedef ::testing::Types</*BAMDbiType*/SQLiteDbiType> TypesToTest;

TYPED_TEST_CASE(Test_U2AssemblyDbi_getAssemblyObject, TypesToTest);

TYPED_TEST(Test_U2AssemblyDbi_getAssemblyObject, t02) {
        U2OpStatusImpl opStatus;
        this->assemblyDbi->getAssemblyObject(this->assemblyId, opStatus);
        ASSERT_FALSE(opStatus.hasError()) << opStatus.getError().toStdString();
}

} // namespace U2
