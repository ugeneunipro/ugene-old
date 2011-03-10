#ifndef _U2_DNA_FRAGMENT_H_
#define _U2_DNA_FRAGMENT_H_

#include <QtCore/QList>


#include <U2Core/U2Region.h>


namespace U2 {

class DNASequenceObject;
class DNAAlphabet;
class AnnotationTableObject;
class Annotation;
class GObject;


struct DNAFragmentTerminus {
    QByteArray enzymeId; 
    QByteArray overhang;
    QByteArray termType;
};

class DNAFragment {
private:
    // Contains fragment region and cuts info
    Annotation* annotatedFragment;
    // Parent sequence of the fragment
    DNASequenceObject* dnaObj;
    // Annotations associated with parent sequence
    // When ligate or export fragments they must be saved
    QList<AnnotationTableObject*> relatedAnnotations;
    // reverse complement
    bool reverseCompl;
    bool isSplitted() const;
    void updateTerms();
    void updateLeftTerm();
    void updateRightTerm();
    void toRevCompl(QByteArray& buf);
    void setTermType(const QByteArray& qName, const QByteArray& type);
    void setOverhang(const QByteArray& qName, const QByteArray& overhang);
    DNAFragmentTerminus leftTerm, rightTerm;

public:
    DNAFragment() : annotatedFragment(NULL), dnaObj(NULL), reverseCompl(false) {}
    DNAFragment(Annotation* fragment, DNASequenceObject* sObj, const QList<AnnotationTableObject*> relatedAnns);
    DNAFragment( const DNAFragment& other );
    DNAFragment& operator=(const DNAFragment& other);
    bool isEmpty() const { return annotatedFragment == NULL || dnaObj == NULL; }
    QString getName() const;
    QString getSequenceName() const;
    QString getSequenceDocName() const;
    QVector<U2Region> getFragmentRegions() const; 
    QByteArray getSequence() const;
    int getLength() const;
    bool isInverted() const { return reverseCompl; }
    DNAAlphabet* getAlphabet() const;
    const QByteArray& getSourceSequence() const;
    const DNAFragmentTerminus& getLeftTerminus() const;
    const DNAFragmentTerminus& getRightTerminus() const;
    void setInverted(bool inverted = true);
    void setRightTermType( const QByteArray& termType );
    void setLeftTermType( const QByteArray& termType );
    void setLeftOverhang( const QByteArray& overhang );
    void setRightOverhang( const QByteArray& overhang );


    const QList<AnnotationTableObject*>& getRelatedAnnotations() const { return relatedAnnotations; }
    
    static QList<DNAFragment> findAvailableFragments();
    static QList<DNAFragment> findAvailableFragments(const QList<GObject*>& aObjects, const QList<GObject*>& sObjects);
   
};



} //namespace

#endif // _U2_DNA_FRAGMENT_H_
