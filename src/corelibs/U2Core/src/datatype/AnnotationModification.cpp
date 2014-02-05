/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "AnnotationModification.h"

namespace U2 {

AnnotationModification::AnnotationModification( AnnotationModificationType _type,
    const Annotation &a )
    : type( _type ), annotation( a )
{

}

QualifierModification::QualifierModification( AnnotationModificationType t, const Annotation &a,
    const U2Qualifier &q )
    : AnnotationModification( t, a ), qualifier( q )
{

}

AnnotationGroupModification::AnnotationGroupModification( AnnotationModificationType t,
    const Annotation &a, const AnnotationGroup &g )
    : AnnotationModification( t, a ), group( g )
{

}

} // namespace U2
