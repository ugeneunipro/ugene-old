#ifndef _U2_SEQUENCE_DBI_TEST_H_
#define _U2_SEQUENCE_DBI_TEST_H_

#include <core/dbi/DbiTest.h>

#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2Sequence.h>


namespace U2 {

class SequenceDbiTest : public BaseDbiTest {
public:
    static APITestData initTestData();
    static bool compareSequences(const U2Sequence& s1, const U2Sequence& s2);
    static void replaceRegion(const QByteArray& originalSequence,
        const QByteArray& dataToInsert,
        const U2Region& region,
        QByteArray& resultSequence);

protected:
    virtual void SetUp();
    
protected:
    U2SequenceDbi* sequenceDbi;
    QList<U2DataId> sequences;
};

class UpdateSequenceArgs {
public:
    int sequenceId;
    QList<U2Region> regionsToReplace;
    QList<QByteArray> datazToInsert;
};

} // namespace

Q_DECLARE_METATYPE(U2::U2Sequence);
Q_DECLARE_METATYPE(U2::UpdateSequenceArgs);

#endif
