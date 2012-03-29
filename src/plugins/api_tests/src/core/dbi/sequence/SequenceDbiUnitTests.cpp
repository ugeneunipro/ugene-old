#include "SequenceDbiUnitTests.h"
#include <U2Core/AppContext.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QDir>

namespace U2 {

const QString& INVALID_SEQUENCE_ID = "invalid_seq_id";
const QString& GET_SEQUENCE_IN = "get_seq_in";
const QString& GET_SEQUENCE_OUT = "get_seq_out";
const QString& GET_SEQUENCE_DATA_ID = "get_seq_data_id";
const QString& GET_SEQUENCE_DATA_REGION = "get_seq_data_region";
const QString& GET_SEQUENCE_DATA_OUT = "get_seq_data_out";

const QString& SequenceTestData::SEQ_DB_URL("sequence-dbi.ugenedb");
QList<U2DataId>* SequenceTestData::sequences = NULL;
U2SequenceDbi* SequenceTestData::sequenceDbi = NULL;
TestDbiProvider SequenceTestData::dbiProvider = TestDbiProvider();

void SequenceTestData::init() {    

    TestRunnerSettings* trs = AppContext::getAppSettings()->getTestRunnerSettings();
    QString originalFile = trs->getVar("COMMON_DATA_DIR") + "/" + SequenceTestData::SEQ_DB_URL;

    QString tmpFile = QDir::temp().absoluteFilePath(QFileInfo(originalFile).fileName());

    if(QFile::exists(tmpFile)) {
        QFile::remove(tmpFile);
    }

    bool create = false;
    if (QFile::exists(originalFile)) {
        SAFE_POINT(QFile::copy(originalFile, tmpFile), "attribute db file not copied", );
    }else{
        create = true;
    }
    dbiProvider.init(tmpFile, create, false);
    U2Dbi* dbi = dbiProvider.getDbi();
    SAFE_POINT(NULL != dbi, "Dbi not loaded", );
    U2ObjectDbi* objDbi = dbi->getObjectDbi();
    SAFE_POINT(NULL != objDbi, "Dbi object not loaded", );
    U2OpStatusImpl opStatus;

    sequenceDbi = dbi->getSequenceDbi();
    SAFE_POINT(NULL != sequenceDbi, "sequence database not loaded", );

    sequences = new QList<U2DataId>(objDbi->getObjects(U2Type::Sequence, 0, U2_DBI_NO_LIMIT, opStatus));
    SAFE_POINT_OP(opStatus, );
}

U2SequenceDbi* SequenceTestData::getSequenceDbi() {
	if (sequenceDbi == NULL) {
		SequenceTestData::init();
	}
	return sequenceDbi;
}

void SequenceTestData::shutdown() {
	if (sequenceDbi != NULL) {
		U2OpStatusImpl opStatus;
		dbiProvider.close();
        sequenceDbi = NULL;
		SAFE_POINT_OP(opStatus, );
	}
}

bool SequenceTestData::compareSequences(const U2Sequence& s1, const U2Sequence& s2) {
    if (s1.id == s2.id && s1.alphabet.id == s2.alphabet.id &&
		s1.circular == s2.circular && s1.length == s2.length) {
        return true;
    }
    return false;
}

void SequenceTestData::checkUpdateSequence(UnitTest *t, const UpdateSequenceArgs& args) {
	U2SequenceDbi* sequenceDbi = SequenceTestData::getSequenceDbi(); 
	const U2DataId& id = sequences->at(args.sequenceId);

	for (int j=0, max = args.regionsToReplace.size(); j<max; j++) {
		U2Region regionToReplace = args.regionsToReplace.at(j);
		QByteArray dataToInsert = args.datazToInsert.at(j);

		qint64 originalLen(0);
		QByteArray originalSequence;

		{
			U2OpStatusImpl os;
			const U2Sequence& so = sequenceDbi->getSequenceObject(id, os);
			SAFE_POINT_OP(os, );
			originalLen = so.length;
		}

		{
			U2OpStatusImpl os;
			originalSequence = sequenceDbi->getSequenceData(id, U2Region(0, originalLen), os);
			SAFE_POINT_OP(os, );
		}

		{
			U2OpStatusImpl os;
			sequenceDbi->updateSequenceData(id,
			regionToReplace,
			dataToInsert,
			os);
			SAFE_POINT_OP(os,);
		}

		qint64 updatedLen(0);
		{
			U2OpStatusImpl os;
			const U2Sequence& so = sequenceDbi->getSequenceObject(id, os);
			SAFE_POINT_OP(os, );

			updatedLen = so.length;
			qint64 expectedLen = 0;
			const U2Region& intersection = U2Region(0, originalLen).intersect(regionToReplace);
			expectedLen = originalLen - intersection.length + dataToInsert.length();
			CHECK_EXT(expectedLen == updatedLen, t->SetError(QString("incorrect updated sequence length")), );
		}

		{
			U2OpStatusImpl os;
			const QByteArray& actualSeq = sequenceDbi->getSequenceData(id, U2Region(0, updatedLen), os);
			SAFE_POINT_OP(os, );
			QByteArray expectedSeq;
			replaceRegion(originalSequence, dataToInsert, regionToReplace, expectedSeq);
			CHECK_EXT(expectedSeq == actualSeq, t->SetError(QString("incorrect updated sequence")), );
		}
	}
}

void SequenceTestData::replaceRegion(const QByteArray& originalSequence,
                   const QByteArray& dataToInsert,
                   const U2Region& region,
                   QByteArray& resultSequence){
    U2Region regionToReplace = U2Region(0, originalSequence.length()).intersect(region);
    if (regionToReplace.length == 0) {
        if (region.startPos == originalSequence.length()) {
            resultSequence += originalSequence + dataToInsert;
        } else {
            SAFE_POINT(region.startPos == 0, "region start position should be 0", );
        }
    } else {
        QByteArray left = originalSequence.mid(0, regionToReplace.startPos);
        QByteArray right = originalSequence.mid(regionToReplace.endPos());
        resultSequence += left + dataToInsert + right;
    }
}



void SequenceDbiUnitTests_getSequenceObject::Test() {
	U2SequenceDbi* sequenceDbi = SequenceTestData::getSequenceDbi(); 

	APITestData testData;
	testData.addValue(GET_SEQUENCE_IN, 1);

    U2Sequence seq;
    seq.alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    seq.circular = false;
    seq.length = 30;
	testData.addValue(GET_SEQUENCE_OUT, seq);

    int i = testData.getValue<int>(GET_SEQUENCE_IN);
    U2DataId id = SequenceTestData::getSequences()->at(i);

	U2Sequence expected = testData.getValue<U2Sequence>(GET_SEQUENCE_OUT);
    expected.id = id;

    U2OpStatusImpl os;
    const U2Sequence& actual = sequenceDbi->getSequenceObject(id, os);
	SAFE_POINT_OP(os, );
    SAFE_POINT(SequenceTestData::compareSequences(actual, expected), "incorrect expected sequence", );
}

void SequenceDbiUnitTests_getSequenceObjectInvalid::Test(){

	U2SequenceDbi* sequenceDbi = SequenceTestData::getSequenceDbi();
	APITestData testData;
    testData.addValue<QByteArray>(INVALID_SEQUENCE_ID, "anmr%");

    const U2DataId& invalidId = testData.getValue<U2DataId>(INVALID_SEQUENCE_ID);
    SAFE_POINT(!SequenceTestData::getSequences()->contains(invalidId), "invalid id should not be in suquences", );
    U2OpStatusImpl os;
    const U2Sequence& seq = sequenceDbi->getSequenceObject(invalidId, os);
	SAFE_POINT(os.hasError(), "error should be thrown", );
    SAFE_POINT(seq.id.isEmpty(), "sequence id shuld be empty", );
}

void SequenceDbiUnitTests_getAllSequenceObjects::Test() {
	U2SequenceDbi* sequenceDbi = SequenceTestData::getSequenceDbi(); 
	
	for (int i =0; i < SequenceTestData::getSequences()->size(); i++) {
        U2OpStatusImpl os;
		const U2DataId& id = SequenceTestData::getSequences()->at(i);
        const U2Sequence& seq = sequenceDbi->getSequenceObject(id, os);
	    SAFE_POINT_OP(os, );
        SAFE_POINT(id == seq.id, "incorrect expected sequence id", );
    }
}

void SequenceDbiUnitTests_createSequenceObject::Test() {
	U2SequenceDbi* sequenceDbi = SequenceTestData::getSequenceDbi(); 
    
	U2Sequence seq;
    seq.circular = true;
    seq.alphabet = BaseDNAAlphabetIds::AMINO_DEFAULT();

    U2OpStatusImpl os;
    sequenceDbi->createSequenceObject(seq, "/", os);
    SAFE_POINT_OP(os, );

	const U2Sequence& actual = sequenceDbi->getSequenceObject(seq.id, os);
    SAFE_POINT_OP(os, );
    SAFE_POINT(!seq.id.isEmpty(), "sequence id shuld not be empty", );
    SAFE_POINT(seq.alphabet.id == actual.alphabet.id, "incorrect expected squence alphabet", );
    SAFE_POINT(seq.circular == actual.circular, "incorrect expected squence circular", );
}


void SequenceDbiUnitTests_getSequenceData::Test(){

	U2SequenceDbi* sequenceDbi = SequenceTestData::getSequenceDbi(); 
	APITestData testData;

	testData.addValue(GET_SEQUENCE_DATA_ID, 5);
	testData.addValue(GET_SEQUENCE_DATA_REGION, U2Region(5, 20));
	testData.addValue<QByteArray>(GET_SEQUENCE_DATA_OUT, "AAGTGATCGTCCTACGATCG");

    int i = testData.getValue<int>(GET_SEQUENCE_DATA_ID);
    const U2DataId& id = SequenceTestData::getSequences()->at(i);
    const U2Region& region = testData.getValue<U2Region>(GET_SEQUENCE_DATA_REGION);
    U2OpStatusImpl os;

    const QByteArray& expected = testData.getValue<QByteArray>(GET_SEQUENCE_DATA_OUT);
    const QByteArray& actual = sequenceDbi->getSequenceData(id, region, os);
    SAFE_POINT_OP(os, );

    SAFE_POINT(expected == actual, "incorrect expected sequence data", );
}

void SequenceDbiUnitTests_updateSequenceData::Test() {
    UpdateSequenceArgs usd;
    usd.sequenceId = 0;
    usd.datazToInsert << "AAAAAAAAAAAAA";
    usd.regionsToReplace << U2Region(20,5);

    usd.datazToInsert << "AAAAAAAAAAAAA";
    usd.regionsToReplace << U2Region(0,40);

    usd.datazToInsert << "AAAAAAAAAAAAA";
    usd.regionsToReplace << U2Region(13,13);

	SequenceTestData::checkUpdateSequence(this, usd);
}

void SequenceDbiUnitTests_updateHugeSequenceData::Test() {
	UpdateSequenceArgs usd;
	usd.sequenceId = 1;
	qint64 long_max = Q_INT64_C(9223372036854775807);
	usd.regionsToReplace << U2Region(0, long_max);	
    usd.datazToInsert << QByteArray(long_max, 'A');
	SequenceTestData::checkUpdateSequence(this, usd);
};

void SequenceDbiUnitTests_updateSequencesData::Test() {
	UpdateSequenceArgs usd;
	usd.sequenceId = 1;
	qint64 length = 100000000000;
	for (int i = 0; i < 10000; i++){
		usd.regionsToReplace << U2Region((length -1) * i, length);
		usd.datazToInsert << QByteArray(length, 'A');
	}
	SequenceTestData::checkUpdateSequence(this, usd);
};

} //namespace