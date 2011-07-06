/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_ANNOTATED_DNA_VIEW_H_
#define _U2_ANNOTATED_DNA_VIEW_H_

#include <U2Gui/ObjectViewModel.h>

#include <QtCore/QPointer>
#include <QtGui/QTextEdit>
#include <QtGui/QSplitter>

class QScrollArea;
class QVBoxLayout;

namespace U2 {

class AnnotatedDNAView;
class AnnotatedDNAViewState;
class DNASequenceObject;
class AnnotationTableObject;
class GSequenceLineView;
class AnnotationsTreeViewL;
class AnnotationSelection;
class AnnotationGroupSelection;
class DNASequenceSelection;
class Task;
class ADVClipboard;
class ADVSequenceWidget;
class ADVSplitWidget;
class ADVSequenceObjectContext;
class PositionSelector;
class GObjectReference;
class ADVSyncViewManager;
class ADVObjectHandler;
class ADVGlobalAction;
class AutoAnnotationObject;
class AutoAnnotationsUpdater;

class U2VIEW_EXPORT AnnotatedDNAView : public GObjectView {
    Q_OBJECT
public:
    AnnotatedDNAView(const QString& viewName, const QList<DNASequenceObject*>& dnaObjects);
    ~AnnotatedDNAView();

    virtual void buildStaticToolbar(QToolBar* tb);

    virtual void buildStaticMenu(QMenu* n);

    virtual Task* updateViewTask(const QString& stateName, const QVariantMap& stateData);

    virtual QVariantMap saveState();
    
    
    
    // view content
    const QList<ADVSequenceObjectContext*>& getSequenceContexts() const {return seqContexts;}

    QList<DNASequenceObject*> getSequenceObjectsWithContexts() const;
    
    QList<GObject*> getSequenceGObjectsWithContexts() const;

    QList<AnnotationTableObject*> getAnnotationObjects( bool includeAutoAnnotations = false) const;

    AnnotationSelection*        getAnnotationsSelection() const {return annotationSelection;}
    
    AnnotationGroupSelection*   getAnnotationsGroupSelection() const {return annotationGroupSelection;}

    const QList<ADVSequenceWidget*> getSequenceWidgets() const {return seqViews;}

    virtual bool canAddObject(GObject* obj);

    void addSequenceWidget(ADVSequenceWidget* v);

    void removeSequenceWidget(ADVSequenceWidget* v);

    void insertWidgetIntoSplitter(ADVSplitWidget* widget);

    void unregisterSplitWidget(ADVSplitWidget* widget);
    
    virtual QString addObject(GObject* o);

    void saveWidgetState();

    ADVSequenceObjectContext* getSequenceContext(AnnotationTableObject* obj) const;

    ADVSequenceObjectContext* getSequenceContext(DNASequenceObject*) const;
    
    ADVSequenceObjectContext* getSequenceContext(const GObjectReference& r) const;

    QWidget* getScrolledWidget() const {return scrolledWidget;}

    ADVSequenceWidget* getSequenceWidgetInFocus() const {return focusedWidget;}

    ADVSequenceObjectContext* getSequenceInFocus() const;

    QList<ADVSequenceObjectContext*> getAllSeqContextsInFocus() const;

    QList<ADVSequenceObjectContext*> findRelatedSequenceContexts(GObject* obj) const;

    void setFocusedSequenceWidget(ADVSequenceWidget* v);

    void updateState(const AnnotatedDNAViewState& s);

    QAction* getCreateAnnotationAction() const {return createAnnotationAction;}
    
    void addADVAction(ADVGlobalAction* a);
    
    void removeADVAction(ADVGlobalAction* a) {advActions.removeAll(a);} 

    AnnotationsTreeViewL* getAnnotationsView() {return annotationsView;}

    void updateAutoAnnotations();

    void addAutoAnnotationsUpdated(AutoAnnotationsUpdater* updater);

    // Tries to add object to the view. Uses GUI functions to ask user if some data if needed
    // Returns error message if failed.
    // If object is unloaded - intitiates async object loading
    QString tryAddObject(GObject* obj);

protected:
    virtual QWidget* createWidget();
    virtual bool onObjectRemoved(GObject* o);
    virtual void onObjectRenamed(GObject* obj, const QString& oldName);
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void timerEvent(QTimerEvent *e);
    
    virtual bool isChildWidgetObject(GObject* o) const;
    virtual void addAnalyseMenu(QMenu* m);
    virtual void addAddMenu(QMenu* m);
    virtual void addExportMenu(QMenu* m);
    virtual void addAlignMenu(QMenu* m);
    virtual void addRemoveMenu(QMenu* m);
    virtual void addEditMenu(QMenu* m);
    virtual ADVSequenceWidget* findSequenceWidgetByPos(const QPoint& globalPos) const;

signals:
    void si_sequenceAdded(ADVSequenceObjectContext* c);
    void si_sequenceRemoved(ADVSequenceObjectContext* c);

    void si_annotationObjectAdded(AnnotationTableObject* obj);
    void si_annotationObjectRemoved(AnnotationTableObject* obj);

    void si_sequenceWidgetAdded(ADVSequenceWidget* w);
    void si_sequenceWidgetRemoved(ADVSequenceWidget* w);

    void si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*);
   
private slots:
    void sl_onContextMenuRequested(const QPoint & pos);
    void sl_onFindDialog();
    void sl_onAnnotationSettings();
    void sl_onPosChangeRequest(int pos);
    void sl_onShowPosSelectorRequest();
    void sl_toggleHL();
    void sl_splitterMoved(int, int);

    void sl_addSequencePart();
    void sl_removeSequencePart();
    void sl_replaceSequencePart();
    void sl_sequenceModifyTaskStateChanged();
    
    virtual void sl_onDocumentAdded(Document*);
    virtual void sl_onDocumentLoadedStateChanged();
    virtual void sl_removeSelectedSequenceObject();

private:
    void updateScrollAreaHeight();
    void updateMultiViewActions();
    
    void addRelatedAnnotations(ADVSequenceObjectContext* seqCtx);
    void addAutoAnnotations(ADVSequenceObjectContext* seqCtx);
    void importDocAnnotations(Document* doc);

    QSplitter*          mainSplitter;
    QScrollArea*        scrollArea;
    QWidget*            scrolledWidget;
    QVBoxLayout*        scrolledWidgetLayout;
    
    QAction*            createAnnotationAction;
    QAction*            findDialogAction;
    QAction*            annotationSettingsAction;
    QAction*            posSelectorAction;
    QAction*            toggleHLAction;
    QAction*            posSelectorWidgetAction;

    QAction*            addSequencePart;
    QAction*            removeSequencePart;
    QAction*            replaceSequencePart;
    QAction*            removeSequenceObjectAction;         
    
    PositionSelector*   posSelector;
    
    QList<ADVSequenceObjectContext*>    seqContexts;
    QList<AnnotationTableObject*>       annotations;
    QList<ADVObjectHandler*>            handlers; 
    QList<ADVGlobalAction*>             advActions;
    
    QMap<ADVSequenceObjectContext*, AutoAnnotationObject*>   autoAnnotationsMap;

    AnnotationsTreeViewL*        annotationsView;
    QList<ADVSequenceWidget*>   seqViews;
    QList<ADVSplitWidget*>      splitWidgets;

    AnnotationSelection *       annotationSelection;
    AnnotationGroupSelection *  annotationGroupSelection;
    
    ADVClipboard*               clipb;
    ADVSyncViewManager*         syncViewManager;

    ADVSequenceWidget*          focusedWidget;
    
    int                         timerId;
};

}//namespace;

#endif
