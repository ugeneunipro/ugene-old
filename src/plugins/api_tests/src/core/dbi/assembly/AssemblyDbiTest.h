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

// copy of U2AssemblyReadData that can be used with QVariant

class ReadData {
public:
    ReadData() : leftmostPos(0), effectiveLen(0), 
        packedViewRow(0), mappingQuality(255) {}

    bool operator ==(const U2AssemblyRead& read) const {
        if (name != read->name) {
            return false;
        }
        if (leftmostPos != read->leftmostPos) {
            return false;
        }
        if (effectiveLen != read->effectiveLen) {
            return false;
        }
        if (packedViewRow != read->packedViewRow) {
            return false;
        }
        if (readSequence != read->readSequence) {
            return false;
        }
        if (quality != read->quality) {
            return false;
        }
        if (mappingQuality != read->mappingQuality) {
            return false;
        }
        if (flags != read->flags) {
            return false;
        }
        return true;
    }

    QByteArray          name;
    qint64              leftmostPos;
    qint64              effectiveLen;
    qint64              packedViewRow;
    QByteArray          readSequence;
    QByteArray          quality;
    quint8              mappingQuality;
    qint64              flags;
};

} // namespace U2

Q_DECLARE_METATYPE(U2::ReadData);

#endif // ASSEMBLYDBITEST_H
