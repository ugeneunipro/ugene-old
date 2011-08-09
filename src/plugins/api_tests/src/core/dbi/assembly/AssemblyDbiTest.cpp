#include "AssemblyDbiTest.h"
#include "AssemblyDbiTestUtil.h"

#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/AppContext.h>

#include <U2Test/TestRunnerSettings.h>

#include <QtCore/QDir>


namespace U2 {

static const QString& INVALID_ASSEMBLY_ID = "invalid_assembly_id";

static const QString& TOTAL_NUM_READS = "total_num_reads";
static const QString& NUM_READS_IN = "num_reads_in";
static const QString& NUM_READS_OUT = "num_reads_out";

static const QString& GET_READS_IN = "get_reads_region";
static const QString& GET_READS_OUT = "get_reads_out";

static const QString& GET_READS_BY_ROW_REGION = "get_reads_by_row_region";
static const QString& GET_READS_BY_ROW_BEGIN = "get_reads_by_row_begin";
static const QString& GET_READS_BY_ROW_END = "get_reads_by_row_end";
static const QString& GET_READS_BY_ROW_OUT = "get_reads_by_row_out";

static const QString& READS_BY_NAME_IN = "reads_by_name_in";

static const QString& MAX_END_POS = "max_end_pos";

static const QString& MAX_PACKED_ROW_IN = "max_packed_row_in";
static const QString& MAX_PACKED_ROW_OUT = "max_packed_row_out";

static const QString& REMOVE_READS_IN = "remove_reads_in";

static const QString& ADD_READ = "add_read";

static const QString& PACK_COUNT = "pack_count";
static const QString& PACK_MAX = "pack_max";

static const QString& COVERAGE_REGION_IN = "calc_coverage_region_in";
static const QString& COVERAGE_REGION_OUT = "calc_coverage_region_out";

APITestData createTestData() {
    APITestData d;
    
    d.addValue<QString>(BaseDbiTest::DB_URL, "assembly-dbi.ugenedb");

    // invalid assembly id
    d.addValue(INVALID_ASSEMBLY_ID, QByteArray("zZÿÿ"));

    // countReads
    d.addValue(TOTAL_NUM_READS, 48);
    d.addValue(NUM_READS_IN, U2Region(10,10));
    d.addValue(NUM_READS_OUT, 6);

    //getReads
    {
        d.addValue(GET_READS_IN, U2Region(140, 145));

        U2AssemblyRead read1(new U2AssemblyReadData());
        read1->name = "2797 Example sequence FW - secondary sequence 5464";
        read1->leftmostPos = 93;
        read1->effectiveLen = 49;
        read1->packedViewRow = 15;
        read1->readSequence = "AAGATCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAG";
        read1->cigar.append(U2CigarToken(U2CigarOp_M, 49));
        read1->flags = None;

        U2AssemblyRead read2(new U2AssemblyReadData());
        read2->name = "4940 Example sequence FW - secondary sequence 5466";
        read2->leftmostPos = 95;
        read2->effectiveLen = 49;
        read2->packedViewRow = 0;
        read2->readSequence = "GATCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAGAT";
        read2->cigar.append(U2CigarToken(U2CigarOp_M, 49));
        read2->flags = None;

        U2AssemblyRead read3(new U2AssemblyReadData());
        read3->name = "2378 Example sequence RV - secondary sequence 5468";
        read3->leftmostPos = 97;
        read3->effectiveLen = 49;
        read3->packedViewRow = 16;
        read3->readSequence = "TCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAGATGC";
        read3->cigar.append(U2CigarToken(U2CigarOp_M, 49));
        read3->flags = None;

        QVariantList reads;
        reads << qVariantFromValue(read1) << qVariantFromValue(read2) << qVariantFromValue(read3);
        d.addValue(GET_READS_OUT, reads);
    }

    // getReadsByRow
    {
        U2AssemblyRead read1(new U2AssemblyReadData());
        read1->name = "4924 Example sequence RV -E(46=3|) secondary sequence 5400";
        read1->leftmostPos = 29;
        read1->effectiveLen = 49;
        read1->packedViewRow = 10;
        read1->readSequence = "TGACGCAGCGAGCCGCCTTCATACTTTGTTAGCACGGCTCCACACATAT";
        read1->flags = None;
        read1->cigar.append(U2CigarToken(U2CigarOp_M, 49));

        U2AssemblyRead read2(new U2AssemblyReadData());
        read2->name = "2728 Example sequence FW - secondary sequence 5440";
        read2->leftmostPos = 69;
        read2->effectiveLen = 49;
        read2->packedViewRow = 5;
        read2->readSequence = "CACACATATGGTCCGACTCTAATGAAGATCCTCATGTTATATCGGCAGT";
        read2->flags = None;
        read2->cigar.append(U2CigarToken(U2CigarOp_M, 49));

        QVariantList reads;
        reads << qVariantFromValue(read1) << qVariantFromValue(read2);

        d.addValue(GET_READS_BY_ROW_REGION, U2Region(74, 3));
        d.addValue(GET_READS_BY_ROW_BEGIN, 5);
        d.addValue(GET_READS_BY_ROW_END, 11);
        d.addValue(GET_READS_BY_ROW_OUT, reads);
    }

    // getReadsByName
    {
        U2AssemblyRead read(new U2AssemblyReadData());
        read->name = "Test read";
        read->leftmostPos = 93;
        read->effectiveLen = 49;
        read->packedViewRow = 0;
        read->readSequence = "AAGATCCTCATGTTATATCGGCAGTGGGTTGATCAATCCACGTGGATAG";
        read->flags = None;
        
        QVariantList readsByName;
        readsByName.append(qVariantFromValue(read));
        d.addValue(READS_BY_NAME_IN, readsByName);
    }

    // getMaxPackedRow
    d.addValue(MAX_PACKED_ROW_IN, U2Region(10,6));
    d.addValue(MAX_PACKED_ROW_OUT, 3);

    // getMaxEndPos
    d.addValue(MAX_END_POS, qint64(146));

    // removeReads
    d.addValue(REMOVE_READS_IN, U2Region(10, 10));    

    // addReads
    {
        U2AssemblyRead read2add(new U2AssemblyReadData());
        read2add->name = "Test read";
        read2add->leftmostPos = 90;
        read2add->readSequence = "GTTATATCGGCAGTGGGTTGATC";
        read2add->effectiveLen = read2add->readSequence.length();
        read2add->flags = None;
        QVariantList reads2add;
        reads2add.append(qVariantFromValue(read2add));
        d.addValue(ADD_READ, reads2add);
    }

    // pack
    d.addValue(PACK_MAX, 28);
    d.addValue(PACK_COUNT, 48);
    
    // calculateCoverage

    d.addValue(COVERAGE_REGION_IN, U2Region(20, 1));
    d.addValue(COVERAGE_REGION_OUT, 7);

    return d;
}

TEST_P(AssemblyDbiTest, getAssemblyObject) {
    const U2DataId& id = this->assemblyIds.first();
    U2OpStatusImpl os;
    U2Assembly assembly = this->assemblyDbi->getAssemblyObject(id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(assembly.id, id);
}

TEST_P(AssemblyDbiTest, getAssemblyObjectInvalid) {
    const U2DataId& invalidId = testData.getValue<QByteArray>(INVALID_ASSEMBLY_ID);
    U2OpStatusImpl os;
    const U2Assembly& assembly = this->assemblyDbi->getAssemblyObject(invalidId, os);
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
    ASSERT_TRUE(assembly.id.isEmpty());
}

TEST_P(AssemblyDbiTest, countReads) {
    const U2DataId& id = this->assemblyIds.first();

    {
        U2OpStatusImpl os;
        qint64 numReads = this->assemblyDbi->countReads(id, U2_REGION_MAX, os);
        ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
        ASSERT_EQ(testData.getValue<qint64>(TOTAL_NUM_READS), numReads);
    }

    U2OpStatusImpl os;
    const U2Region& testRegion = testData.getValue<U2Region>(NUM_READS_IN);
    qint64 numReads = this->assemblyDbi->countReads(id, testRegion, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(testData.getValue<qint64>(NUM_READS_OUT), numReads);
}

TEST_P(AssemblyDbiTest, countReadsInvalid) {
    const U2DataId& id = testData.getValue<U2DataId>(INVALID_ASSEMBLY_ID);
    U2OpStatusImpl os;
    qint64 res = this->assemblyDbi->countReads(id, U2_REGION_MAX, os);
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
    ASSERT_EQ(res, -1);
}

TEST_P(AssemblyDbiTest, getReads) {
    const U2DataId& id = this->assemblyIds.first();
    const U2Region& region = testData.getValue<U2Region>(GET_READS_IN);
    U2OpStatusImpl os;

    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > iter;
    iter.reset(this->assemblyDbi->getReads(id, region, os));
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    QVariantList expectedVar = testData.getValue<QVariantList>(GET_READS_OUT);
    QList<U2AssemblyRead> expectedReads;
    AssemblyDbiTestUtil::var2readList(expectedVar, expectedReads);
    ASSERT_TRUE(AssemblyDbiTestUtil::compareReadLists(iter.get(), expectedReads));
}

TEST_P(AssemblyDbiTest, getReadsInvalid) {
    const U2DataId& id = testData.getValue<U2DataId>(INVALID_ASSEMBLY_ID);
    U2OpStatusImpl os;
    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > iter;
    iter.reset(this->assemblyDbi->getReads(id, U2_REGION_MAX, os));
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
    ASSERT_TRUE(iter.get() == NULL);
}

TEST_P(AssemblyDbiTest, getReadsByRow) {
    const U2DataId& id = this->assemblyIds.first();
    const U2Region& region = testData.getValue<U2Region>(GET_READS_BY_ROW_REGION);
    U2OpStatusImpl os;

    qint64 begin = testData.getValue<qint64>(GET_READS_BY_ROW_BEGIN);
    qint64 end = testData.getValue<qint64>(GET_READS_BY_ROW_END);

    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > iter;
    iter.reset(this->assemblyDbi->getReadsByRow(id, region, begin, end, os));
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    QVariantList expectedVar = testData.getValue<QVariantList>(GET_READS_BY_ROW_OUT);
    QList<U2AssemblyRead> expectedReads;
    AssemblyDbiTestUtil::var2readList(expectedVar, expectedReads);
    ASSERT_TRUE(AssemblyDbiTestUtil::compareReadLists(iter.get(), expectedReads));
}

TEST_P(AssemblyDbiTest, getReadsByRowInvalid) {
    const U2DataId& id = testData.getValue<U2DataId>(INVALID_ASSEMBLY_ID);
    qint64 begin = testData.getValue<qint64>(GET_READS_BY_ROW_BEGIN);
    qint64 end = testData.getValue<qint64>(GET_READS_BY_ROW_END);
    const U2Region& region = testData.getValue<U2Region>(GET_READS_BY_ROW_REGION);    
    U2OpStatusImpl os;

    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > iter;
    iter.reset(this->assemblyDbi->getReadsByRow(id, region, begin, end, os));
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
    ASSERT_TRUE(iter.get() == NULL);
}

TEST_P(AssemblyDbiTest, getReadsByName) {
    QVariantList readsVar = testData.getValue<QVariantList>(READS_BY_NAME_IN);
    ASSERT_FALSE(readsVar.isEmpty()) << "Incorrect test data for 'getReadsByName'. Reads list is empty.";

    QList<U2AssemblyRead> reads;
    AssemblyDbiTestUtil::var2readList(readsVar, reads);
    const QByteArray& name = reads.first()->name;
    const U2DataId& id = this->assemblyIds.first();

    {
        U2OpStatusImpl os;
        BufferedDbiIterator<U2AssemblyRead> it(reads);
        this->assemblyDbi->addReads(id, &it, os);
        ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    }

    U2OpStatusImpl os;
    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > iter;
    iter.reset(this->assemblyDbi->getReadsByName(id, name, os));
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_TRUE(AssemblyDbiTestUtil::compareReadLists(iter.get(), reads)) << "Reads not found";
}

TEST_P(AssemblyDbiTest, getReadsByNameInvalid) {    
    const U2DataId& id = testData.getValue<U2DataId>(INVALID_ASSEMBLY_ID);
    U2OpStatusImpl os;
    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > iter;
    iter.reset(this->assemblyDbi->getReadsByName(id, "", os));
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
    ASSERT_TRUE(iter.get() == NULL);
}

TEST_P(AssemblyDbiTest, getMaxPackedRow) {
    const U2DataId& id = this->assemblyIds.first();
    U2OpStatusImpl os;

    qint64 actual = this->assemblyDbi->getMaxPackedRow(id, testData.getValue<U2Region>(MAX_PACKED_ROW_IN), os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(actual, testData.getValue<qint64>(MAX_PACKED_ROW_OUT));
}

TEST_P(AssemblyDbiTest, getMaxPackedRowInvalid) {
    const U2DataId& id = testData.getValue<U2DataId>(INVALID_ASSEMBLY_ID);
    U2OpStatusImpl os;
    qint64 res = this->assemblyDbi->getMaxPackedRow(id, U2_REGION_MAX, os);
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
    ASSERT_EQ(res, -1);
}

TEST_P(AssemblyDbiTest, getMaxEndPos) {
    U2DataId id = this->assemblyIds.first();
    U2OpStatusImpl os;
    qint64 res = this->assemblyDbi->getMaxEndPos(id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_EQ(testData.getValue<qint64>(MAX_END_POS), res);
}

TEST_P(AssemblyDbiTest, getMaxEndPosInvalid) {
    const U2DataId& id = testData.getValue<U2DataId>(INVALID_ASSEMBLY_ID);
    U2OpStatusImpl os;
    qint64 res = this->assemblyDbi->getMaxEndPos(id, os);
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
    ASSERT_EQ(res, -1);
}

TEST_P(AssemblyDbiTest, createAssemblyObject) {
    U2Assembly assembly;
    U2AssemblyReadsImportInfo importInfo;
    U2OpStatusImpl os;

    this->assemblyDbi->createAssemblyObject(assembly, "/", NULL, importInfo, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    U2Assembly res = this->assemblyDbi->getAssemblyObject(assembly.id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
}

TEST_P(AssemblyDbiTest, removeReads) {
    const U2DataId& id = this->assemblyIds.first();
    const U2Region& region = testData.getValue<U2Region>(REMOVE_READS_IN);
    U2OpStatusImpl os;

    QList<U2DataId> readIds;
    {
        std::auto_ptr< U2DbiIterator<U2AssemblyRead> > iter;
        iter.reset(this->assemblyDbi->getReads(id, region, os));
        ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

        while (iter->hasNext()) {
            const U2AssemblyRead& read = iter->next();
            readIds.append(read->id);
        }
    }

    this->assemblyDbi->removeReads(id, readIds, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > iter;
    iter.reset(this->assemblyDbi->getReads(id, region, os));
    ASSERT_FALSE(iter->hasNext()) << "Reads were not deleted";
}

TEST_P(AssemblyDbiTest, removeReadsInvalid) {
    const U2DataId& id = testData.getValue<U2DataId>(INVALID_ASSEMBLY_ID);
    U2OpStatusImpl os;
    this->assemblyDbi->removeReads(id, QList<U2DataId>(), os);
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
}

TEST_P(AssemblyDbiTest, addReads) {
    QVariantList reads2add = testData.getValue<QVariantList>(ADD_READ);
    QList<U2AssemblyRead> reads;
    foreach(QVariant var, reads2add) {
        const U2AssemblyRead& read = qVariantValue<U2AssemblyRead>(var);
        reads.append(read);
    }

    const U2DataId& id = this->assemblyIds.first();
    BufferedDbiIterator<U2AssemblyRead> it(reads);
    U2OpStatusImpl os;

    this->assemblyDbi->addReads(id, &it, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    foreach(U2AssemblyRead read, reads) {
        std::auto_ptr< U2DbiIterator<U2AssemblyRead> > iter;
        iter.reset(this->assemblyDbi->getReads(id, U2_REGION_MAX, os));
        ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

        bool added = false;
        while (iter->hasNext()) {
            if (AssemblyDbiTestUtil::compareReads(read, iter->next())) {
                added = true;
                break;
            }
        }
        ASSERT_TRUE(added);
    }
}

TEST_P(AssemblyDbiTest, addReadsInvalid) {
    const U2DataId& id = testData.getValue<U2DataId>(INVALID_ASSEMBLY_ID);
    QList<U2AssemblyRead> reads;
    BufferedDbiIterator<U2AssemblyRead> it(reads);
    U2OpStatusImpl os;
    this->assemblyDbi->addReads(id, &it, os);
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
}

TEST_P(AssemblyDbiTest, pack) {
    const U2DataId& id = assemblyIds.first();
    U2AssemblyPackStat stats;
    U2OpStatusImpl os;

    this->assemblyDbi->pack(id, stats, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    
    int maxProw = testData.getValue<int>(PACK_MAX);
    qint64 readsCount = testData.getValue<int>(PACK_COUNT);
    
    ASSERT_EQ(stats.maxProw, maxProw);
    ASSERT_EQ(stats.readsCount, readsCount);
}

TEST_P(AssemblyDbiTest, packInvalid) {
    const U2DataId& id = testData.getValue<U2DataId>(INVALID_ASSEMBLY_ID);
    U2AssemblyPackStat stat;
    U2OpStatusImpl os;
    this->assemblyDbi->pack(id, stat, os);
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
}

TEST_P(AssemblyDbiTest, calculateCoverage) {
    U2OpStatusImpl os;
    const U2DataId& id = this->assemblyIds.first();
    const U2Region& region = testData.getValue<U2Region>(COVERAGE_REGION_IN);
    U2AssemblyCoverageStat c;
    c.coverage.resize(1);
    this->assemblyDbi->calculateCoverage(id, region, c, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    int res = c.coverage.first().maxValue;
    ASSERT_EQ(testData.getValue<int>(COVERAGE_REGION_OUT), res);
}

TEST_P(AssemblyDbiTest, calculateCoverageInvalid) {
    const U2DataId& id = testData.getValue<U2DataId>(INVALID_ASSEMBLY_ID);
    const U2Region& region = testData.getValue<U2Region>(COVERAGE_REGION_IN);
    U2AssemblyCoverageStat c;
    U2OpStatusImpl os;
    this->assemblyDbi->calculateCoverage(id, region, c, os);
    ASSERT_TRUE(os.hasError()) << AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID;
}

INSTANTIATE_TEST_CASE_P(
                        AssemblyDbiTestInstance,
                        AssemblyDbiTest,
                        ::testing::Values(createTestData()));

} // namespace U2
