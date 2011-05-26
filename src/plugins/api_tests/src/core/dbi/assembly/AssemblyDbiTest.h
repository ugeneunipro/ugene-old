#ifndef ASSEMBLYDBITEST_H
#define ASSEMBLYDBITEST_H

#include <gtest/gtest.h>
#include "DbiTest.h"

#include <U2Core/U2ObjectDbi.h>


namespace U2 {

class AssemblyDbiTest : public BaseDbiTest {
protected:
    virtual void SetUp();
protected:
    QList<U2DataId> assemblyIds;
    U2AssemblyDbi *assemblyDbi;
};

} // namespace U2

#endif // ASSEMBLYDBITEST_H
