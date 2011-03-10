#ifndef _U2_ANNOTATED_DNA_VIEW_CLIPBOARD_H_
#define _U2_ANNOTATED_DNA_VIEW_CLIPBOARD_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <QtGui/QAction>

namespace U2 {

class AnnotatedDNAView;
class LRegionsSelection;
class Annotation;
class AnnotationSelection;
class ADVSequenceObjectContext;
class ADVSequenceWidget;

class U2VIEW_EXPORT ADVClipboard: public QObject {
    Q_OBJECT
public:
    ADVClipboard(AnnotatedDNAView* ctx);

    QAction* getCopySequenceAction() const {return copySequenceAction;}
    QAction* getCopyTranslationAction() const {return copyTranslationAction;}
    QAction* getCopyComplementAction() const {return copyComplementSequenceAction;}
    QAction* getCopyComplementTranslationAction() const {return copyComplementTranslationAction;}

    QAction* getCopyAnnotationSequenceAction() const {return copyAnnotationSequenceAction;}
    QAction* getCopyAnnotationSequenceTranslationAction() const {return copyAnnotationSequenceTranslationAction;}

    void addCopyMenu(QMenu* m);

public slots:
    
    void sl_onDNASelectionChanged(LRegionsSelection* s, const QVector<U2Region>& added, const QVector<U2Region>& removed);
    void sl_onAnnotationSelectionChanged(AnnotationSelection* s, const QList<Annotation*>& added, const QList<Annotation*>& removed);
    void sl_onFocusedSequenceWidgetChanged(ADVSequenceWidget*, ADVSequenceWidget*);

    void sl_copySequence();
    void sl_copyTranslation();
    void sl_copyComplementSequence();
    void sl_copyComplementTranslation();
    void sl_copyAnnotationSequence();
    void sl_copyAnnotationSequenceTranslation();

    ADVSequenceObjectContext* getSequenceContext() const;

private:
    void connectSequence(ADVSequenceObjectContext* s);
    void updateActions();
    void copySequenceSelection(bool complement, bool amino);

    AnnotatedDNAView* ctx;
    QAction* copySequenceAction;
    QAction* copyTranslationAction;
    QAction* copyComplementSequenceAction;
    QAction* copyComplementTranslationAction;
    QAction* copyAnnotationSequenceAction;
    QAction* copyAnnotationSequenceTranslationAction;
};

}//namespace

#endif
