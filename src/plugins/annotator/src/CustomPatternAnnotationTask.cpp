#include <QtCore/QFile>

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2View/FindPatternTask.h>

#include "CustomPatternAnnotationTask.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
// CustomPatternAnnotationTask

CustomPatternAnnotationTask::CustomPatternAnnotationTask(AnnotationTableObject* aObj, const U2::U2EntityRef &entityRef, const SharedFeatureStore &store)
    : Task(tr("Custom pattern annotation"), TaskFlags_NR_FOSCOE), aTableObj(aObj), seqRef(entityRef),  featureStore(store)
{
    GCOUNTER( cvar, tvar, "CustomPatternAnnotationTask" );
}


void CustomPatternAnnotationTask::prepare()
{
    // what if circular sequence?
    U2SequenceObject dnaObj("ref", seqRef);
    QByteArray sequence = dnaObj.getWholeSequenceData();

    const QList<FeaturePattern>& patterns = featureStore->getFeatures();


    foreach (const FeaturePattern& pattern, patterns) {
        // check alphabet?
        FindAlgorithmTaskSettings settings;

        settings.sequence = sequence;
        settings.pattern = pattern.second;
        settings.patternSettings = FindAlgorithmPatternSettings_Exact;
        settings.searchRegion = U2Region(0,dnaObj.getSequenceLength());

        // strand
        DNATranslation* compTT = AppContext::getDNATranslationRegistry()->lookupComplementTranslation( dnaObj.getAlphabet() );
        if (compTT) {
            settings.strand = FindAlgorithmStrand_Both;
            settings.complementTT = compTT;
        } else {
            settings.strand = FindAlgorithmStrand_Direct;
        }

        //TODO: circular?

        QString annotName = pattern.first;
        QString annotGroup = featureStore->getName();

        // Creating and registering the task
        FindPatternTask* task = new FindPatternTask(settings,  aTableObj,   annotName, annotGroup, false);

        addSubTask(task);

    }


}

//////////////////////////////////////////////////////////////////////////
// FeatureStore

bool FeatureStore::load()
{
    QFile inputFile(path);

    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        return false;
    }

    while (!inputFile.atEnd()) {
        QByteArray line = inputFile.readLine().trimmed();
        QList<QByteArray> lineItems = line.split('\t');

        if (lineItems.size() != 3) {
            break;
        }

        FeaturePattern pattern;

        pattern.first = lineItems[0];
        pattern.second = lineItems[2].toUpper();

        features.append(pattern);

    }

    return features.size() > 0;
}

//////////////////////////////////////////////////////////////////////////
// CustomPatternAutoAnnotationUpdater

CustomPatternAutoAnnotationUpdater::CustomPatternAutoAnnotationUpdater(const SharedFeatureStore& store )
    : AutoAnnotationsUpdater(tr("Custom annotations"), store->getName() ), featureStore(store)
{
    assert(featureStore);
}


Task* CustomPatternAutoAnnotationUpdater::createAutoAnnotationsUpdateTask( const AutoAnnotationObject* aa )
{
    AnnotationTableObject* aObj = aa->getAnnotationObject();
    const U2EntityRef& dnaRef = aa->getSeqObject()->getEntityRef();
    Task* task = new CustomPatternAnnotationTask(aObj, dnaRef, featureStore );

    return task;
}

bool CustomPatternAutoAnnotationUpdater::checkConstraints( const AutoAnnotationConstraints& constraints )
{
   if (constraints.alphabet == NULL) {
        return false;
   }

    return constraints.alphabet->isNucleic();
}


}
