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

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include "../ProjectFilterNames.h"

#include "FeatureKeyFilterTask.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// FeatureKeyFilterTask
//////////////////////////////////////////////////////////////////////////

FeatureKeyFilterTask::FeatureKeyFilterTask(const ProjectTreeControllerModeSettings &settings, const QList<QPointer<Document> > &docs)
    : AbstractProjectFilterTask(settings, ProjectFilterNames::FEATURE_KEY_FILTER_NAME, docs)
{

}

void FeatureKeyFilterTask::run() {
    foreach (const QPointer<Document> &doc, docs) {
        if (!doc.isNull()) {
            filterDocument(doc.data());
        }
        if (stateInfo.isCoR()) {
            break;
        }
    }
}

void FeatureKeyFilterTask::filterDocument(Document *doc) {
    SAFE_POINT(NULL != doc, L10N::nullPointerError("document"), );
    CHECK(doc->isLoaded(), );

    foreach (GObject *obj, doc->getObjects()) {
        QSet<QString> filterNames = getMatchedFilterNames(obj);
        SafeObjList filteredResult;
        filteredResult.append(obj);
        foreach (const QString &filterName, filterNames) {
            emit si_objectsFiltered(filterName, filteredResult);
        }
        stateInfo.setProgress(++processedObjectCount / totalObjectCount);
        if (stateInfo.isCoR()) {
            break;
        }
    }
}

QSet<QString> FeatureKeyFilterTask::getMatchedFilterNames(GObject *obj) {
    QSet<QString> filterNames;

    AnnotationTableObject *annObject = qobject_cast<AnnotationTableObject *>(obj);
    CHECK(NULL != annObject, filterNames);

    annObject->ref();

    foreach (const Annotation &annotation, annObject->getAnnotations()) {
        const QString annotationName = annotation.getName();
        if (filterNames.contains(annotationName)) {
            continue;
        }
        foreach (const U2Qualifier &qual, annotation.getQualifiers()) {
            if (settings.nameFilterAcceptsString(qual.value)) {
                filterNames.insert(annotationName);
                break;
            }
        }
        if (stateInfo.isCoR()) {
            break;
        }
    }

    annObject->deref();

    return filterNames;
}

//////////////////////////////////////////////////////////////////////////
/// FeatureKeyFilterTaskFactory
//////////////////////////////////////////////////////////////////////////

AbstractProjectFilterTask * FeatureKeyFilterTaskFactory::createNewTask(const ProjectTreeControllerModeSettings &settings,
    const QList<QPointer<Document> > &docs) const
{
    return new FeatureKeyFilterTask(settings, docs);
}

} // namespace U2
