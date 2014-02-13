/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GSEQUENCE_LINE_VIEW_ANNOTATED_H_
#define _U2_GSEQUENCE_LINE_VIEW_ANNOTATED_H_

#include <U2Core/Annotation.h>
#include <U2Core/AnnotationGroup.h>
#include <U2Core/AnnotationSelection.h>
#include <U2Core/Task.h>

#include "GSequenceLineView.h"

namespace U2 {

class AnnotationTableObject;
class AnnotationSettings;
class AnnotationModification;
class ClearAnnotationsTask;

class U2VIEW_EXPORT GSequenceLineViewAnnotated : public GSequenceLineView {
    Q_OBJECT
public:
    class DrawSettings {
    public:
                DrawSettings( );

        bool    drawAnnotationNames;
        bool    drawAnnotationArrows;
        bool    drawCutSites;
    };

                                            GSequenceLineViewAnnotated( QWidget *p,
                                                ADVSequenceObjectContext *ctx );

    bool                                    isAnnotationVisible( const Annotation &a ) const;
    const DrawSettings &                    getDrawSettings( ) const;

    virtual QList<AnnotationSelectionData>  selectAnnotationByCoord( const QPoint &coord ) const;

    static QString                          prepareAnnotationText( const AnnotationData &a,
                                                const AnnotationSettings *as );

    QList<Annotation>                       findAnnotationsInRange( const U2Region &range ) const;

protected:
    void                                    mousePressEvent( QMouseEvent *e );

    virtual bool                            event( QEvent *e );
    virtual QString                         createToolTip( QHelpEvent *e );

    virtual void                            registerAnnotations( const QList<Annotation> &l );
    virtual void                            unregisterAnnotations( const QList<Annotation> &l );
    virtual void                            ensureVisible( const AnnotationData &a,
                                                int locationIdx );

protected slots:
    virtual void                            sl_onAnnotationSettingsChanged(
                                                const QStringList &changedSettings );

    void                                    sl_onAnnotationObjectAdded( AnnotationTableObject * );
    void                                    sl_onAnnotationObjectRemoved(
                                                AnnotationTableObject * );
    void                                    sl_onAnnotationsInGroupRemoved(
                                                const QList<Annotation> &,
                                                const AnnotationGroup & );
    void                                    sl_onAnnotationsAdded( const QList<Annotation> & );
    void                                    sl_onAnnotationsRemoved( const QList<Annotation> & );
    virtual void                            sl_onAnnotationsModified(
                                                const AnnotationModification &md );
    virtual void                            sl_onAnnotationSelectionChanged( AnnotationSelection *,
                                                const QList<Annotation> &added,
                                                const QList<Annotation> &removed );

private:
    void                                    connectAnnotationObject(
                                                const AnnotationTableObject *ao );

protected:
    DrawSettings    drawSettings;

    friend class ClearAnnotationsTask;
};


class U2VIEW_EXPORT GSequenceLineViewAnnotatedRenderArea : public GSequenceLineViewRenderArea {
public:
                                    GSequenceLineViewAnnotatedRenderArea(
                                        GSequenceLineViewAnnotated *d,
                                        bool annotationsCanOverlap );
                                    ~GSequenceLineViewAnnotatedRenderArea( );

    virtual U2Region                getAnnotationYRange( const Annotation &a, int region,
                                        const AnnotationSettings *as ) const = 0;
    GSequenceLineViewAnnotated *    getGSequenceLineViewAnnotated( ) const;

protected:
    virtual void                    drawAnnotations( QPainter &p );
    virtual void                    drawBoundedText( QPainter &p, const QRect &r,
                                        const QString &text ) const;
    virtual void                    drawAnnotationConnections( QPainter &p, const Annotation &a,
                                        const AnnotationSettings *as );
    virtual void                    drawAnnotationsSelection( QPainter &p );
    virtual void                    drawCutSite( QPainter &p, const QRect &r, const QColor &color,
                                        int pos, bool direct );

    bool                            isAnnotationSelectionInVisibleRange( ) const;

    enum DrawAnnotationPass {
        DrawAnnotationPass_DrawFill,
        DrawAnnotationPass_DrawBorder
    };

    void                            drawAnnotation( QPainter &p, DrawAnnotationPass pass,
                                        const Annotation &a, const QPen &borderPen,
                                        bool selected = false, const AnnotationSettings *as = NULL,
                                        U2Region y = U2Region( ) );

protected:
    bool annotationsCanOverlap;

    //af* == annotation font
    QFont *afNormal;
    QFont *afSmall;

    QFontMetrics *afmNormal;
    QFontMetrics *afmSmall;

    int afNormalCharWidth;
    int afSmallCharWidth;

    QBrush gradientMaskBrush;
};

class ClearAnnotationsTask : public Task {
public:
                                    ClearAnnotationsTask( const QList<Annotation> &     list,
                                                          AnnotationTableObject *       aobj,
                                                          GSequenceLineViewAnnotated *  view );

    void                            run( );
    Task::ReportResult              report( );

private:
    QList<Annotation>               l;
    AnnotationTableObject *         aobj;
    GSequenceLineViewAnnotated *    view;
};

} // namespace U2

#endif
