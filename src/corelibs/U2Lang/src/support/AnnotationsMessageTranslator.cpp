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

#include <QtCore/QVariant>

#include <U2Lang/WorkflowContext.h>

#include "AnnotationsMessageTranslator.h"

const char *ANNOTATIONS_COUNT_LABEL = "Count of annotations: ";
const char *ANNOTATION_NAME_LABEL = " Name: ";
const char *REGION_LIST_LABEL = " Regions: ";
const QString NEW_LINE_SYMBOL = "\n";

namespace U2 {

AnnotationsMessageTranslator::AnnotationsMessageTranslator(const QVariant &atomicMessage, WorkflowContext *initContext)
    : BaseMessageTranslator(atomicMessage, initContext)
{
    Q_ASSERT(source.canConvert<QList<SharedAnnotationData> >());
    annotations = source.value<QList<SharedAnnotationData> >();
}

AnnotationsMessageTranslator::~AnnotationsMessageTranslator() {

}

QString AnnotationsMessageTranslator::getTranslation() const {
    QString result = QString().append(QObject::tr(ANNOTATIONS_COUNT_LABEL)
        + QString::number(annotations.size()) + INFO_TAGS_SEPARATOR + NEW_LINE_SYMBOL);
    quint32 annotationsCounter = 1;
    foreach(SharedAnnotationData annotation, annotations) {
        result.append(" " + QString::number(annotationsCounter) + ". "
            + QObject::tr(ANNOTATION_NAME_LABEL) + "'" + annotation->name + "'"
            + INFO_FEATURES_SEPARATOR);
        QVector<U2Region> annotatedRegions = annotation->getRegions();
        if(!annotatedRegions.isEmpty()) {
            result.append(QObject::tr(REGION_LIST_LABEL));
            foreach(U2Region region, annotatedRegions) {
                result.append(region.toString() + INFO_FEATURES_SEPARATOR);
            }
            result = result.left(result.size() - INFO_FEATURES_SEPARATOR.size());
        }
        result.append(INFO_TAGS_SEPARATOR + NEW_LINE_SYMBOL);
        ++annotationsCounter;
    }
    result = result.left(result.size() - INFO_TAGS_SEPARATOR.size() - NEW_LINE_SYMBOL.size());
    return result;
}

} // namespace U2
