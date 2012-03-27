#ifndef _U2_ASSEMBLY_DBI_UNIT_TESTS_H_
#define _U2_ASSEMBLY_DBI_UNIT_TESTS_H_

#include "core/dbi/DbiTest.h"

#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Test/TestRunnerSettings.h>
#include <unittest.h>

#include <memory>

namespace U2 {

class AssemblyTestData {
public:
    static void init();
    static void shutdown();
	static U2AssemblyDbi* getAssemblyDbi();
	static void close();
	static QList<U2DataId>* getAssemblyIds() { return assemblyIds; };

public:
    static U2AssemblyDbi* assemblyDbi;
    static QList<U2DataId>* assemblyIds;

	static const QString& ASS_DB_URL;

protected:
    static TestDbiProvider dbiProvider;
};

class AssemblyDbiUnitTests_getAssemblyObject : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getAssemblyObjectInvalid : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_countReads : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_countReadsInvalid : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getReads : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getReadsInvalid : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getReadsByRow : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getReadsByRowInvalid : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getReadsByName : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getReadsByNameInvalid : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getMaxPackedRow : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getMaxPackedRowInvalid : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getMaxEndPos : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_getMaxEndPosInvalid : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_createAssemblyObject : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_removeReads : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_removeReadsInvalid : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_addReads : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_addReadsInvalid : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_pack : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_packInvalid : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_calculateCoverage : public UnitTest {
public:
    void Test();
};

class AssemblyDbiUnitTests_calculateCoverageInvalid : public UnitTest {
public:
    void Test();
};

}
#endif //_U2_ASSEMBLY_DBI_UNITTESTS_H_