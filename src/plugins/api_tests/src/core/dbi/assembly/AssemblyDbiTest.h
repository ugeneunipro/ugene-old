#ifndef ASSEMBLYDBITEST_H
#define ASSEMBLYDBITEST_H

#include <core/dbi/DbiTest.h>

#include <U2Core/U2ObjectDbi.h>


namespace U2 {

class AssemblyDbiTest : public BaseDbiTest {
protected:
    virtual void SetUp() {
        ASSERT_NO_FATAL_FAILURE(BaseDbiTest::SetUp());

        U2ObjectDbi *objectDbi = this->dbi->getObjectDbi();
        ASSERT_NE((U2ObjectDbi *)NULL, objectDbi);

        U2OpStatusImpl opStatus;
        assemblyIds = objectDbi->getObjects(U2Type::Assembly, 0, U2_DBI_NO_LIMIT, opStatus);
        ASSERT_FALSE(opStatus.hasError());

        assemblyDbi = this->dbi->getAssemblyDbi();
        ASSERT_NE((U2AssemblyDbi *)NULL, this->assemblyDbi);
    }

protected:
    QList<U2DataId> assemblyIds;
    U2AssemblyDbi *assemblyDbi;
};

} // namespace U2

Q_DECLARE_METATYPE(U2::U2AssemblyRead);

#endif // ASSEMBLYDBITEST_H
