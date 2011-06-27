#include "SequenceDbiTest.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/DNAAlphabet.h>

#include <QtCore/QDir>


namespace U2 {

const QString& INVALID_SEQ_ID = "invalid_seq_id";

const QString& GET_SEQ_IN = "get_seq_in";
const QString& GET_SEQ_OUT = "get_seq_out";

const QString& GET_SEQ_DATA_ID = "get_seq_data_id";
const QString& GET_SEQ_DATA_REGION = "get_seq_data_region";
const QString& GET_SEQ_DATA_OUT = "get_seq_data_out";

const QString& CREATE_SEQ = "create_seq";

const QString& UPDATE_SEQ = "update_seq";

APITestData SequenceDbiTest::initTestData() {
    APITestData d;
    // COMMON_DATA_DIR
    {
        QString dataDirPath = AppContext::getAppSettings()->getTestRunnerSettings()->getVar("SequenceDbiTest");
        QDir dataDir(dataDirPath);
        QString dbUrl = dataDir.absoluteFilePath("test-sequenceDbi1.ugenedb");
        d.addValue(BaseDbiTest::DB_URL, dbUrl);
    }

    d.addValue<QByteArray>(INVALID_SEQ_ID, "anmr%");

    // getSequenceObject
    {
        d.addValue(GET_SEQ_IN, 1);

        U2Sequence seq;
        seq.alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
        seq.circular = false;
        seq.length = 30;
        d.addValue(GET_SEQ_OUT, seq);
    }

    // getSequenceData
    {
        d.addValue(GET_SEQ_DATA_ID, 5);
        d.addValue(GET_SEQ_DATA_REGION, U2Region(5, 20));
        d.addValue<QByteArray>(GET_SEQ_DATA_OUT, "AAGTGATCGTCCTACGATCG");
    }

    // createSequenceObject
    {
        U2Sequence seq;
        seq.circular = true;
        seq.alphabet = BaseDNAAlphabetIds::AMINO_DEFAULT();
        d.addValue(CREATE_SEQ, seq);
    }

    // updateSequenceData
    {
        UpdateSequenceArgs usd;
        usd.sequenceId = 0;
        usd.datazToInsert << "AAAAAAAAAAAAA";
        usd.regionsToReplace << U2Region(20,5);

        usd.datazToInsert << "AAAAAAAAAAAAA";
        usd.regionsToReplace << U2Region(0,40);

        usd.datazToInsert << "AAAAAAAAAAAAA";
        usd.regionsToReplace << U2Region(13,13);

        d.addValue(UPDATE_SEQ, usd);
    }

    return d;
}

bool SequenceDbiTest::compareSequences(const U2Sequence& s1, const U2Sequence& s2) {
    if (s1.id != s2.id) {
        return false;
    }
    if (s1.alphabet.id != s2.alphabet.id) {
        return false;
    }
    if (s1.circular != s2.circular) {
        return false;
    }
    if (s1.length != s2.length) {
        return false;
    }
    return true;
}

void SequenceDbiTest::replaceRegion(const QByteArray& originalSequence,
                   const QByteArray& dataToInsert,
                   const U2Region& region,
                   QByteArray& resultSequence)
{
    U2Region regionToReplace = U2Region(0, originalSequence.length()).intersect(region);
    if (regionToReplace.length == 0) {
        if (region.startPos == originalSequence.length()) {
            resultSequence += originalSequence + dataToInsert;
        } else {
            ASSERT_EQ(region.startPos, 0);
        }
    } else {
        QByteArray left = originalSequence.mid(0, regionToReplace.startPos);
        QByteArray right = originalSequence.mid(regionToReplace.endPos());
        resultSequence += left + dataToInsert + right;
    }
}

void SequenceDbiTest::SetUp() {
    ASSERT_NO_FATAL_FAILURE(BaseDbiTest::SetUp());

    sequenceDbi = this->dbi->getSequenceDbi();
    ASSERT_NE((U2SequenceDbi*)NULL, this->sequenceDbi);

    U2ObjectDbi* objDbi = this->dbi->getObjectDbi();
    ASSERT_NE((U2ObjectDbi*)NULL, objDbi);

    U2OpStatusImpl os;
    sequences = objDbi->getObjects(U2Type::Sequence, 0, U2_DBI_NO_LIMIT, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
}

TEST_P(SequenceDbiTest, getSequenceObject) {
    int i = testData.getValue<int>(GET_SEQ_IN);
    U2DataId id = this->sequences.at(i);
    U2OpStatusImpl os;
    U2Sequence expected = testData.getValue<U2Sequence>(GET_SEQ_OUT);
    expected.id = id;
    const U2Sequence& actual = this->sequenceDbi->getSequenceObject(id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_TRUE(compareSequences(actual, expected));
}

TEST_P(SequenceDbiTest, getSequenceObjectInvalid) {
    const U2DataId& invalidId = testData.getValue<U2DataId>(INVALID_SEQ_ID);
    ASSERT_FALSE(sequences.contains(invalidId));
    U2OpStatusImpl os;
    const U2Sequence& seq = this->sequenceDbi->getSequenceObject(invalidId, os);
    ASSERT_TRUE(os.hasError());
    ASSERT_TRUE(seq.id.isEmpty());
}

TEST_P(SequenceDbiTest, getAllSequenceObjects) {
    foreach(const U2DataId& id, this->sequences) {
        U2OpStatusImpl os;
        const U2Sequence& seq = this->sequenceDbi->getSequenceObject(id, os);
        ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
        ASSERT_EQ(id, seq.id);
    }
}

TEST_P(SequenceDbiTest, getSequenceData) {
    int i = testData.getValue<int>(GET_SEQ_DATA_ID);
    const U2DataId& id = this->sequences.at(i);
    const U2Region& region = testData.getValue<U2Region>(GET_SEQ_DATA_REGION);
    U2OpStatusImpl os;

    const QByteArray& expected = testData.getValue<QByteArray>(GET_SEQ_DATA_OUT);
    const QByteArray& actual = this->sequenceDbi->getSequenceData(id, region, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    ASSERT_EQ(expected, actual);
}

TEST_P(SequenceDbiTest, getSequenceDataInvalid) {
    const U2DataId& invalidId = testData.getValue<U2DataId>(INVALID_SEQ_ID);
    U2OpStatusImpl os;
    const QByteArray& res = this->sequenceDbi->getSequenceData(invalidId, U2Region(0, 10), os);
    ASSERT_TRUE(res.isEmpty());
}

TEST_P(SequenceDbiTest, createSequenceObject) {
    U2Sequence seq = testData.getValue<U2Sequence>(CREATE_SEQ);
    {
	    U2OpStatusImpl os;
	    this->sequenceDbi->createSequenceObject(seq, "/", os);
	    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    }
    U2OpStatusImpl os;
    const U2Sequence& actual = this->sequenceDbi->getSequenceObject(seq.id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_FALSE(seq.id.isEmpty());
    ASSERT_EQ(seq.alphabet.id, actual.alphabet.id);
    ASSERT_EQ(seq.circular, actual.circular);
}

TEST_P(SequenceDbiTest, updateSequenceData) {
    const UpdateSequenceArgs& args = testData.getValue<UpdateSequenceArgs>(UPDATE_SEQ);
    const U2DataId& id = this->sequences.at(args.sequenceId);

    ASSERT_EQ(args.regionsToReplace.size(), args.datazToInsert.size());
    for (int j=0, max = args.regionsToReplace.size(); j<max; j++) {
        U2Region regionToReplace = args.regionsToReplace.at(j);
        QByteArray dataToInsert = args.datazToInsert.at(j);

        qint64 originalLen(0);
        QByteArray originalSequence;

        {
            U2OpStatusImpl os;
            const U2Sequence& so = this->sequenceDbi->getSequenceObject(id, os);
            ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
            originalLen = so.length;

            {
                U2OpStatusImpl os;
                originalSequence = this->sequenceDbi->getSequenceData(id, U2Region(0, originalLen), os);
                ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
            }
        }

        {
            U2OpStatusImpl os;
            this->sequenceDbi->updateSequenceData(id,
                regionToReplace,
                dataToInsert,
                os);
            ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
        }

        qint64 updatedLen(0);

        {
            U2OpStatusImpl os;
            const U2Sequence& so = this->sequenceDbi->getSequenceObject(id, os);
            ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

            updatedLen = so.length;
            qint64 expectedLen = 0;
            const U2Region& intersection = U2Region(0, originalLen).intersect(regionToReplace);
            expectedLen = originalLen - intersection.length + dataToInsert.length();
            ASSERT_EQ(expectedLen, updatedLen);
        }

        {
            U2OpStatusImpl os;
            const QByteArray& actualSeq = this->sequenceDbi->getSequenceData(id, U2Region(0, updatedLen), os);
            ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

            QByteArray expectedSeq;
            replaceRegion(originalSequence, dataToInsert, regionToReplace, expectedSeq);
            ASSERT_EQ(expectedSeq, actualSeq);
        }
    }
}

INSTANTIATE_TEST_CASE_P(
                        SequenceDbiTestInstance,
                        SequenceDbiTest,
                        ::testing::Values(SequenceDbiTest::initTestData()));

} //namespace
