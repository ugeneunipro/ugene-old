#include "QDTests.h"
#include "QDSceneIOTasks.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DocumentModel.h>

#include <U2Designer/QDScheduler.h>


namespace U2 {

#define SCHEMA "schema"
#define SEQUENCE_NAME "seq"
#define EXPECTED_RESULT "expected_result"

static const QString GROUP_NAME = "Query results";

void GTest_QDSchedulerTest::init(XMLTestFormat *tf, const QDomElement& el) {
    sched = NULL;
    expectedResult = NULL;
    seqObj = NULL;
    result = new AnnotationTableObject("QDResult");
    schema = new QDScheme;

    seqName = el.attribute(SEQUENCE_NAME);
    if (seqName.isEmpty()) {
        failMissingValue(SEQUENCE_NAME);
        return;
    }

    expectedResName = el.attribute(EXPECTED_RESULT);
    if (expectedResName.isEmpty()) {
        failMissingValue(EXPECTED_RESULT);
        return;
    }

    schemaUri = el.attribute(SCHEMA);
    if (schemaUri.isEmpty()) {
        failMissingValue(SCHEMA);
        return;
    }

    schemaUri = env->getVar("COMMON_DATA_DIR") + "/" + schemaUri;
}

void GTest_QDSchedulerTest::prepare() {
    if (hasErrors() || isCanceled()) {
        return;
    }
    Document* seqDoc = getContext<Document>(this, seqName);
    if (seqDoc == NULL){
        stateInfo.setError("can't find sequence");
        return;
    }
    seqObj = qobject_cast<DNASequenceObject*>(seqDoc->findGObjectByType(GObjectTypes::SEQUENCE).first());
    if (seqObj == NULL){
        stateInfo.setError("can't find sequence");
        return;
    }

    Document* expDoc = getContext<Document>(this, expectedResName);
    if (expDoc == NULL){
        stateInfo.setError("can't find result");
        return;
    }
    expectedResult = qobject_cast<AnnotationTableObject*>(expDoc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE).first());
    if (expectedResult == NULL){
        stateInfo.setError("can't find result");
        return;
    }

    QDDocument* doc = new QDDocument;
    QFile f(schemaUri);
    if (!f.open(QIODevice::ReadOnly)) {
        stateInfo.setError("can't read result");
        return;
    }
    QByteArray data = f.readAll();
    const QString& content = QString::fromUtf8(data);
    f.close();
    bool res = doc->setContent(content);
    if (!res) {
        stateInfo.setError( tr("Invalid content: %1").arg(expectedResName) );
        return;
    }

    bool ok = QDSceneSerializer::doc2scheme( (QList<QDDocument*>() << doc), schema );
    delete doc;
    if (!ok) {
        stateInfo.setError(tr("can't read %1").arg(expectedResName));
        return;
    }

    QDRunSettings s;
    schema->setDNA(seqObj);
    s.region = seqObj->getSequenceRange();
    s.scheme = schema;
    s.annotationsObj = result;
    s.groupName = GROUP_NAME;
    sched = new QDScheduler(s);
    addSubTask(sched);
}

QList<Task*> GTest_QDSchedulerTest::onSubTaskFinished(Task* subTask) {
    QList<Task*> subs;
    if (subTask==sched) {
        const QList<AnnotationGroup*>& res = result->getRootGroup()->getSubgroup(GROUP_NAME, false)->getSubgroups();
        const QList<AnnotationGroup*>& expRes = expectedResult->getRootGroup()->getSubgroup(GROUP_NAME, false)->getSubgroups();;
        subs.append(new CompareATObjectsTask(res, expRes));
    } else {
        CompareATObjectsTask* compareTask = qobject_cast<CompareATObjectsTask*>(subTask);
        assert(compareTask);
        if (!compareTask->areEqual()) {
            setError(tr("Results do not match."));
        }
    }
    return subs;
}

GTest_QDSchedulerTest::~GTest_QDSchedulerTest() {
    delete schema;
    delete result;
}

QList<XMLTestFactory*> QDTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_QDSchedulerTest::createFactory());
    return res;
}

static bool containsRegion(AnnotationGroup* g, const U2Region& subj) {
    foreach(Annotation* a, g->getAnnotations()) {
        foreach(const U2Region& r, a->getRegions()) {
            if (r==subj) {
                return true;
            }
        }
    }
    return false;
}

static bool compareGroups(AnnotationGroup* g1, AnnotationGroup* g2) {
    foreach(Annotation* a1, g1->getAnnotations()) {
        foreach(const U2Region& r1, a1->getRegions()) {
            if (!containsRegion(g2, r1)) {
                return false;
            }
        }
    }
    return true;
}

static bool containsGroup(const QList<AnnotationGroup*>& grps, AnnotationGroup* subj) {
    foreach(AnnotationGroup* grp, grps) {
        if (compareGroups(grp, subj)) {
            return true;
        }
    }
    return false;
}

void CompareATObjectsTask::run() {
    foreach(AnnotationGroup* g1, grps1) {
        if(!containsGroup(grps2, g1)) {
            equal = false;
            return;
        }
    }

    equal = true;
}

}//namespace
