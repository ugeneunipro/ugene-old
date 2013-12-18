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

#ifndef _U2_ANNOTATION_SELECTION_H_
#define _U2_ANNOTATION_SELECTION_H_

#include <U2Core/Annotation.h>
#include <U2Core/AnnotationGroup.h>
#include <U2Core/U2Region.h>

#include "SelectionTypes.h"

namespace U2 {

class FeaturesTableObject;
class DNATranslation;
class U2EntityRef;
class U2OpStatus;

class U2CORE_EXPORT AnnotationSelectionData {
public:
                        AnnotationSelectionData( const __Annotation &a, int lIdx );

    bool                operator ==( const AnnotationSelectionData &d ) const;
    int                 getSelectedRegionsLen( ) const;
    /**
     * Returns all regions from selected annotations locations without any modification
     */
    QVector<U2Region>   getSelectedRegions( ) const;

    /**
     * The annotation selected
     */
    __Annotation        annotation;
    /**
     * Location idx that is preferable for visualization
     * -1 == not specified, 0..N == idx of U2Region in 'location' field
     * BUG:404: do we need a separate class for this feature?
     */
    int                 locationIdx;
};

class  U2CORE_EXPORT AnnotationSelection : public GSelection {
    Q_OBJECT
public:
                                            AnnotationSelection( QObject *p = NULL );

    const QList<AnnotationSelectionData> &  getSelection( ) const;

    const AnnotationSelectionData *         getAnnotationData( const __Annotation &a ) const;
    /**
     * Adds annotation to selection.
     * If annotation is already in selection and have a different locationIdx
     * -> removes the old annotaiton selection data
     * and adds the annotation again with updated locationIdx
     */
    void                                    addToSelection( const __Annotation &a,
                                                int locationIdx = -1 );

    void                                    removeFromSelection( const __Annotation &a,
                                                int locationIdx = -1 );

    bool                                    isEmpty( ) const;

    void                                    clear( );

    void                                    removeObjectAnnotations( FeaturesTableObject *obj );

    bool                                    contains( const __Annotation &a ) const;

    bool                                    contains( const __Annotation &a, int locationIdx ) const;

    static void                             getAnnotationSequence( QByteArray &res,
                                                const AnnotationSelectionData &ad, char gapSym,
                                                const U2EntityRef &ref, DNATranslation *complTT,
                                                DNATranslation *aminoTT, U2OpStatus &os );
    /**
     * Returns list of locations of all selected annotations that belongs to the objects in list
     */
    QVector<U2Region>                       getSelectedLocations(
                                                const QSet<FeaturesTableObject *> &objects ) const;

signals:
    void si_selectionChanged(
        AnnotationSelection *thiz,
        const QList<__Annotation> &added,
        const QList<__Annotation> &removed);

private:
    QList<AnnotationSelectionData> selection;
};

//////////////////////////////////////////////////////////////////////////
// AnnotationGroupSelection

class  U2CORE_EXPORT AnnotationGroupSelection : public GSelection {
    Q_OBJECT
public:
                                        AnnotationGroupSelection( QObject *p = NULL );

    const QList<__AnnotationGroup> &    getSelection( ) const;

    void                                addToSelection( const __AnnotationGroup &g );

    void                                removeFromSelection( const __AnnotationGroup &g );

    bool                                isEmpty( ) const;

    void                                clear( );

    void                                removeObjectGroups( FeaturesTableObject *obj );

    bool                                contains( const __AnnotationGroup &g ) const;

signals:
    void si_selectionChanged(
        AnnotationGroupSelection *thiz,
        const QList<__AnnotationGroup> &added,
        const QList<__AnnotationGroup> &removed );

private:
    QList<__AnnotationGroup> selection;
};


}//namespace

#endif

