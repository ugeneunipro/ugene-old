#ifndef _U2_ADV_SEQUENCE_OBJECT_CONTEXT_H_
#define _U2_ADV_SEQUENCE_OBJECT_CONTEXT_H_

#include  <U2Core/global.h>

#include <QtCore/QSet>
#include <QtGui/QWidget>
#include <QtGui/QMenu>

namespace U2 {

class AnnotatedDNAView;
class DNASequenceObject;
class DNAAlphabet;
class DNATranslation;
class DNASequenceSelection;
class ADVSequenceWidget;
class AnnotationTableObject;
class AnnotationSelection;
class GObject;
class Annotation;

class U2VIEW_EXPORT ADVSequenceObjectContext : public QObject {
    Q_OBJECT
public:
    ADVSequenceObjectContext(AnnotatedDNAView* v, DNASequenceObject* obj);
    
    AnnotatedDNAView* getAnnotatedDNAView() const {return view;}
    DNATranslation* getComplementTT() const {return complTT;}
    DNATranslation* getAminoTT() const {return aminoTT;}
    DNASequenceObject* getSequenceObject() const {return seqObj;}
    GObject*           getSequenceGObject() const;

    const QByteArray& getSequenceData() const;
    int getSequenceLen() const;
    DNAAlphabet* getAlphabet() const;

    DNASequenceSelection*   getSequenceSelection() const {return selection;}
    
    QSet<AnnotationTableObject*> getAnnotationObjects(bool includeAutoAnnotations = false) const;
    QList<GObject*> getAnnotationGObjects() const;

    QMenu* createTranslationsMenu();
    void setAminoTranslation(const QString& tid);

    void addAnnotationObject(AnnotationTableObject* obj);
    void addAutoAnnotationObject(AnnotationTableObject* obj);
    void removeAnnotationObject(AnnotationTableObject* obj);

    AnnotationSelection* getAnnotationsSelection() const;
    
    const QList<ADVSequenceWidget*>& getSequenceWidgets() const {return seqWidgets;}
    void addSequenceWidget(ADVSequenceWidget* w);
    void removeSequenceWidget(ADVSequenceWidget* w);

    QList<Annotation*> selectRelatedAnnotations(const QList<Annotation*>& alist) const;

private slots:
    void sl_setAminoTranslation();
signals:
    void si_aminoTranslationChanged();
    void si_annotationObjectAdded(AnnotationTableObject* obj);
    void si_annotationObjectRemoved(AnnotationTableObject* obj);

private:
    void guessAminoTT(AnnotationTableObject* ao);


    AnnotatedDNAView*               view;
    DNASequenceObject*              seqObj;
    DNATranslation*                 aminoTT;
    DNATranslation*                 complTT;
    DNASequenceSelection*           selection;
    QActionGroup*                   translations;
    QList<ADVSequenceWidget*>       seqWidgets;
    QSet<AnnotationTableObject*>    annotations;
    QSet<AnnotationTableObject*>    autoAnnotations;
    bool                            clarifyAminoTT;
};

} //namespace

#endif
