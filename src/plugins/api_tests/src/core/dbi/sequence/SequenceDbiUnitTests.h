#ifndef _U2_SEQUENCE_DBI_UNITTESTS_H_
#define _U2_SEQUENCE_DBI_UNITTESTS_H_

#include "core/dbi/DbiTest.h"

#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Test/TestRunnerSettings.h>

#include <unittest.h>
#include <memory>

namespace U2 {

class UpdateSequenceArgs {
public:
    int sequenceId;
    QList<U2Region> regionsToReplace;
    QList<QByteArray> datazToInsert;
};

class SequenceTestData {
public:
    static void init();
    static void shutdown();
	static U2SequenceDbi* getSequenceDbi();
	static QList<U2DataId>* getSequences() { return sequences; };
    static bool compareSequences(const U2Sequence& s1, const U2Sequence& s2);
	static void checkUpdateSequence(UnitTest *t, const UpdateSequenceArgs& args);
    static void replaceRegion(const QByteArray& originalSequence,
        const QByteArray& dataToInsert,
        const U2Region& region,
        QByteArray& resultSequence);

public:
    static U2SequenceDbi* sequenceDbi;
    static QList<U2DataId>* sequences;

	static const QString& SEQ_DB_URL;

protected:
    static TestDbiProvider dbiProvider;
};

class SequenceDbiUnitTests_getSequenceObject : public UnitTest {
public:
    void Test();
};

class SequenceDbiUnitTests_getAllSequenceObjects : public UnitTest {
public:
    void Test();
};

class SequenceDbiUnitTests_getSequenceObjectInvalid : public UnitTest {
public:
    void Test();
};

class SequenceDbiUnitTests_createSequenceObject : public UnitTest {
public:
    void Test();
};

class SequenceDbiUnitTests_updateSequenceObject : public UnitTest {
public:
    void Test();
};

class SequenceDbiUnitTests_getSequenceData : public UnitTest {
public:
    void Test();
};

class SequenceDbiUnitTests_updateSequenceData : public UnitTest {
public:
    void Test();
};

class SequenceDbiUnitTests_updateHugeSequenceData : public UnitTest {
public:
    void Test();
};

class SequenceDbiUnitTests_updateSequencesData : public UnitTest {
public:
    void Test();
};

} // namespace U2

Q_DECLARE_METATYPE(U2::U2Sequence);
Q_DECLARE_METATYPE(U2::UpdateSequenceArgs);

#endif //_U2_SEQUENCE_DBI_UNITTESTS_H_