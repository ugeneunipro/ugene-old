#include "RepeatQuery.h"
#include "FindRepeatsDialog.h"

#include <U2Core/L10n.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/TaskSignalMapper.h>
#include "U2Core/FailTask.h"

#include <U2Lang/QDConstraint.h>
#include <U2Lang/BaseTypes.h>
#include <U2Designer/DelegateEditors.h>

namespace U2 {

static const QString LEN_ATTR("min-length");
static const QString IDENTITY_ATTR("identity");
static const QString MIN_DIST_ATTR("min_dist");
static const QString MAX_DIST_ATTR("max_dist");
static const QString INVERT_ATTR("invert");
static const QString NESTED_ATTR("filter-nested");
static const QString ALGO_ATTR("algorithm");
static const QString THREADS_ATTR("threads");
static const QString MAX_LEN_ATTR("max-length");
    
QDRepeatActor::QDRepeatActor(QDActorPrototype const* proto) : QDActor(proto) {
    simmetric = true;
    cfg->setAnnotationKey("repeat_unit");
    units["left"] = new QDSchemeUnit(this);
    units["right"] = new QDSchemeUnit(this);
    paramConstraints << new QDDistanceConstraint(units.values(), E2S, 0, 5000);
}

static const QString ALGO_SUFFIX = "suffix";
static const QString ALGO_DIAG = "diagonals";
static const QString ALGO_AUTO = "auto";

QList< QPair<QString,QString> > QDRepeatActor::saveConfiguration() const {
    QList< QPair<QString,QString> > res = QDActor::saveConfiguration();
    Attribute* a = cfg->getParameter(ALGO_ATTR);
    for (int i=0; i<res.size(); i++) {
        QPair<QString, QString>& attr = res[i];
        if (attr.first==a->getId()) {
            RFAlgorithm alg = RFAlgorithm(a->getAttributeValue<int>());
            switch (alg) {
            case RFAlgorithm_Auto:
                attr.second = ALGO_AUTO;
                break;
            case RFAlgorithm_Diagonal:
                attr.second = ALGO_DIAG;
                break;
            case RFAlgorithm_Suffix:
                attr.second = ALGO_SUFFIX;
                break;
            default:
                break;
            }
        }
    }
    return res;
}

void QDRepeatActor::loadConfiguration(const QList< QPair<QString,QString> >& strMap) {
    QDActor::loadConfiguration(strMap);
    foreach(const StringAttribute& attr, strMap) {
        if (attr.first==ALGO_ATTR) {
            int alg;
            const QString& strandVal = attr.second;
            if (strandVal==ALGO_AUTO) {
                alg = 0;
            }
            else if (strandVal==ALGO_DIAG) {
                alg = 1;
            }
            else if (strandVal==ALGO_SUFFIX) {
                alg = 2;
            }
            cfg->setParameter(ALGO_ATTR, qVariantFromValue(alg));
        }
    }
}

int QDRepeatActor::getMinResultLen() const {
    return cfg->getParameter(LEN_ATTR)->getAttributeValue<int>();
}

int QDRepeatActor::getMaxResultLen() const {
    return cfg->getParameter(MAX_LEN_ATTR)->getAttributeValue<int>();
}

QString QDRepeatActor::getText() const {
    QString inverted = cfg->getParameter(INVERT_ATTR)->getAttributePureValue().toBool() ?
        QDRepeatActor::tr("inverted") : QDRepeatActor::tr("direct");

    inverted = QString("<a href=%1>%2</a>").arg(INVERT_ATTR).arg(inverted);

    int identity = cfg->getParameter(IDENTITY_ATTR)->getAttributeValue<int>();
    QString identityStr = QString("<a href=%1>%2%</a>").arg(IDENTITY_ATTR).arg(identity);

    int minLen = cfg->getParameter(LEN_ATTR)->getAttributeValue<int>();
    QString minLenStr = QString("<a href=%1>%2</a>").arg(LEN_ATTR).arg(minLen);

    QString doc = QDRepeatActor::tr("Finds <u>%1</u> repeats."
        "<br>Detects <u>%2 identical</u> repeats <u>not shorter than %3 bps</u>.")
        .arg(inverted)
        .arg(identityStr)
        .arg(minLenStr);

    return doc;
}

Task* QDRepeatActor::getAlgorithmTask(const QVector<U2Region>& location) {
    assert(scheme);
    Task* t = NULL;

    settings.algo = RFAlgorithm(cfg->getParameter(ALGO_ATTR)->getAttributeValue<int>());
    settings.minLen = cfg->getParameter(LEN_ATTR)->getAttributeValue<int>();
    int identity = cfg->getParameter(IDENTITY_ATTR)->getAttributeValue<int>();
    settings.setIdentity(identity);
    settings.nThreads = cfg->getParameter(THREADS_ATTR)->getAttributeValue<int>();
    settings.inverted = cfg->getParameter(INVERT_ATTR)->getAttributeValue<bool>();
    settings.filterNested = cfg->getParameter(NESTED_ATTR)->getAttributeValue<bool>();

    QDDistanceConstraint* dc = static_cast<QDDistanceConstraint*>(paramConstraints.first());
    settings.minDist = dc->getMin();
    settings.maxDist = dc->getMax();

    assert(identity <= 100 && identity >= 0);
    assert(settings.minDist>=0);
    DNASequenceObject* dna = scheme->getDNA();
    if (!dna->getAlphabet()->isNucleic()) {
        return new FailTask(tr("Sequence should be nucleic"));
    }

    t = new Task(tr("RepeatQDTask"), TaskFlag_NoRun);
    foreach(const U2Region& r, location) {
        FindRepeatsTaskSettings stngs(settings);
        stngs.seqRegion = r;
        FindRepeatsToAnnotationsTask* st = new FindRepeatsToAnnotationsTask(stngs, dna->getDNASequence(), 
                                                            "repeat unit", QString(), GObjectReference());
        t->addSubTask(st);
        repTasks.append(st);
    }
    connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_onAlgorithmTaskFinished()));
    return t;
}

void QDRepeatActor::sl_onAlgorithmTaskFinished() {
    QList<SharedAnnotationData> res;
    foreach(FindRepeatsToAnnotationsTask* frt, repTasks) {
        res = frt->importAnnotations();
    }
    repTasks.clear();
    foreach(const SharedAnnotationData& ad, res) {
        if (ad->location->regions[0].length > getMaxResultLen()) {
            continue;
        }

        //////////////////////////////////////////////////////////////////////////
        QDResultUnit ru1(new QDResultUnitData);
        QDResultUnit ru2(new QDResultUnitData);
        ru1->quals = ad->qualifiers;
        ru2->quals = ad->qualifiers;
        ru1->region = ad->location->regions[0];
        ru2->region = ad->location->regions[1];
        ru1->owner = units.value("left");
        ru2->owner = units.value("right");
        ru1->strand = U2Strand::Direct;
        ru2->strand = U2Strand::Direct;
        QDResultGroup* g = new QDResultGroup(QDStrand_Both);
        g->add(ru1); g->add(ru2);
        results.append(g);
        //////////////////////////////////////////////////////////////////////////
        
        /*if (scheme->getStrand()==QDStrand_DirectOnly) {
            addResults(ad, false);
        } else if (scheme->getStrand()==QDStrand_ComplementOnly) {
            addResults(ad, true);
        } else {
            addResults(ad, true);
            addResults(ad, false);
        }*/
    }
}

//void QDRepeatActor::addResults( const SharedAnnotationData& ad, bool complement ) {
//    QDResultUnit ru1(new QDResultUnitData);
//    QDResultUnit ru2(new QDResultUnitData);
//    ru1->quals = ad->qualifiers;
//    ru2->quals = ad->qualifiers;
//    ru1->region = ad->location->regions[0];
//    ru2->region = ad->location->regions[1];
//
//    QDResultGroup* g = NULL;
//    if (complement) {
//        ru1->owner = units.value("right");
//        ru2->owner = units.value("left");
//        ru1->strand = U2Strand::Complementary;
//        ru2->strand = U2Strand::Complementary;
//        g = new QDResultGroup(QDStrand_ComplementOnly);
//    } else {
//        ru1->owner = units.value("left");
//        ru2->owner = units.value("right");
//        ru1->strand = U2Strand::Direct;
//        ru2->strand = U2Strand::Direct;
//        g = new QDResultGroup(QDStrand_DirectOnly);
//    }
//    g->add(ru1); g->add(ru2);
//    results.append(g);
//}

QDRepeatActorPrototype::QDRepeatActorPrototype() {
    descriptor.setId("repeats");
    descriptor.setDisplayName(QDRepeatActor::tr("Repeats"));
    descriptor.setDocumentation(QDRepeatActor::tr("Finds repeats in supplied sequence, stores found regions as annotations."));

    Descriptor idd(IDENTITY_ATTR, QDRepeatActor::tr("Identity"), QDRepeatActor::tr("Repeats identity."));
    Descriptor ld(LEN_ATTR, QDRepeatActor::tr("Min length"), QDRepeatActor::tr("Minimum length of repeats."));
    Descriptor ind(INVERT_ATTR, QDRepeatActor::tr("Inverted"), QDRepeatActor::tr("Search for inverted repeats."));
    Descriptor nsd(NESTED_ATTR, QDRepeatActor::tr("Filter nested"), QDRepeatActor::tr("Filter nested repeats."));
    Descriptor ald(ALGO_ATTR, QDRepeatActor::tr("Algorithm"), QDRepeatActor::tr("Control over variations of algorithm."));
    Descriptor thd(THREADS_ATTR, QDRepeatActor::tr("Parallel threads"), QDRepeatActor::tr("Number of parallel threads used for the task."));
    Descriptor mld(MAX_LEN_ATTR, QDRepeatActor::tr("Max length"), QDRepeatActor::tr("Maximum length of repeats."));

    FindRepeatsTaskSettings stngs = FindRepeatsDialog::defaultSettings();

    attributes << new Attribute(ld, BaseTypes::NUM_TYPE(), true, stngs.minLen);
    attributes << new Attribute(idd, BaseTypes::NUM_TYPE(), false, stngs.getIdentity());
    attributes << new Attribute(ind, BaseTypes::BOOL_TYPE(), false, stngs.inverted);
    attributes << new Attribute(nsd, BaseTypes::BOOL_TYPE(), false, stngs.filterNested);
    attributes << new Attribute(ald, BaseTypes::NUM_TYPE(), false, stngs.algo);
    attributes << new Attribute(thd, BaseTypes::NUM_TYPE(), false, stngs.nThreads);
    attributes << new Attribute(mld, BaseTypes::NUM_TYPE(), true, QDActor::DEFAULT_MAX_RESULT_LENGTH);

    QMap<QString, PropertyDelegate*> delegates;    
    {
        QVariantMap m; m["minimum"] = 2; m["maximum"] = INT_MAX; m["suffix"] = L10N::suffixBp();
        delegates[LEN_ATTR] = new SpinBoxDelegate(m);
        delegates[MAX_LEN_ATTR] = new SpinBoxDelegate(m);
    }
    {
        QVariantMap m; m["minimum"] = 50; m["maximum"] = 100; m["suffix"] = "%";
        delegates[IDENTITY_ATTR] = new SpinBoxDelegate(m);
    }
    {
        QVariantMap m; m["specialValueText"] = "Auto";
        delegates[THREADS_ATTR] = new SpinBoxDelegate(m);
    }
    {
        QVariantMap m; 
        m["Auto"] = RFAlgorithm_Auto;
        m["Diagonals"] = RFAlgorithm_Diagonal;
        m["Suffix index"] = RFAlgorithm_Suffix;
        delegates[ALGO_ATTR] = new ComboBoxDelegate(m);
    }

    editor = new DelegateEditor(delegates);
}

}//namespace
