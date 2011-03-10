#ifndef _U2_ORF_MARKER_TESTS_H_
#define _U2_ORF_MARKER_TESTS_H_

#include <U2Test/XMLTestUtils.h>
#include <U2Core/GObject.h>
#include <QtXml/QDomElement>
#include <U2Core/U2Region.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2Algorithm/ORFAlgorithmTask.h>

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AppContext.h>

namespace U2 {

class GTest_ORFMarkerTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_ORFMarkerTask, "plugin_orf-marker-search");

    void prepare();
    Task::ReportResult report();
private:
    QString seqName;
    QString translationId;
    ORFAlgorithmSettings settings;
    QVector<U2Region> expectedResults;
    ORFFindTask *task;
};

//FIXME! this class is a partial copy of DetView
class Translator {
public:
    Translator(const DNASequenceObject *s, const QString& tid);

    //void setAminoTranslation(const QString& id);
    DNATranslation* getComplTranslation() const {return complTransl;}
    DNATranslation* getAminoTranslation() const {return aminoTransl;}

private:
    //QStringList translations;
    const DNASequenceObject* seq;
    DNATranslation* complTransl;
    DNATranslation* aminoTransl;

};

} //namespace
#endif

