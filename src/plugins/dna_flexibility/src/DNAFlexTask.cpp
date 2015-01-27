/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "DNAFlexTask.h"
#include "FindHighFlexRegions.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

DNAFlexTask::DNAFlexTask(const HighFlexSettings& _settings,
        AnnotationTableObject *_annotObject,
        const QString& _annotName,
        const QString& _annotGroup,
        const DNASequence& _sequence)
    : Task(tr("DNA Flexibility task"), TaskFlags_NR_FOSCOE),
      settings(_settings),
      annotObject(_annotObject),
      annotName(_annotName),
      annotGroup(_annotGroup),
      sequence(_sequence)
{
    addSubTask(findHighFlexTask = new FindHighFlexRegions(_sequence, settings));
}

QList<Task*> DNAFlexTask::onSubTaskFinished(Task* subTask)
{
    QList<Task*> resultsList;

    if (subTask->hasError() && subTask == findHighFlexTask)
    {
        stateInfo.setError(subTask->getError());
    }

    if (hasError() || isCanceled()) {
        return resultsList;
    }

    if (annotObject.isNull()) {
        stateInfo.setError(tr("The annotations object is not available!"));
        return resultsList;
    }

    if (subTask == findHighFlexTask)
    {
        FindHighFlexRegions* findTask = qobject_cast<FindHighFlexRegions*>(findHighFlexTask);
        SAFE_POINT(findTask, "Failed to cast FindHighFlexRegions task!", QList<Task*>());

        QList<HighFlexResult> results = findTask->getResults();
        QList<AnnotationData> annots = getAnnotationsFromResults(results);

        if(!annots.isEmpty()) {
            resultsList.append(new CreateAnnotationsTask(annotObject, annots, annotGroup));
        }
    }
    return resultsList;
}


QList<AnnotationData> DNAFlexTask::getAnnotationsFromResults(const QList<HighFlexResult>& results)
{
    QList<AnnotationData> annotResults;

    foreach(const HighFlexResult& result, results) {
        AnnotationData annotData;
        annotData.name = annotName;
        annotData.location->regions << result.region;

        annotData.qualifiers.append(
            U2Qualifier("area_average_threshold", QString::number(result.averageThreshold, 'f', 3)));
        annotData.qualifiers.append(
            U2Qualifier("windows_number", QString::number(result.windowsNumber)));
        annotData.qualifiers.append(
            U2Qualifier("total_threshold", QString::number(result.totalThreshold, 'f', 3)));

        annotResults.append(annotData);
    }
    return annotResults;
}


} // namespace
