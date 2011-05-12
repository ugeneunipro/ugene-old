#ifndef DBITEST_H
#define DBITEST_H

#include <memory>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>

#include <gtest/gtest.h>

namespace U2 {

/**
 * DBI type descriptor for BAMDbi.
 */
class BAMDbiType {
public:
    /**
     * Id of BAMDbi.
     */
    static const char *ID;
};

/**
 * DBI type descriptor for FileDbi.
 */
class FileDbiType {
public:
    /**
     * Id of FileDbi.
     */
    static const char *ID;
};

/**
 * DBI type descriptor for SQLiteDbi.
 */
class SQLiteDbiType {
public:
    /**
     * Id of SQLiteDbi.
     */
    static const char *ID;
};

/**
 * DBI types.
 */
typedef ::testing::Types<BAMDbiType, FileDbiType, SQLiteDbiType> DbiTypes;

/**
 * Base class for DBI tests.
 *
 * Create a new DBI instance of the specified type in SetUp() and deletes it in TearDown().
 *
 * @param DBI_TYPE_PARAM Type of the DBI.
 */
template<class DBI_TYPE_PARAM>
class BaseDbiTest : public ::testing::Test {
public:
    /**
     * Type of the DBI.
     */
    typedef DBI_TYPE_PARAM DBI_TYPE;
protected:
    /**
     * Creates a new DBI instance.
     */
    virtual void SetUp() {
        {
            U2DbiFactory *factory = AppContext::getDbiRegistry()->getDbiFactoryById(DBI_TYPE::ID);
            ASSERT_NE((U2DbiFactory *)NULL, factory);
            dbi.reset(factory->createDbi());
        }
        ASSERT_NE((U2Dbi *)NULL, dbi.get());
    }

    /**
     * Deletes the DBI instance.
     */
    virtual void TearDown() {
        dbi.reset();
    }

protected:
    /**
     * Pointer to the DBI instance.
     */
    std::auto_ptr<U2Dbi> dbi;
};

/**
 * Base class for DBI tests.
 *
 * Creates a new DBI instance in SetUp() and deletes it in TearDown().
 *
 * @param DBI_TYPE_PARAM Type of the DBI.
 */
template<class DBI_TYPE_PARAM>
class UninitializedDbiTest {
};

/**
 * Base class for DBI tests.
 *
 * Creates a new DBI instance in SetUp() and deletes it in TearDown().
 * Specialization for BAMDbi.
 *
 * @param DBI_TYPE_PARAM Type of the DBI.
 */
template<>
class UninitializedDbiTest<BAMDbiType> : public BaseDbiTest<BAMDbiType> {
protected:
    /**
     * Creates and initializes a new DBI instance.
     */
    virtual void SetUp();

    /**
     * Deletes the DBI instance.
     */
    virtual void TearDown();
protected:
    /**
     * Name of the database file copy.
     */
    QString fileName;
    /**
     * Properties for DBI initialization.
     */
    QHash<QString, QString> properties;
};

/**
 * Base class for DBI tests.
 *
 * Creates a new DBI instance in SetUp() and deletes it in TearDown().
 * Specialization for FileDbi.
 *
 * @param DBI_TYPE_PARAM Type of the DBI.
 */
template<>
class UninitializedDbiTest<FileDbiType> : public BaseDbiTest<FileDbiType> {
protected:
    /**
     * Creates and initializes a new DBI instance.
     */
    virtual void SetUp();

    /**
     * Deletes the DBI instance.
     */
    virtual void TearDown();
protected:
    /**
     * Name of the database file copy.
     */
    QString fileName;
    /**
     * Properties for DBI initialization.
     */
    QHash<QString, QString> properties;
};

/**
 * Base class for DBI tests.
 *
 * Creates a new DBI instance in SetUp() and deletes it in TearDown().
 * Specialization for FileDbi.
 *
 * @param DBI_TYPE_PARAM Type of the DBI.
 */
template<>
class UninitializedDbiTest<SQLiteDbiType> : public BaseDbiTest<SQLiteDbiType> {
protected:
    /**
     * Creates and initializes a new DBI instance.
     */
    virtual void SetUp();

    /**
     * Deletes the DBI instance.
     */
    virtual void TearDown();
protected:
    /**
     * Name of the database file copy.
     */
    QString fileName;
    /**
     * Properties for DBI initialization.
     */
    QHash<QString, QString> properties;
};

/**
 * Base class for DBI tests.
 *
 * Creates and initializes a new DBI instance in SetUp() and shutdowns and deletes it in TearDown().
 *
 * @param DBI_TYPE_PARAM Type of the DBI.
 */
template<class DBI_TYPE_PARAM>
class InitializedDbiTest {
};

/**
 * Base class for DBI tests.
 *
 * Creates and initializes a new DBI instance in SetUp() and shutdowns and deletes it in TearDown().
 * Specialization for BAMDbi.
 *
 * @param DBI_TYPE_PARAM Type of the DBI.
 */
template<>
class InitializedDbiTest<BAMDbiType> : public UninitializedDbiTest<BAMDbiType> {
protected:
    /**
     * Creates and initializes a new DBI instance.
     */
    virtual void SetUp();

    /**
     * Shutdowns and deletes the DBI instance.
     */
    virtual void TearDown();
};

/**
 * Base class for DBI tests.
 *
 * Creates and initializes a new DBI instance in SetUp() and shutdowns and deletes it in TearDown().
 * Specialization for FileDbi.
 *
 * @param DBI_TYPE_PARAM Type of the DBI.
 */
template<>
class InitializedDbiTest<FileDbiType> : public UninitializedDbiTest<FileDbiType> {
protected:
    /**
     * Creates and initializes a new DBI instance.
     */
    virtual void SetUp();

    /**
     * Shutdowns and deletes the DBI instance.
     */
    virtual void TearDown();
};

/**
 * Base class for DBI tests.
 *
 * Creates and initializes a new DBI instance in SetUp() and shutdowns and deletes it in TearDown().
 * Specialization for SQliteDbi.
 *
 * @param DBI_TYPE_PARAM Type of the DBI.
 */
template<>
class InitializedDbiTest<SQLiteDbiType> : public UninitializedDbiTest<SQLiteDbiType> {
protected:
    /**
     * Creates and initializes a new DBI instance.
     */
    virtual void SetUp();

    /**
     * Shutdowns and deletes the DBI instance.
     */
    virtual void TearDown();
};

} // namespace U2

#endif // DBITEST_H
