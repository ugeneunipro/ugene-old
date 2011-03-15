/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "SmithWatermanReportCallback.h"

namespace U2 {

SmithWatermanReportCallbackImpl::SmithWatermanReportCallbackImpl(
                                AnnotationTableObject* _aobj,
                                const QString& _annotationName,
                                const QString& _annotationGroup,
                                QObject* pOwn): 
    QObject(pOwn), 
    annotationName(_annotationName), annotationGroup(_annotationGroup), 
    aObj(_aobj), autoReport(_aobj != NULL)
{
}

QString SmithWatermanReportCallbackImpl::report(const QList<SmithWatermanResult>& results) {    
    if (autoReport && aObj.isNull()) {
        return tr("Annotation object not found.");
    }

    if (autoReport && aObj->isStateLocked()) {
        return tr("Annotation table is read-only");
    }

    foreach (const SmithWatermanResult& res , results) {
        anns.append(res.toAnnotation(annotationName));
    }

    if (autoReport) {
        QList<Annotation*> annotations;
        foreach(const SharedAnnotationData& ad, anns) {
            annotations.append(new Annotation(ad));
        }
        aObj->addAnnotations(annotations, annotationGroup);
    }
    return QString();
}

} // namespace
