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

#include <U2Core/Annotation.h>
#include <U2Core/AnnotationGroup.h>

namespace U2 {

enum AnnotationModificationType {
    AnnotationModification_NameChanged,
    AnnotationModification_QualifierAdded,
    AnnotationModification_QualifierRemoved,
    AnnotationModification_LocationChanged,
    AnnotationModification_AddedToGroup, // Emitted when annotation is added to a group. 
                                         // Note that first time annotation added to the object it automatically 
                                         // has a group assigned and this signal is not emitted
    AnnotationModification_RemovedFromGroup
};

class U2CORE_EXPORT AnnotationModification {
public:
                                    AnnotationModification( AnnotationModificationType type,
                                        const __Annotation &a );

    AnnotationModificationType      type;
    __Annotation                    annotation;
};

class  U2CORE_EXPORT QualifierModification : public AnnotationModification {
public:
                                    QualifierModification( AnnotationModificationType type,
                                        const __Annotation &a, const U2Qualifier &q );

    U2Qualifier                     qualifier;
};

class  U2CORE_EXPORT AnnotationGroupModification : public AnnotationModification {
public:
                                    AnnotationGroupModification( AnnotationModificationType type,
                                        const __Annotation &a, const __AnnotationGroup &g );

    __AnnotationGroup               group;
};

} // namespace U2
