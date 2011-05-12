#ifndef ASSEMBLYDBITEST_H
#define ASSEMBLYDBITEST_H

#include <gtest/gtest.h>
#include "DbiTest.h"

#include <U2Core/U2ObjectDbi.h>


namespace U2 {

/**
 * Base class for AssemblyDbiTests.
 *
 * Creates and initializes a new DBI instance in SetUp() and shutdowns and deletes it in TearDown().
 *
 * @param DBI_TYPE_PARAM Type of the DBI.
 */
template<class DBI_TYPE_PARAM>
class AssemblyDbiTest : public InitializedDbiTest<DBI_TYPE_PARAM>
{
public:
    typedef DBI_TYPE_PARAM DBI_TYPE;
protected:
    /**
     * Creates and initializes a new DBI instance and retrieves an assembly object.
     */
    virtual void SetUp() {
        ASSERT_NO_FATAL_FAILURE(InitializedDbiTest<DBI_TYPE>::SetUp());
        QList<U2DataId> assemblyIds;
        {
            U2ObjectDbi *objectRDbi = this->dbi->getObjectDbi();
            ASSERT_NE((U2ObjectDbi *)NULL, objectRDbi);
            U2OpStatusImpl opStatus;
            assemblyIds = objectRDbi->getObjects(U2Type::Assembly, 0, U2_DBI_NO_LIMIT, opStatus);
            ASSERT_FALSE(opStatus.hasError());
            //ASSERT_GE(assemblyIds.size(), 1);
        }
        if (!assemblyIds.isEmpty()) {
            assemblyId = assemblyIds.front();
        }
    }

    /**
     * Shutdowns and deletes the DBI instance.
     */
    virtual void TearDown() {
        ASSERT_NO_FATAL_FAILURE(InitializedDbiTest<DBI_TYPE>::TearDown());
    }

protected:
    /**
     * An id of an assembly object for testing.
     */
    U2DataId assemblyId;
};

} // namespace U2

#endif // ASSEMBLYDBITEST_H
