#ifndef _U2_CLONING_UTIL_TASKS_H_
#define _U2_CLONING_UTIL_TASKS_H_

#include "EnzymeModel.h"
#include "DNAFragment.h"

#include <limits>

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>
#include <U2Core/GUrl.h>

#include <U2Core/AnnotationTableObject.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMutex>

namespace U2 {


class DNASequenceObject;

class DigestSequenceTask : public Task {
    Q_OBJECT
public:
    DigestSequenceTask(const DNASequenceObject*  dnaObj, AnnotationTableObject* sourceTable, 
        AnnotationTableObject* destTable, const QList<SEnzymeData>& cutSites);
    DigestSequenceTask(const DNASequenceObject*  dnaObj, AnnotationTableObject* destTable, 
        const QList<SEnzymeData>& cutSites);
    virtual void prepare();
    virtual void run();
    virtual QString generateReport() const;
    virtual ReportResult report();

private:
   
    void findCutSites(); 
    void saveResults();
    AnnotationData* createFragment( int pos1, const QString& enzymeId1, const QByteArray& leftOverhang,
                                    int pos2, const QString& enzymeId2, const QByteArray& rightOverhang );
    bool searchForRestrictionSites;
    bool isCircular;
    U2Region seqRange;
    AnnotationTableObject *sourceObj, *destObj;
    const DNASequenceObject*  dnaObj;
    QList<SEnzymeData> enzymeData;
    QMap<int, SEnzymeData> cutSiteMap;
    QList<SharedAnnotationData> results;

};

struct LigateFragmentsTaskConfig {
    LigateFragmentsTaskConfig() : annotateFragments(false), checkOverhangs(true), makeCircular(false), addDocToProject(true), openView(false), saveDoc(false) {}
    GUrl docUrl;
    QString seqName;
    bool annotateFragments;
    bool checkOverhangs;
    bool makeCircular;
    bool addDocToProject;
    bool saveDoc;
    bool openView;
   
};

class DNAAlphabet;

class LigateFragmentsTask : public Task {
public:
    LigateFragmentsTask(const QList<DNAFragment>& fragments, const LigateFragmentsTaskConfig& cfg );
    Document* getResultDocument() {return resultDoc; }
    void prepare(); 
    void cleanup();

private:
    static QList<Annotation*> cloneAnnotationsInRegion(const U2Region& region, AnnotationTableObject* source, int globalOffset);
    static QList<Annotation*> cloneAnnotationsInFragmentRegion(const DNAFragment& fragment, AnnotationTableObject* source, int globalOffset);
    static Annotation* createSourceAnnotation(int regLen);
    static Annotation* createFragmentAnnotation(const DNAFragment&, int startPos);
    void createDocument(const QByteArray& seq, const QList<Annotation*> annotations);
    void processOverhangs(const DNAFragment& leftFragment, const DNAFragment& rightFragment, QByteArray& overhangAddition);

    QList<DNAFragment> fragmentList;
    QList<Annotation*> annotations;
    LigateFragmentsTaskConfig cfg;
    Document* resultDoc;
    DNAAlphabet* resultAlphabet;
    QMap<Annotation*,int> offsetMap;
    

};


} //namespace

#endif // _U2_CLONING_UTIL_TASKS_H_
