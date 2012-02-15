/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CREATE_ANNOTATION_TASK_H_
#define _U2_CREATE_ANNOTATION_TASK_H_

#include <U2Core/Task.h>

#include <U2Core/AnnotationTableObject.h>

namespace U2 {

class AnnotationTableObject;
class AnnotationData;

class U2CORE_EXPORT CreateAnnotationsTask : public Task {
    Q_OBJECT
public:
    // Adds annotations to the object. Waits object to be unlocked if needed
    // Works only in a context of active project
    CreateAnnotationsTask(AnnotationTableObject* o, const QString& group, SharedAnnotationData data);
    CreateAnnotationsTask(AnnotationTableObject* o, const QString& group, QList<SharedAnnotationData> data);
    CreateAnnotationsTask(const GObjectReference& ref, const QString& group, QList<SharedAnnotationData> data);

    ReportResult report();
    const QList<Annotation*>& getAnnotations() const {return annotations;}

private:
    GObjectReference            aRef;
    QPointer<AnnotationTableObject>  aobj;
    QString                     groupName;
    QList<SharedAnnotationData> aData;
    QList<Annotation*>          annotations;
    int                         pos;
};

}//namespace

#endif
