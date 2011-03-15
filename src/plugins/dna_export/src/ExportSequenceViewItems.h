#ifndef _U2_EXPORT_SEQUENCE_VIEW_ITEMS_H_
#define _U2_EXPORT_SEQUENCE_VIEW_ITEMS_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class ADVSequenceObjectContext;
class AnnotatedDNAView;
class AnnotationSelection;
class Annotation;
class LRegionsSelection;
class MAlignment;

class ExportSequenceViewItemsController : public GObjectViewWindowContext {
    Q_OBJECT
public:
    ExportSequenceViewItemsController(QObject* p);


protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* v, QMenu* m);
};

class ADVExportContext : public QObject {
    Q_OBJECT;
public:
    ADVExportContext(AnnotatedDNAView* v);
    void buildMenu(QMenu* m);

protected slots:
    void sl_saveSelectedSequences();
    void sl_saveSelectedAnnotationsSequence();
    void sl_saveSelectedAnnotations();
    void sl_saveSelectedAnnotationsToAlignment();
    void sl_saveSelectedAnnotationsToAlignmentWithTranslation();
    void sl_saveSelectedSequenceToAlignment();
    void sl_saveSelectedSequenceToAlignmentWithTranslation();
    void sl_getSequenceByDBXref();
    void sl_getSequenceByAccession();
    void sl_getSequenceById();

    void sl_onSequenceContextAdded(ADVSequenceObjectContext* c);
    void sl_onSequenceContextRemoved(ADVSequenceObjectContext* c);

    void sl_onAnnotationSelectionChanged(AnnotationSelection*, const QList<Annotation*>& added, const QList<Annotation*>& removed);
    void sl_onSequenceSelectionChanged(LRegionsSelection* thiz, const QVector<U2Region>& added, const QVector<U2Region>& removed);

    void updateActions();

private:
    QString prepareMAFromAnnotations(MAlignment& ma, bool translate);
    QString prepareMAFromSequences(MAlignment& ma, bool translate);
    void fetchSequencesFromRemoteDB(const QString & listId);

    void selectionToAlignment(const QString& title, bool annotations, bool translate);

    AnnotatedDNAView*   view;
    
    QAction*  sequence2SequenceAction;
    QAction*  annotations2SequenceAction;
    QAction*  annotations2CSVAction;
    QAction*  annotationsToAlignmentAction;
    QAction*  annotationsToAlignmentWithTranslatedAction;
    QAction*  sequenceToAlignmentAction;
    QAction*  sequenceToAlignmentWithTranslationAction;
    QAction*  sequenceById;
    QAction*  sequenceByAccession;
    QAction*  sequenceByDBXref;

};

}//namespace

#endif
