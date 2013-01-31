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

#ifndef __EXTRACT_ANNOTATED_REGION_TASK_H__
#define __EXTRACT_ANNOTATED_REGION_TASK_H__

#include <U2Core/Task.h>
#include <U2Core/DNASequence.h>
#include <U2Core/AnnotationData.h>

#include <U2Core/DNATranslation.h>

namespace U2 {

struct U2CORE_EXPORT ExtractAnnotatedRegionTaskSettings {
    ExtractAnnotatedRegionTaskSettings() : gapSym('-'), gapLength(0), translate(true), complement(true), extLeft(0), extRight(0) {}
    char    gapSym;
    int     gapLength;
    bool    translate;
    bool    complement;
    int     extLeft; 
    int     extRight;
};

class U2CORE_EXPORT ExtractAnnotatedRegionTask : public Task {
    Q_OBJECT
public:
    ExtractAnnotatedRegionTask( const DNASequence & sequence, SharedAnnotationData sd, const ExtractAnnotatedRegionTaskSettings & cfg);
    void prepare();
    void run();
    DNASequence getResultedSequence() const {return resultedSeq;}
    SharedAnnotationData getResultedAnnotation() {return resultedAnn;}
private:
    void prepareTranslations();
    void extractLocations(QList<QByteArray>& resParts, QVector<U2Region>& resLocation, const QVector<U2Region>& origLocation);
    
    DNASequence inputSeq;
    SharedAnnotationData inputAnn;
    ExtractAnnotatedRegionTaskSettings cfg;

    QVector<U2Region> extendedRegions;
    DNATranslation * complT;
    DNATranslation * aminoT;

    SharedAnnotationData resultedAnn;
    DNASequence resultedSeq;
};

}// ns

#endif 
