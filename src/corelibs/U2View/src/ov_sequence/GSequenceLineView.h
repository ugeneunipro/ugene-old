/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GSEQUENCE_LINE_VIEW_H_
#define _U2_GSEQUENCE_LINE_VIEW_H_

#include <U2Core/U2Region.h>

#include <QtCore/QFlag>
#include <QtGui/QWidget>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QFocusEvent>
#include <QtGui/QPainter>
#include <QtGui/QMenu>


namespace U2 {

class GSequenceLineViewRenderArea;
class DNASequenceSelection;
class LRegionsSelection;
class GScrollBar;
class ADVSequenceObjectContext;
class GObjectViewOpConstraints;
class GObject;
class U2SequenceObject;

enum GSLV_UpdateFlag {
    GSLV_UF_NeedCompleteRedraw  = 1<<0,
    GSLV_UF_ViewResized         = 1<<1,
    GSLV_UF_VisibleRangeChanged = 1<<2,
    GSLV_UF_SelectionChanged    = 1<<3,
    GSLV_UF_FocusChanged        = 1<<4,
    GSLV_UF_FrameChanged        = 1<<5,
    GSLV_UF_AnnotationsChanged  = 1<<6
};

enum GSLV_FeatureFlag {
    GSLV_FF_SupportsCustomRange = 0x1
};

typedef QFlags<GSLV_UpdateFlag> GSLV_UpdateFlags;
typedef QFlags<GSLV_FeatureFlag> GSLV_FeatureFlags;

//single-line sequence view
class U2VIEW_EXPORT GSequenceLineView : public QWidget {
    Q_OBJECT
public:
    GSequenceLineView(QWidget* p, ADVSequenceObjectContext* ctx);

    const U2Region& getVisibleRange() const {return visibleRange;}

    ADVSequenceObjectContext* getSequenceContext() const {return ctx;}

    virtual void setStartPos(qint64 pos);

    virtual void setCenterPos(qint64 pos);

    qint64 getSequenceLength() const {return seqLen;}
    
    virtual void addUpdateFlags(GSLV_UpdateFlags newFlags) {lastUpdateFlags|=newFlags;}
       
    virtual void clearUpdateFlags() {lastUpdateFlags = 0;}

    GSLV_UpdateFlags getUpdateFlags() const {return lastUpdateFlags;}

    virtual void setFrameView(GSequenceLineView* frameView); 

    virtual GSequenceLineView* getFrameView() const {return frameView;}
    
    virtual void setCoherentRangeView(GSequenceLineView* rangeView);

    virtual GSequenceLineView* getConherentRangeView() const {return coherentRangeView;}

    // [0..seqLen)
    virtual void setVisibleRange(const U2Region& reg, bool signal = true);

    virtual QAction* getZoomInAction() const {return coherentRangeView == NULL ? NULL : coherentRangeView->getZoomInAction();}

    virtual QAction* getZoomOutAction() const {return coherentRangeView == NULL ? NULL : coherentRangeView->getZoomOutAction();}

    virtual QAction* getZoomToSelectionAction() const {return coherentRangeView == NULL ? NULL : coherentRangeView->getZoomToSelectionAction();}

    virtual QAction* getZoomToSequenceAction() const {return coherentRangeView == NULL ? NULL : coherentRangeView->getZoomToSequenceAction();}
    
    virtual U2SequenceObject* getSequenceObject() const;

    virtual void buildPopupMenu(QMenu& m){ Q_UNUSED(m); }

    virtual bool isWidgetOnlyObject(GObject* o) const { Q_UNUSED(o); return false;}

    virtual bool eventFilter (QObject * watched, QEvent * event);

signals:
    void si_visibleRangeChanged();
    void si_centerPosition(qint64 pos);

protected:
    void resizeEvent(QResizeEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* me);
    void mousePressEvent(QMouseEvent* me);
    void mouseReleaseEvent(QMouseEvent* me);
    void mouseMoveEvent(QMouseEvent* me);
    void wheelEvent(QWheelEvent* we);
    void focusInEvent(QFocusEvent* fe);
    void focusOutEvent(QFocusEvent* fe);
    void keyPressEvent(QKeyEvent *e);
    
    virtual void onVisibleRangeChanged(bool signal = true);

public slots:
    void sl_centerPosition(int pos) {setCenterPos(pos);}

protected slots:
    virtual void sl_onScrollBarMoved(int pos);
    virtual void sl_onDNASelectionChanged(LRegionsSelection* thiz, const QVector<U2Region>& added, const QVector<U2Region>& removed);
    virtual void sl_sequenceChanged();
    void sl_onFrameRangeChanged();
    void sl_onCoherentRangeViewRangeChanged();
    void completeUpdate(); 

protected:
    QPoint toRenderAreaPoint(const QPoint& p);
    void updateScrollBar();
    void setSelection(const U2Region& r);
    void addSelection(const U2Region& r);
    void removeSelection(const U2Region& r);
    virtual void pack();
    virtual int getSingleStep() const;
    virtual int getPageStep() const;

    ADVSequenceObjectContext*       ctx;
    GSequenceLineViewRenderArea*    renderArea;
    U2Region                        visibleRange;
    GScrollBar*                     scrollBar;
    qint64                          lastPressPos;
    qint64                          seqLen;
    GSLV_UpdateFlags                lastUpdateFlags;
    GSLV_FeatureFlags               featureFlags;
    GSequenceLineView*              frameView;
    GSequenceLineView*              coherentRangeView;
    double coefScrollBarMapping;

    // special flag setup by child classes that tells to this class do or skip
    // any changes to selection on mouse ops
    bool                            ignoreMouseSelectionEvents;


};

class U2VIEW_EXPORT GSequenceLineViewRenderArea : public QWidget  {
public:
    GSequenceLineViewRenderArea(GSequenceLineView* p);
    ~GSequenceLineViewRenderArea();

    virtual qint64 coordToPos(int x) const;
    virtual int posToCoord(qint64 p, bool useVirtualSpace = false) const;
    virtual float posToCoordF(qint64 p, bool useVirtualSpace = false) const;
    //number of pixels per base
    virtual double getCurrentScale() const;
    //char width, derived from current 'font'
    int getCharWidth() const {return charWidth;}

protected:
    virtual void paintEvent(QPaintEvent *e);
    
    virtual void drawAll(QPaintDevice* pd) = 0;
    void drawFrame(QPainter& p);
    virtual void drawFocus(QPainter& p);

    void updateFontMetrics();

    GSequenceLineView* view;
    QPixmap* cachedView;
    QPixmap* tmpView;

    //per char and per line metrics
    QFont   sequenceFont;
    QFont   smallSequenceFont;
    QFont   rulerFont;
    
    int     charWidth;
    int     smallCharWidth;

    int     lineHeight;
    int     yCharOffset;
    int     xCharOffset;
};

} //namespace

#endif
