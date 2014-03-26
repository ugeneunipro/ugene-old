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

#ifndef _U1_ANNOTATION_UTILS_H_
#define _U1_ANNOTATION_UTILS_H_

#include <U2Core/AnnotationData.h>
#include <U2Core/Annotation.h>

namespace U2 {

class DNAAlphabet;
class AnnotationTableObject;
class GObject;
class GObjectReference;
class U2SequenceObject;

class U2CORE_EXPORT AnnotatedRegion {
public:
    AnnotatedRegion();
    AnnotatedRegion(const Annotation &annotation, int regionIdx);
    AnnotatedRegion(const AnnotatedRegion &annRegion);
public:
    Annotation  annotation;
    int         regionIdx;
};

/**
 * U2Annotation and related structures utility functions
 */
class U2CORE_EXPORT U1AnnotationUtils {
public:

    enum AnnotationStrategyForResize {
        AnnotationStrategyForResize_Resize, 
        AnnotationStrategyForResize_Remove,
        AnnotationStrategyForResize_Split_To_Joined,
        AnnotationStrategyForResize_Split_To_Separate
    };

    /**
     * Corrects annotation locations for a sequence. The passed list is original locations
     * The returned list contains set of regions. Each set is per 1 annotation.
     * If specified strategy is 'remove', removes all locations which intersect the modified region or fall inside it.
     */
    static QList<QVector<U2Region> > fixLocationsForReplacedRegion( const U2Region &region2Remove,
        qint64 region2InsertLength, const QVector<U2Region> &originalLoc, AnnotationStrategyForResize s );
    /**
     * Returns translation frame[0,1,2] the region is placed on
     */
    static int getRegionFrame( int sequenceLen, const U2Strand &strand, bool order, int region,
        const QVector<U2Region> &location );
    /**
     * Returns true if annotation location is splitted by sequence "edges".
     * For example, location JOIN(N..SeqSize - 1, 0..M) is splitted.
     */
    static bool isSplitted( const U2Location &location, const U2Region &seqRange );
    /**
     * Return a list of lower/upper case annotations for @data sequence
     * If an annotation is placed from some symbol till the end of the sequence
     * then @isUnfinishedRegion == true and @unfinishedRegion keep this unfinished region
     */
    static QList<AnnotationData> getCaseAnnotations( const char *data, int dataLen, int globalOffset,
        bool &isUnfinishedRegion, U2Region &unfinishedRegion, bool isLowerCaseSearching );

    static QList<AnnotationData> finalizeUnfinishedRegion( bool isUnfinishedRegion,
        U2Region &unfinishedRegion, bool isLowerCaseSearching );
    /**
     * If @annotationsObject is NULL then it creates a new annotation object
     */
    static void addAnnotations( QList<GObject *> &objects, const QList<AnnotationData> &annList,
        const GObjectReference &sequenceRef, AnnotationTableObject *annotationsObject );

    static QList<U2Region> getRelatedLowerCaseRegions( const U2SequenceObject *so,
        const QList<GObject *> &anns );

    static char * applyLowerCaseRegions( char *seq, qint64 first, qint64 len, qint64 globalOffset,
        const QList<U2Region> &regs );

    static QString guessAminoTranslation( AnnotationTableObject *ao, const DNAAlphabet *al );

    static QList<AnnotatedRegion>  getAnnotatedRegionsByStartPos(QList<AnnotationTableObject*> annotationObjects,
                                                                 qint64 startPos );

    static QString lowerCaseAnnotationName;
    static QString upperCaseAnnotationName;
};

} // namespace U2

#endif
