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

#ifndef _U2_PAN_VIEW_H_
#define _U2_PAN_VIEW_H_

#include <QAction>
#include <QFont>
#include <QScrollBar>
#include <QToolButton>

#include <U2Core/Annotation.h>

#include <U2Gui/GraphUtils.h>

#include "GSequenceLineViewAnnotated.h"

namespace U2 {

class PanViewRenderArea;
class GScrollBar;
class GObjectView;
class PVRowsManager;
class ADVSingleSequenceWidget;
class PVRowData;
class PanViewRenderer;


class RulerInfo {
public:
    RulerInfo(): offset(0){}
    RulerInfo(const QString& _name, int _offset, const QColor& _color) : name(_name), offset(_offset), color(_color) {}

    QString name;
    int     offset;
    QColor  color;
};

class PanViewLinesSettings {
public:
    PanViewLinesSettings();

    int getRowLine(int i) const;
    int getSelectionLine() const;

    int getRulerLine() const;
    int getCustomRulerLine(int n) const;

    int getFirstRowLine() const;
    int getAdditionalLines() const;

    int getNumVisibleRows() const;

    bool isRowVisible(int row) const;

    int                 numLines;       // number of visible lines
    int                 rowLinesOffset; // row number on the first row line

    bool                showMainRuler;
    bool                showCustomRulers;
    QList<RulerInfo>    customRulers;
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

    PanView(ADVSingleSequenceWidget* p, ADVSequenceObjectContext* ctx);
    ~PanView();

    const U2Region& getFrameRange() const {return frameView->getVisibleRange();}

    virtual QAction* getZoomInAction() const {return zoomInAction;}

    virtual QAction* getZoomOutAction() const {return zoomOutAction;}

    virtual QAction* getZoomToSelectionAction() const {return zoomToSelectionAction;}

    virtual QAction* getZoomToSequenceAction() const {return zoomToSequenceAction;}

    // [0..seqLen)
    virtual void setVisibleRange(const U2Region& reg, bool signal = true);

    PVRowsManager* getRowsManager() const {return rowsManager;}

    virtual void setNumBasesVisible(qint64 n);

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

    PanViewLinesSettings* getLinesSettings() const {return settings; }

protected:
    virtual int getSingleStep() const {return qMax(1, int(visibleRange.length) / 10);}
    virtual int getPageStep() const {return qMax(1, int(visibleRange.length) / 5);}
    virtual void onVisibleRangeChanged(bool signal = true);
    virtual void pack();

    virtual void registerAnnotations(const QList<Annotation *> &l);
    virtual void unregisterAnnotations(const QList<Annotation *> &l);
    virtual void ensureVisible(Annotation *a, int locationIdx);
protected slots:
    virtual void sl_sequenceChanged();
    void sl_onAnnotationsModified(const AnnotationModification& md);

private slots:
    void sl_zoomInAction();
    void sl_zoomOutAction();
    void sl_zoomToSelection();
    void sl_zoomToSequence();

    void sl_onRowBarMoved(int);

    void sl_onRangeChangeRequest(qint64 start, qint64 end);

    virtual void sl_onDNASelectionChanged(LRegionsSelection* s, const QVector<U2Region>& added, const QVector<U2Region>& removed);
    virtual void sl_onAnnotationSettingsChanged(const QStringList& changedSettings);

    void sl_toggleMainRulerVisibility(bool visible);
    void sl_toggleCustomRulersVisibility(bool visible);
    void sl_updateRows();

signals:
    void si_updateRows();

public:
    void setSelection(const U2Region& r);
    void centerRow(int row);
    int  calculateNumRowBarSteps() const;

    void setNumVisibleRows(int numRows);
    void updateNumVisibleRows();

    void updateActions();
    void updateRows();
    void updateRowBar();

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

    PanViewLinesSettings*     settings;

    int zoomUsing;

    ADVSingleSequenceWidget*    seqWidget;
};


class PanViewRenderArea : public GSequenceLineViewAnnotatedRenderArea {
    friend class PanView;
    Q_OBJECT
public:
    PanViewRenderArea(PanView* d);
    ~PanViewRenderArea();

    PanViewRenderer* getRenderer() { return renderer; }

    virtual U2Region getAnnotationYRange(Annotation *a, int region, const AnnotationSettings *as) const;

    int getRowLineHeight() const;

protected:
    virtual void drawAll(QPaintDevice* pd);

    void resizeEvent(QResizeEvent *e);

private:
    bool isSequenceCharsVisible() const;

    PanView* getPanView() const {return static_cast<PanView*>(view);}

    PanView*            panView;
    PanViewRenderer*    renderer;
};

}//namespace;

#endif
