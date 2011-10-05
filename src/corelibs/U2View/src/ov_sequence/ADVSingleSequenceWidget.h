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

#ifndef _U2_ADV_SINGLE_SEQ_WIDGET_H_
#define _U2_ADV_SINGLE_SEQ_WIDGET_H_

#include "ADVSequenceWidget.h"
#include <U2Core/U2Region.h>
#include <U2Core/GAutoDeleteList.h>
#include "PanView.h"

#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>

#ifdef Q_OS_LINUX
#include <QtCore/QTimer>
#endif // Q_OS_LINUX


namespace U2 {

class AnnotatedDNAView;
class U2SequenceObject;
class GSequenceLineView;
class DetView;
class PanView;
class Overview;
class GSingleSeqBlockState;
class DNATranslation;
class ADVSequenceObjectContext;
class ADVSingleSequenceHeaderWidget;
class Annotation;
class AnnotationSelection;
class DNAAlphabet;

class U2VIEW_EXPORT ADVSingleSequenceWidget : public ADVSequenceWidget {
    Q_OBJECT
public:
    ADVSingleSequenceWidget(ADVSequenceObjectContext* seqCtx, AnnotatedDNAView* ctx);
    ~ADVSingleSequenceWidget();

    const QList<GSequenceLineView*>& getLineViews() const {return lineViews;}

    GSequenceLineView* getPanGSLView() const;

    GSequenceLineView* getDetGSLView() const;

    PanView* getPanView() const {return panView;}

    DetView* getDetView() const {return detView;}

    virtual void centerPosition(int pos, QWidget* skipView = NULL);
    
    ADVSequenceObjectContext*   getActiveSequenceContext() const {return (seqContexts.isEmpty()) ? NULL : seqContexts.first();}
    ADVSequenceObjectContext*   getSequenceContext() const {return getActiveSequenceContext();}
    
    DNATranslation* getComplementTT() const;

    DNATranslation* getAminoTT() const;

    DNASequenceSelection*   getSequenceSelection() const;
    
    int getSequenceLength() const;

    virtual void addSequenceView(GSequenceLineView* v, QWidget* after = NULL);

    virtual void removeSequenceView(GSequenceLineView* v, bool deleteView = true);

    U2SequenceObject* getSequenceObject() const;

    virtual void buildPopupMenu(QMenu& m);

    virtual bool isWidgetOnlyObject(GObject* o) const;

    virtual void addADVSequenceWidgetAction(ADVSequenceWidgetAction* action);

    void setViewCollapsed(bool v);
    bool isViewCollapsed() const;

    void setPanViewCollapsed(bool v);
    bool isPanViewCollapsed() const;

    void setDetViewCollapsed(bool v);
    bool isDetViewCollapsed() const;

    void setOverviewCollapsed(bool v);
    bool isOverviewCollapsed() const;

    virtual void updateState(const QVariantMap& m);
    virtual void updateSelectionActions();
    QVector<U2Region> getSelectedAnnotationRegions(int max);

    virtual void saveState(QVariantMap& m);

    void closeView();

    virtual U2Region getVisibleRange() const;

    virtual void setVisibleRange(const U2Region& r);

    virtual int getNumBasesVisible() const;

    virtual void setNumBasesVisible(int n);
    
    QAction* getSelectRangeAction() const {return selectRangeAction1;}

    virtual void onSequenceObjectRenamed(const QString& oldName);

protected slots:
    void sl_onViewDestroyed(QObject*);

    void sl_toggleView();
    void sl_togglePanView();
    void sl_toggleDetView() {setDetViewCollapsed(!isDetViewCollapsed());}
    void sl_toggleOverview() {setOverviewCollapsed(!isOverviewCollapsed());}
    void sl_onSelectRange();
    void sl_onSelectInRange();
    void sl_onSelectOutRange();
    void sl_zoomToRange();
    void sl_onLocalCenteringRequest(int pos);
    void sl_createCustomRuler();
    void sl_removeCustomRuler();
    
    void sl_onAnnotationSelectionChanged(AnnotationSelection* thiz, const QList<Annotation*>& added, const QList<Annotation*>& removed);

// QT 4.5.0 bug workaround
public slots:
    void sl_closeView();

private slots:
    void sl_saveScreenshot();


protected:
    void init();
    bool eventFilter(QObject* o, QEvent* e);

private:
    virtual void updateMinMaxHeight();

    void addStateActions(QMenu& m);

    void addRulersMenu(QMenu& m);
    void addSelectMenu(QMenu& m);

    virtual GSequenceLineView* findSequenceViewByPos(const QPoint& globalPos) const;

    virtual void addZoomMenu(const QPoint& globalPos, QMenu* m);
    
    DetView*                        detView;
    PanView*                        panView;
    Overview*                       overview;
    QList<GSequenceLineView*>       lineViews;
    QVBoxLayout*                    linesLayout;
    ADVSingleSequenceHeaderWidget   *headerWidget;
    
    QAction*        toggleViewAction;
    QAction*        togglePanViewAction;
    QAction*        toggleDetViewAction;
    QAction*        toggleOverviewAction;
    QAction*        selectRangeAction1;
    QAction*        selectRangeAction2;
    QAction*        selectInAnnotationRangeAction;
    QAction*        selectOutAnnotationRangeAction;
    QAction*        zoomToRangeAction;
    QAction*        createNewRulerAction;
    QAction*        shotScreenAction;

    QList<QMenu*>   tbMenues;
    QToolButton*    ttButton;
    GAutoDeleteList<QAction> rulerActions;

    PanView::ZoomUseObject zoomUseObject;

    friend class ADVSingleSequenceHeaderWidget;
};

class U2VIEW_EXPORT ADVSingleSequenceHeaderWidget : public QWidget {
    Q_OBJECT
public:
    ADVSingleSequenceHeaderWidget(ADVSingleSequenceWidget* p);
    
    QToolBar* getToolBar() const { return toolBar;}
    void setTitle(const QString & title) {nameLabel->setText(title);}
    void updateTitle();

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    
    virtual bool eventFilter (QObject *o, QEvent *e);

protected slots:
    void sl_advFocusChanged(ADVSequenceWidget* prevFocus, ADVSequenceWidget* newFocus);
    void sl_actionTriggered(QAction* a);
    void sl_showStateMenu();
    void sl_closeView();

private:
    void populateToolBars();
    void updateActiveState();
    QString getShortAlphabetName(DNAAlphabet* al);

    ADVSingleSequenceWidget*        ctx;
    QToolBar*                       toolBar;
    QToolBar*                       closeBar;
    QLabel*                         pixLabel;
    QLabel*                         nameLabel;

    QAction*        closeViewAction;
    QToolButton*    widgetStateMenuButton;
};

}//namespace

#endif

