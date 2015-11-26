/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_DET_VIEW_H_
#define _U2_DET_VIEW_H_

#include <QAction>
#include <QFont>

#include <U2Core/U2Location.h>

#include "GSequenceLineViewAnnotated.h"


class QActionGroup;

namespace U2 {

class Annotation;
class DNATranslation;
class DetViewRenderArea;
class DetViewRenderer;

class U2VIEW_EXPORT DetView : public GSequenceLineViewAnnotated {
    Q_OBJECT
public:
    DetView(QWidget* p, ADVSequenceObjectContext* ctx);

    DetViewRenderArea* getDetViewRenderArea() const;

    bool hasTranslations();
    bool hasComplementaryStrand();
    bool isWrapMode();

    virtual void setStartPos(qint64 pos);
    virtual void setCenterPos(qint64 pos);

    DNATranslation* getComplementTT() const;
    DNATranslation* getAminoTT() const;

    void setShowComplement(bool t);
    void setShowTranslation(bool t);

    void setDisabledDetViewActions(bool t);

protected slots:
    virtual void sl_sequenceChanged();
    void sl_onAminoTTChanged();
    void sl_translationRowsChanged();
    void sl_showComplementToggle(bool v);
    void sl_showTranslationToggle(bool v);
    void sl_wrapSequenceToggle(bool v);
    void sl_verticalSrcollBarMoved(int position);

protected:
    virtual void pack();

    void showEvent(QShowEvent * e);
    void hideEvent(QHideEvent * e);

    void mouseMoveEvent(QMouseEvent* me);
    void mouseReleaseEvent(QMouseEvent* me);
    void wheelEvent(QWheelEvent* we);
    void resizeEvent(QResizeEvent *e);

    void updateVisibleRange();
    void updateActions();
    void updateSize();
    void updateVerticalScrollBar();

    QAction*        showComplementAction;
    QAction*        showTranslationAction;
    QAction*        wrapSequenceAction;

    GScrollBar*     verticalScrollBar;
};

class DetViewRenderArea : public GSequenceLineViewAnnotatedRenderArea {
public:
    DetViewRenderArea(DetView* d);
    ~DetViewRenderArea();

    DetViewRenderer* getRenderer() { return renderer; }

    virtual U2Region getAnnotationYRange(Annotation *a, int region, const AnnotationSettings *as) const;
    virtual double getCurrentScale() const;

    void setWrapSequence(bool v);

    qint64 coordToPos(const QPoint& p) const;

    DetView* getDetView() const;

    int getSymbolsPerLine() const;
    int getFullyVisibleLinesCount() const;
    int getVisibleSymbolsCount() const;

    void updateSize();

    bool isOnTranslationsLine(const QPoint& p) const;
    bool isPosOnAnnotationYRange(const QPoint &p, Annotation *a, int region, const AnnotationSettings *as) const;

protected:
    virtual void drawAll(QPaintDevice* pd);

private:
    DetViewRenderer* renderer;
};

} // namespace U2

#endif
