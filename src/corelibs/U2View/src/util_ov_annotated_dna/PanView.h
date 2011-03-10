#ifndef _U2_PAN_VIEW_H_
#define _U2_PAN_VIEW_H_

#include "GSequenceLineViewAnnotated.h"

#include <QtGui/QFont>
#include <QtGui/QAction>
#include <QtGui/QScrollBar>

namespace U2 {

class PanViewRenderArea;
class GScrollBar;
class GObjectView;
class PVRowsManager;
class AnnotationTableObject;
class Annotation;

class RulerInfo {
public:
    RulerInfo(): offset(0){}
    RulerInfo(const QString& _name, int _offset, const QColor& _color) : name(_name), offset(_offset), color(_color) {}
    
    QString name;
    int     offset;
    QColor  color;
};

class U2VIEW_EXPORT PanView : public GSequenceLineViewAnnotated {
    Q_OBJECT

public:

    class U2VIEW_EXPORT ZoomUseObject {
    public:
        ZoomUseObject();
        ZoomUseObject(PanView *pv);
        virtual ~ZoomUseObject();

        void useZoom();
        void releaseZoom();
        void setPanView(PanView *pv);
    private:
        bool usingZoom;
        PanView *panView;
    };

    PanView(QWidget* p, ADVSequenceObjectContext* ctx);
    ~PanView();

    const U2Region& getFrameRange() const {return frameView->getVisibleRange();}

    virtual QAction* getZoomInAction() const {return zoomInAction;}

    virtual QAction* getZoomOutAction() const {return zoomOutAction;}

    virtual QAction* getZoomToSelectionAction() const {return zoomToSelectionAction;}

    virtual QAction* getZoomToSequenceAction() const {return zoomToSequenceAction;}

    // [0..seqLen)
    virtual void setVisibleRange(const U2Region& reg, bool signal = true);

    int getSeqLen() const {return seqLen;}

    PVRowsManager* getRowsManager() const {return rowsManager;}

    virtual void setNumBasesVisible(int n);

    void setSyncOffset(int o);
 
    int getSyncOffset() const {return syncOffset;}

    QList<RulerInfo> getCustomRulers() const;

    void addCustomRuler(const RulerInfo& r);

    void removeCustomRuler(const QString& name);

    void removeAllCustomRulers();

    QAction* getToggleMainRulerAction() const {return toggleMainRulerAction;}
    
    QAction* getToggleCustomRulersAction() const {return toggleCustomRulersAction;}

    void hideEvent(QHideEvent *ev);

    void showEvent(QShowEvent *ev);

protected:
    virtual int getSingleStep() const {return qMax(1, int(visibleRange.length) / 10);}
    virtual int getPageStep() const {return qMax(1, int(visibleRange.length) / 5);}
    virtual void onVisibleRangeChanged(bool signal = true);
    virtual void pack();

    virtual void registerAnnotations(const QList<Annotation*>& l);
    virtual void unregisterAnnotations(const QList<Annotation*>& l);
    virtual void ensureVisible(Annotation* a, int locationIdx);
protected slots:
    virtual void sl_sequenceChanged();
    void sl_onAnnotationsModified(const AnnotationModification& md);

private slots:
    void sl_zoomInAction();
    void sl_zoomOutAction();
    void sl_zoomToSelection();
    void sl_zoomToSequence();
    
    void sl_onRowBarMoved(int);

    void sl_onRangeChangeRequest(int start, int end);

    virtual void sl_onDNASelectionChanged(LRegionsSelection* s, const QVector<U2Region>& added, const QVector<U2Region>& removed);
    virtual void sl_onAnnotationSettingsChanged(const QStringList& changedSettings);

    void sl_toggleMainRulerVisibility(bool visible);
    void sl_toggleCustomRulersVisibility(bool visible);

private:
    void setSelection(const U2Region& r);
    void centerRow(int row);
    int  calculateNumRowBarSteps() const;

    void updateActions();
    void updateRows();
    void updateRowBar();
    void updateRAHeight();

    void useZoom();
    void releaseZoom();

    PanViewRenderArea* getRenderArea() const;

    U2Region             frameRange;
    int                 minNuclsPerScreen;

    QAction*            zoomInAction;
    QAction*            zoomOutAction;
    QAction*            zoomToSelectionAction;
    QAction*            zoomToSequenceAction;
    QAction*            toggleMainRulerAction;
    QAction*            toggleCustomRulersAction;

    PVRowsManager*      rowsManager;
    QScrollBar*         rowBar;
    int                 syncOffset; //used by ADVSyncViewManager only

    int zoomUsing;
};


class PanViewRenderArea : public GSequenceLineViewAnnotatedRenderArea {
    friend class PanView;
    Q_OBJECT
public:
    PanViewRenderArea(PanView* d);

    bool isRowVisible(int row) const { return getRowLine(row) >= 0; }
    int getRowLine(int i) const;
    int getNumVisibleRows() const {return getFirstRowLine() + 1;}
    int getRowLinesOffset() const {return rowLinesOffset;}
    void setRowLinesOffset(int r);

    virtual U2Region getAnnotationYRange(Annotation* a, int region, const AnnotationSettings* as) const;

    bool updateNumVisibleRows();

protected:
    virtual void drawAll(QPaintDevice* pd);
    virtual void drawAnnotations(QPainter& p);
    virtual void drawSequence(QPainter& p);

private:
    int getSelectionLine() const {return numLines - 1;}

    int getRulerLine() const {
        assert(showMainRuler); 
        return numLines - 2;
    }

    int getCustomRulerLine(int n) const {
       assert(showCustomRulers); 
       assert(n >= 0 && n < customRulers.count());
        return numLines - (showMainRuler ? 3 : 2) - n;
    }

    int getFirstRowLine()const {return numLines - 2 - (showMainRuler ? 1 : 0) - (showCustomRulers ? customRulers.count() : 0 );}
    
    bool isSequenceCharsVisible() const;

    PanView* getPanView() const {return (PanView*)view;}

    int getLineY(int line) const {
        return cachedView->height() - ((numLines - line) * lineHeight);
    }

    void drawRuler(QPainter& p);
    void drawCustomRulers(QPainter& p);
    void drawSequenceSelection(QPainter& p);

    PanView*            panView;
    int                 numLines;       // number of visible lines
    int                 rowLinesOffset; // row number on the first row line

    bool                showMainRuler;
    bool                showCustomRulers;
    QList<RulerInfo>    customRulers;
};


}//namespace;

#endif
