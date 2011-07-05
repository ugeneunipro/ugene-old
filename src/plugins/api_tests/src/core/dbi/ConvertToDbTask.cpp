#include "ConvertToDbTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentObject.h>


namespace U2 {

ConvertToSQLiteTask::ConvertToSQLiteTask(const QString& file, const QString& dbUrl)
: Task(tr("Convert %1 to SQLite task").arg(file), TaskFlag_NoRun) {
    if (QFile::exists(dbUrl)) {
        QFile::remove(dbUrl);
    }

    U2DbiFactory *factory = AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi");
    dbi.reset(factory->createDbi());

    QHash<QString, QString> props;
    props[U2_DBI_OPTION_CREATE] = U2_DBI_VALUE_ON;
    props[U2_DBI_OPTION_URL] = dbUrl;

    QVariantMap persistentData;
    U2OpStatusImpl opStatus;

    dbi->init(props, persistentData, opStatus);

    U2OpStatusImpl os;
    dbi->getObjectDbi()->createFolder("/", os);

    loadTask = LoadDocumentTask::getDefaultLoadDocTask(GUrl(file));
    if (!loadTask) {
        setError(tr("Unable to load %1").arg(file));
        return;
    }
    addSubTask(loadTask);
}

ConvertToSQLiteTask::~ConvertToSQLiteTask() {
    U2OpStatusImpl os;
    dbi->shutdown(os);
    assert(!os.hasError());
}

QList<Task*> ConvertToSQLiteTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> tasks;
    if (hasError() || isCanceled()) {
        return tasks;
    }
    if (subTask == loadTask) {
        Document* doc = loadTask->getDocument();
        const QList<GObject*>& objs = doc->getObjects();
        tasks << new FillDbTask(dbi.get(), objs);
    }
    return tasks;
}

FillDbTask::FillDbTask(U2Dbi* dbi, const QList<GObject*>& objects)
: Task(tr("Save objects to db task"), TaskFlag_None), dbi(dbi), objects(objects) {}

void FillDbTask::addSequence(U2Sequence& seq, const QByteArray& data) {
    U2SequenceDbi* sequenceDbi = dbi->getSequenceDbi();

    {
	    U2OpStatusImpl os;
	    sequenceDbi->createSequenceObject(seq, "/", os);
	    assert(!os.hasError());
    }

    {
        U2OpStatusImpl os;
	    sequenceDbi->updateSequenceData(seq.id, U2Region(0, data.length()), data, os);
	    assert(!os.hasError());
    }
}

void FillDbTask::run() {
    foreach(GObject* obj, objects) {
        if (obj->getGObjectType() == GObjectTypes::SEQUENCE) {
            DNASequenceObject* dna = qobject_cast<DNASequenceObject*>(obj);
            U2Sequence sequence;
            sequence.circular = dna->isCircular();
            sequence.alphabet = dna->getAlphabet()->getId();
            addSequence(sequence, dna->getSequence());
        } else if (obj->getGObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT) {
            U2MsaDbi* msaDbi = dbi->getMsaDbi();

            MAlignmentObject* maObj = qobject_cast<MAlignmentObject*>(obj);

            U2Msa msa;
            {
	            msa.alphabet = maObj->getAlphabet()->getId();
	            U2OpStatusImpl os;
	            msaDbi->createMsaObject(msa, "/", os);
	            assert(!os.hasError());
            }

            U2Sequence seq;
            seq.circular = false;
            seq.alphabet = msa.alphabet;

            QList<U2MsaRow> rowsToAdd;
            const QList<MAlignmentRow>& rows = maObj->getMAlignment().getRows();
            foreach(const MAlignmentRow& r, rows) {
                const QByteArray& rdata = r.getCore();
                U2MsaRow row;
                row.sequenceId = seq.id;
                row.gaps << U2MsaGap(0, r.getCoreStart());
                qint64 totalOffset = 0;
                QByteArray data;
                for (int i=0, last = rdata.size(); i < last; i++) {
                    if (rdata[i] == MAlignment_GapChar) {
                        int offset = i - totalOffset;
                        while (i < last && rdata[i] == MAlignment_GapChar) {
                            ++i;
                        }
                        int gap = i - offset - totalOffset;
                        row.gaps << U2MsaGap(offset, gap);
                        totalOffset += gap;
                    }
                    data.append(rdata[i]);
                }
                int offset = rdata.length() + totalOffset + r.getCoreStart();
                int gap = maObj->getLength() - offset;
                assert(offset>=0);
                if (offset != 0) {
                    row.gaps << U2MsaGap(offset, gap);
                }
                rowsToAdd.append(row);
                addSequence(seq, data);
            }
            U2OpStatusImpl os;
            msaDbi->addSequences(msa, rowsToAdd, os);
            assert(!os.hasError());
        }
    }
}

} //namespace
