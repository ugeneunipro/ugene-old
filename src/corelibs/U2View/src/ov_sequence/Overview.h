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

#ifndef _U2_OVERVIEW_H_
#define _U2_OVERVIEW_H_

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QToolButton>
#else
#include <QtWidgets/QToolButton>
#endif

#include <U2Core/Annotation.h>
#include <U2Core/AnnotationGroup.h>

#include "GSequenceLineView.h"
#include "ADVSequenceObjectContext.h"

namespace U2 {

class PanView;
class DetView;
class OverviewRenderArea;
class AnnotationModification;
class AnnotationTableObject;
class ADVSingleSequenceWidget;


class U2VIEW_EXPORT Overview : public GSequenceLineView {
    Q_OBJECT
public:
    Overview(ADVSingleSequenceWidget* p, ADVSequenceObjectContext* ctx);

protected slots:
    void sl_visibleRangeChanged();
    void sl_graphActionTriggered();
    void sl_annotationsAdded(const QList<Annotation *> &a);
    void sl_annotationsRemoved(const QList<Annotation *> &a);
    void sl_onAnnotationsInGroupRemoved(const QList<Annotation *> &, AnnotationGroup *);
    void sl_annotationModified(const AnnotationModification &md);
    void sl_onAnnotationSettingsChanged(const QStringList &changedSettings);
    void sl_annotationObjectAdded(AnnotationTableObject *obj);
    void sl_annotationObjectRemoved(AnnotationTableObject *obj);
    void sl_sequenceChanged();
protected:
    void pack();
    virtual bool event(QEvent* e);
    void mousePressEvent(QMouseEvent *me);
    void mouseMoveEvent(QMouseEvent* me);
    void mouseDoubleClickEvent(QMouseEvent* me);
    void mouseReleaseEvent(QMouseEvent* me);
    void wheelEvent(QWheelEvent* we);

    QString createToolTip(QHelpEvent* he);
    PanView *getPan() const;
    DetView *getDet() const;

    bool        panSliderClicked;
    bool        detSliderClicked;
    bool        panSliderMovedRight;
    bool        panSliderMovedLeft;

    qint64         offset;
private:
    PanView*        panView;
    DetView*        detView;
    QPoint          mousePosToSlider;
    ADVSingleSequenceWidget* seqWidget;


friend class OverviewRenderArea;
};

class OverviewRenderArea : public GSequenceLineViewRenderArea {
    Q_OBJECT
public:
    OverviewRenderArea(Overview* p);

    const QRectF getPanSlider() const;
    const QRectF getDetSlider() const;

    int getAnnotationDensity(int pos) const;

    bool    showGraph;
protected:
    void drawAll(QPaintDevice* pd);
private:
    void drawRuler(QPainter& p);
    void drawSelection(QPainter& p);
    void drawSlider(QPainter& p, QRectF rect, QColor col);
    void drawArrow(QPainter& p, QRectF rect, QColor col);
    void setAnnotationsOnPos();
    void drawGraph(QPainter& p);
    QColor getUnitColor(int count);

    QRectF          panSlider;
    QRectF          detSlider;
    QBrush          gradientMaskBrush;
    QVector<int>    annotationsOnPos;
};

}//namespace

#endif
