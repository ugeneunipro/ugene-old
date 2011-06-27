#include "BaseFormatsToSeqDbiConverter.h"

#include <U2Core/AppContext.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>


namespace U2 {

FillDbTask::FillDbTask(const QList<DNASequence>& sequences_, const QString& dbUrl)
: Task(tr("Fill db with sequences task"), TaskFlag_None), sequences(sequences_) {
    if (QFile::exists(dbUrl)) {
        QFile::remove(dbUrl);
    }
    U2DbiFactory *factory = AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi");
    assert(factory);
    dbi.reset(factory->createDbi());

    QHash<QString, QString> props;
    props[U2_DBI_OPTION_CREATE] = U2_DBI_VALUE_ON;
    props["url"] = dbUrl;

    QVariantMap persistentData;
    U2OpStatusImpl opStatus;
    dbi->init(props, persistentData, opStatus);

    U2OpStatusImpl os;
    dbi->getObjectDbi()->createFolder("/", os);
}

void FillDbTask::run() {
    U2SequenceDbi* sequenceDbi = dbi->getSequenceDbi();
    foreach(DNASequence dna, sequences) {        
        U2OpStatusImpl os;
        U2Sequence seq;
        seq.circular = dna.circular;
        seq.length = dna.length();
        seq.alphabet = dna.alphabet->getId();
        sequenceDbi->createSequenceObject(seq, "/", os);
        assert(!os.hasError());
        sequenceDbi->updateSequenceData(seq.id, U2Region(0, dna.length()), dna.seq, os);
        assert(!os.hasError());
    }

    U2OpStatusImpl os;
    dbi->shutdown(os);
    assert(!os.hasError());
    dbi.reset();
}

ConvertBaseFormatToDbTask::ConvertBaseFormatToDbTask(const QString& srcDoc, const QString& dbUrl_)
: Task(tr("Convert base format to sequence dbi"), TaskFlag_NoRun), dbUrl(dbUrl_) {
    loadTask = LoadDocumentTask::getDefaultLoadDocTask(GUrl(srcDoc));
    addSubTask(loadTask);
}

QList<Task*> ConvertBaseFormatToDbTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> tasks;
    if (subTask == loadTask) {
        Document* doc = loadTask->getDocument();
        QList<GObject*> objs = doc->findGObjectByType(GObjectTypes::SEQUENCE);
        QList<DNASequence> sequences;
        foreach(GObject* o, objs) {
            DNASequenceObject* so = qobject_cast<DNASequenceObject*>(o);
            assert(so);
            sequences << so->getDNASequence();
        }
        tasks << new FillDbTask(sequences, dbUrl);
    }
    return tasks;
}

} //namespace
