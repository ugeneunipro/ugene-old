#include "QDFindActor.h"

#include <U2Core/Log.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/FailTask.h>

#include <U2Lang/BaseTypes.h>


namespace U2 {

static const QString PATTERN_ATTR("pattern");

QDFindActor::QDFindActor(QDActorPrototype const* proto) : QDActor(proto) {
    units["find"] = new QDSchemeUnit(this);
}

int QDFindActor::getMinResultLen() const {
    return cfg->getParameter(PATTERN_ATTR)->getAttributeValue<QString>().length();
}

int QDFindActor::getMaxResultLen() const {
    return getMinResultLen();
}

QString QDFindActor::getText() const {
    QString hypPtrn = cfg->getParameter(PATTERN_ATTR)->getAttributeValue<QString>().toUpper();

    if (hypPtrn.isEmpty()) {
        hypPtrn = QString("&lt;<a href=%1>pattern</a>&gt;").arg(PATTERN_ATTR);
    }
    else {
        hypPtrn = QString("<a href=%1>%2</a>")
            .arg(PATTERN_ATTR)
            .arg(hypPtrn);
    }

    return hypPtrn;
}

Task* QDFindActor::getAlgorithmTask(const QVector<U2Region>& location) {
    Task* t = new Task(tr("Find"), TaskFlag_NoRun);
    settings.sequence = scheme->getDNA()->getSequence();
    settings.pattern = cfg->getParameter(PATTERN_ATTR)->getAttributeValue<QString>().toAscii().toUpper();

    if (settings.pattern.isEmpty()) {
        QString err = tr("%1: pattern is empty.").arg(getParameters()->getLabel());
        return new FailTask(err);
    }

    DNAAlphabet* ptrnAl = AppContext::getDNAAlphabetRegistry()->findAlphabet(settings.pattern);
    if (ptrnAl->getType()!=DNAAlphabet_NUCL) {
        QString err = tr("%1: pattern has to be nucleic").arg(getParameters()->getLabel());
        return new FailTask(err);
    }

    foreach(const U2Region& r, location) {
        FindAlgorithmTaskSettings s(settings);
        s.searchRegion = r;
        FindAlgorithmTask* findTask = new FindAlgorithmTask(s);
        connect(new TaskSignalMapper(findTask), SIGNAL(si_taskFinished(Task*)), SLOT(sl_onFindTaskFinished(Task*)));
        t->addSubTask(findTask);
    }
    return t;
}

void QDFindActor::sl_onFindTaskFinished(Task* t) {
    FindAlgorithmTask* findTask = qobject_cast<FindAlgorithmTask*>(t);
    QList<FindAlgorithmResult> res = findTask->popResults();
    foreach(const FindAlgorithmResult& r, res) {
        QDResultUnit ru(new QDResultUnitData);
        ru->strand = r.strand;
        ru->region = r.region;
        ru->owner = units.value("find");
        QDResultGroup* g = new QDResultGroup(QDStrand_DirectOnly);
        g->add(ru);
        results.append(g);
    }
}

QDFindActorPrototype::QDFindActorPrototype() {
    descriptor.setId("search");
    descriptor.setDisplayName(QDFindActor::tr("Pattern"));
    descriptor.setDocumentation(QDFindActor::tr("Finds pattern."));

    Descriptor pd(PATTERN_ATTR, QDFindActor::tr("Pattern"), QDFindActor::tr("A subsequence pattern to look for."));
    attributes << new Attribute(pd, BaseTypes::STRING_TYPE(), true);
}

}//namespace
