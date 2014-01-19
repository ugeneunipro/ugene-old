/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <QtCore/QFile>

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Algorithm/SArrayIndex.h>
#include <U2Core/TextUtils.h>

#include "CustomPatternAnnotationTask.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
// CustomPatternAnnotationTask

CustomPatternAnnotationTask::CustomPatternAnnotationTask(AnnotationTableObject* aObj, const U2::U2EntityRef &entityRef, const SharedFeatureStore &store)
    : Task(tr("Custom pattern annotation"), TaskFlags_NR_FOSCOE), dnaObj("ref", entityRef), aTableObj(aObj), featureStore(store)
{
    GCOUNTER( cvar, tvar, "CustomPatternAnnotationTask" );
}

void CustomPatternAnnotationTask::prepare()
{
    sequence = dnaObj.getWholeSequenceData();
    
    if (dnaObj.isCircular()) {
        sequence += sequence;
    }

    const QList<FeaturePattern>& patterns = featureStore->getFeatures();

    if (patterns.length() == 0) {
        return;
    }
    char unknownChar = 'N';
    index = QSharedPointer<SArrayIndex>( new SArrayIndex(sequence.constData(), sequence.length(), 
        featureStore->getMinFeatureSize(), stateInfo, unknownChar) );
    
    if (hasError()) {
        return;
    }
    
    DNATranslation* complTT = AppContext::getDNATranslationRegistry()->lookupComplementTranslation( dnaObj.getAlphabet() );
    assert(complTT);

    foreach (const FeaturePattern& pattern, patterns) {
        
        if (pattern.sequence.length() > sequence.length()) {
            continue;
        }
        
        SArrayBasedSearchSettings settings;
        settings.unknownChar = unknownChar;
        settings.query = pattern.sequence;

        SArrayBasedFindTask* task = new SArrayBasedFindTask(index.data(), settings);
        taskFeatureNames.insert(task, PatternInfo(pattern.name, true) );
        addSubTask(task);
        
        complTT->translate( settings.query.data( ), settings.query.size() );
        TextUtils::reverse( settings.query.data( ), settings.query.size( ) );

        SArrayBasedFindTask* revComplTask = new SArrayBasedFindTask(index.data(), settings);
        taskFeatureNames.insert(revComplTask, PatternInfo(pattern.name, false) );
        addSubTask(revComplTask);
    }
}


QList<Task*> CustomPatternAnnotationTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> subTasks;
    
    if (!taskFeatureNames.contains(subTask)) {
        return subTasks;
    }

    SArrayBasedFindTask* task = static_cast<SArrayBasedFindTask*> (subTask);
    const QList<int>& results = task->getResults();
    PatternInfo info = taskFeatureNames.take(task);
    
    qint64 seqLen = dnaObj.getSequenceLength();

    foreach (int pos, results) {
    
        
        if (pos > dnaObj.getSequenceLength() ) {
            continue;
        } 

        int endPos = pos + task->getQuery().length() - 1;

        AnnotationData data;
        data.name = info.name;
        U2Strand strand = info.forwardStrand ? U2Strand::Direct : U2Strand::Complementary;
        data.setStrand(strand);

        if (dnaObj.isCircular() && endPos > seqLen) {
            
            int outerLen = endPos - seqLen;
            int innerLen = task->getQuery().length() - outerLen;
            U2Region region1(pos - 1, innerLen);
            U2Region region2(0, outerLen);
            data.location->regions << region1 << region2;
            data.setLocationOperator(U2LocationOperator_Join);
        } else {
            U2Region region(pos - 1, task->getQuery().length() );
            data.location->regions << region;
        }

        annotations.append(data);

    }

    if (taskFeatureNames.isEmpty() && annotations.size() > 0) {
        subTasks.append( new CreateAnnotationsTask( aTableObj, featureStore->getName(), annotations) );
    }

    return subTasks;

}

//////////////////////////////////////////////////////////////////////////
// FeatureStore

bool FeatureStore::load()
{
    QFile inputFile(path);

    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        return false;
    }
    
    int minPatternSize = INT_MAX;

    while (!inputFile.atEnd()) {
        QByteArray line = inputFile.readLine().trimmed();
        QList<QByteArray> lineItems = line.split('\t');
        
        if (line.startsWith("#")) {
            continue;
        }

        assert( lineItems.size() == 3 );
        if (lineItems.size() != 3) {
            break;
        }

        FeaturePattern pattern;

        pattern.name = lineItems[0].trimmed();
        pattern.sequence = lineItems[2].toUpper();
        if (pattern.sequence.length() < minPatternSize) {
            minPatternSize = pattern.sequence.length();
        }

        features.append(pattern);
    }

    if (minPatternSize != INT_MAX) {
        minFeatureSize = minPatternSize;
    }

    return !features.isEmpty( );
}

//////////////////////////////////////////////////////////////////////////
// CustomPatternAutoAnnotationUpdater

CustomPatternAutoAnnotationUpdater::CustomPatternAutoAnnotationUpdater(const SharedFeatureStore& store )
    : AutoAnnotationsUpdater(tr("Plasmid features"), store->getName(), true ), featureStore(store)
{
    assert(featureStore);
}


Task* CustomPatternAutoAnnotationUpdater::createAutoAnnotationsUpdateTask( const AutoAnnotationObject* aa )
{
    AnnotationTableObject *aObj = aa->getAnnotationObject();
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

} // namespace U2
