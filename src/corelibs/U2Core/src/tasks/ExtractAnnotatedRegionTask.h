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
