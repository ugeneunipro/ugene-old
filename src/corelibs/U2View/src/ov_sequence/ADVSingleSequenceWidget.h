/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QSplitter>

#ifdef Q_OS_UNIX
#include <QtCore/QTimer>
#endif // Q_OS_UNIX


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

    void addADVSequenceWidgetActionToViewsToolbar(ADVSequenceWidgetAction* action);

    void setViewCollapsed(bool v);
    bool isViewCollapsed() const;
    void updateViewButtonState();

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

    virtual void setNumBasesVisible(qint64 n);

    QAction* getSelectRangeAction() const {return selectRangeAction1;}

    virtual void onSequenceObjectRenamed(const QString& oldName);

signals:
    void si_titleClicked(ADVSequenceWidget*);

protected slots:
    void sl_onViewDestroyed(QObject*);

    void sl_toggleView();
    void sl_togglePanView();
    void sl_toggleDetView();
    void sl_toggleOverview();
    void sl_onSelectRange();
    void sl_onSelectInRange();
    void sl_onSelectOutRange();
    void sl_zoomToRange();
    void sl_onLocalCenteringRequest(qint64 pos);
    void sl_createCustomRuler();
    void sl_removeCustomRuler();

    void sl_onAnnotationSelectionChanged( AnnotationSelection *thiz, const QList<Annotation *> &added, const QList<Annotation *> &removed );

// QT 4.5.0 bug workaround
public slots:
    void sl_closeView();

signals:
    void si_updateGraphView(const QStringList &, const QVariantMap&);
private slots:
    void sl_saveScreenshot();


protected:
    void init();
    bool eventFilter(QObject* o, QEvent* e);

private:
    virtual void updateMinMaxHeight();

    QToolButton* addButtonWithActionToToolbar(QAction * buttonAction, QToolBar * toolBar, int position = -1) const;
    void addRulersMenu(QMenu& m);
    void addSelectMenu(QMenu& m);
    void setupGeneticCodeMenu(ADVSequenceObjectContext *seqCtx);

    /** Used by several other functions to set new selected region */
    void setSelectedRegion(const U2Region& region);

    virtual GSequenceLineView* findSequenceViewByPos(const QPoint& globalPos) const;

    virtual void addZoomMenu(const QPoint& globalPos, QMenu* m);

    DetView*                        detView;
    PanView*                        panView;
    Overview*                       overview;
    QList<GSequenceLineView*>       lineViews;
    QVBoxLayout*                    linesLayout;
    QSplitter*                      linesSplitter;
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
    QAction*        closeViewAction;

    QList<QMenu*>   tbMenues;
    QToolButton*    ttButton;
    GAutoDeleteList<QAction> rulerActions;
    QList<QString> * buttonTabOrederedNames;

    PanView::ZoomUseObject zoomUseObject;

    friend class ADVSingleSequenceHeaderWidget;
};

class U2VIEW_EXPORT ADVSingleSequenceHeaderWidget : public QWidget {
    Q_OBJECT
public:
    ADVSingleSequenceHeaderWidget(ADVSingleSequenceWidget* p);

    QToolBar* getStandardToolBar() const { return standardToolBar;}
    QToolBar* getViewsToolBar() const { return viewsToolBar; }

    void setTitle(const QString & title) {nameLabel->setText(title);}
    void updateTitle();

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);

protected slots:
    void sl_advFocusChanged(ADVSequenceWidget* prevFocus, ADVSequenceWidget* newFocus);
    void sl_actionTriggered(QAction* a);

private:
    void updateActiveState();
    QString getShortAlphabetName(const DNAAlphabet* al);

    ADVSingleSequenceWidget*        ctx;
    QToolBar*                       standardToolBar;
    QToolBar*                       viewsToolBar;
    QLabel*                         pixLabel;
    QLabel*                         nameLabel;
};

}//namespace

#endif

