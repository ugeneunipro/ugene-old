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

#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtGui/QPainterPath>
#include <QtGui/QToolTip>

#include <U2Core/AnnotationData.h>
#include <U2Core/AnnotationModification.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/Timer.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GUIUtils.h>

#include <U2Formats/GenbankFeatures.h>
#include <U2Formats/GenbankLocationParser.h>

#include "ADVSequenceObjectContext.h"
#include "GSequenceLineViewAnnotated.h"

static const int MIN_ANNOTATION_WIDTH = 3;
static const int MIN_ANNOTATION_TEXT_WIDTH = 5;
static const int MIN_SELECTED_ANNOTATION_WIDTH = 4;
static const int MIN_WIDTH_TO_DRAW_EXTRA_FEATURES = 10;

static const int FEATURE_ARROW_HLEN = 3;
static const int FEATURE_ARROW_VLEN = 3;

static const int CUT_SITE_HALF_WIDTH = 4;
static const int CUT_SITE_HALF_HEIGHT = 2;

static const int MAX_VIRTUAL_RANGE = 10000;

namespace U2 {

GSequenceLineViewAnnotated::DrawSettings::DrawSettings( )
    : drawAnnotationNames( false ), drawAnnotationArrows( false ), drawCutSites( true )
{

}

GSequenceLineViewAnnotated::GSequenceLineViewAnnotated(QWidget* p, ADVSequenceObjectContext* ctx) 
: GSequenceLineView(p, ctx)
{
    const QSet<AnnotationTableObject*> aObjs = ctx->getAnnotationObjects(true);
    foreach ( const AnnotationTableObject *ao, aObjs ) {
        connectAnnotationObject( ao );
    }
    connect(ctx->getAnnotationsSelection(),
        SIGNAL(si_selectionChanged(AnnotationSelection*, const QList<Annotation>&, const QList<Annotation>& )),
        SLOT(sl_onAnnotationSelectionChanged(AnnotationSelection*, const QList<Annotation>&, const QList<Annotation>&)));

    connect( ctx, SIGNAL( si_annotationObjectAdded( AnnotationTableObject * ) ),
        SLOT( sl_onAnnotationObjectAdded( AnnotationTableObject * ) ) );
    connect( ctx, SIGNAL( si_annotationObjectRemoved( AnnotationTableObject * ) ),
        SLOT( sl_onAnnotationObjectRemoved( AnnotationTableObject * ) ) );
    
    connect(AppContext::getAnnotationsSettingsRegistry(),
        SIGNAL(si_annotationSettingsChanged(const QStringList&)),
        SLOT(sl_onAnnotationSettingsChanged(const QStringList&)));

}

void GSequenceLineViewAnnotated::connectAnnotationObject( const AnnotationTableObject *ao ) {
    connect( ao, SIGNAL(si_onAnnotationsAdded( const QList<Annotation> & ) ),
        SLOT( sl_onAnnotationsAdded( const QList<Annotation> & ) ) );
    connect( ao, SIGNAL( si_onAnnotationsRemoved( const QList<Annotation> & ) ),
        SLOT( sl_onAnnotationsRemoved( const QList<Annotation> & ) ) );
    connect( ao, SIGNAL( si_onAnnotationsInGroupRemoved( const QList<Annotation> &, const AnnotationGroup & ) ),
        SLOT( sl_onAnnotationsInGroupRemoved( const QList<Annotation> &, const AnnotationGroup & ) ) );
    connect( ao, SIGNAL( si_onAnnotationModified( const AnnotationModification & ) ),
        SLOT( sl_onAnnotationsModified( const AnnotationModification & ) ) );
}

void GSequenceLineViewAnnotated::sl_onAnnotationSettingsChanged(const QStringList&) {
    addUpdateFlags(GSLV_UF_AnnotationsChanged);
    update();
}

void GSequenceLineViewAnnotated::sl_onAnnotationObjectAdded( AnnotationTableObject *o ) {
    connectAnnotationObject(o);
    sl_onAnnotationsAdded(o->getAnnotations());
}

void GSequenceLineViewAnnotated::sl_onAnnotationObjectRemoved( AnnotationTableObject *o ) {
    o->disconnect(this);
    sl_onAnnotationsRemoved(o->getAnnotations());
}


void GSequenceLineViewAnnotated::sl_onAnnotationsAdded(const QList<Annotation> &l) {
    GTIMER(c2,t2,"GSequenceLineViewAnnotated::sl_onAnnotationsAdded");
    registerAnnotations(l);
    addUpdateFlags(GSLV_UF_AnnotationsChanged);
    update();
}

void GSequenceLineViewAnnotated::sl_onAnnotationsRemoved(const QList<Annotation> &l) {
    unregisterAnnotations(l);
    addUpdateFlags(GSLV_UF_AnnotationsChanged);
    update();
}

void GSequenceLineViewAnnotated::sl_onAnnotationsInGroupRemoved(const QList<Annotation> &l, const AnnotationGroup & ) {
    AnnotationTableObject *aobj = static_cast<AnnotationTableObject *>(sender());
    ClearAnnotationsTask *task = new ClearAnnotationsTask(l, aobj, this);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
}

ClearAnnotationsTask::ClearAnnotationsTask( const QList<Annotation> &_list,
    AnnotationTableObject *_aobj, GSequenceLineViewAnnotated *_view)
    : Task( "Clear annotations", TaskFlag_None ), l( _list ), aobj( _aobj ), view( _view )
{

}

void ClearAnnotationsTask::run() {
    view->unregisterAnnotations(l);
}

Task::ReportResult ClearAnnotationsTask::report() {
    view->addUpdateFlags(GSLV_UF_AnnotationsChanged);
    view->update();
    return ReportResult_Finished;
}

void GSequenceLineViewAnnotated::sl_onAnnotationSelectionChanged( AnnotationSelection* as,
    const QList<Annotation> &_added, const QList<Annotation> &_removed )
{
    const QSet<AnnotationTableObject *> aos = ctx->getAnnotationObjects( true );

    bool changed = false;
    const QList<Annotation> added = ctx->selectRelatedAnnotations( _added );
    const QList<Annotation> removed = ctx->selectRelatedAnnotations( _removed );

    if ( 1 == added.size( ) ) {
        const Annotation &a = added.first( );
        if ( aos.contains( a.getGObject( ) ) ) {
            const AnnotationSelectionData *asd = as->getAnnotationData( a );
            ensureVisible( a.getData( ), asd->locationIdx );
            changed = true;
        }
    }

    if ( !changed ) {
        foreach( const Annotation &a, added ) {
            if ( aos.contains( a.getGObject( ) ) && isAnnotationVisible( a ) ) {
                changed = true;
                break;
            }
        }
        if ( !changed ) {
            foreach( const Annotation &a, removed ) {
                if ( aos.contains( a.getGObject( ) ) && isAnnotationVisible( a ) ) {
                    changed = true;
                    break;
                }
            }
        }
    }

    if ( changed ) {
        addUpdateFlags( GSLV_UF_SelectionChanged );
        update( );
    }
}

bool GSequenceLineViewAnnotated::isAnnotationVisible( const Annotation &a) const  {
    foreach (const U2Region &r, a.getRegions()) {
        if (visibleRange.intersects(r)) {
            return true;
        }
    }
    return false;
}

const GSequenceLineViewAnnotated::DrawSettings & GSequenceLineViewAnnotated::getDrawSettings( )
    const
{
    return drawSettings;
}

QList<AnnotationSelectionData> GSequenceLineViewAnnotated::selectAnnotationByCoord(
    const QPoint &p ) const
{
    QList<AnnotationSelectionData> res;
    GSequenceLineViewAnnotatedRenderArea *ra
        = static_cast<GSequenceLineViewAnnotatedRenderArea *>( renderArea );
    AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry( );
    const qint64 pos = ra->coordToPos( p.x( ) );
    qint64 dPos = 0;
    if ( visibleRange.length > renderArea->width( ) ) {
        float scale = renderArea->getCurrentScale( );
        dPos = static_cast<qint64>( ( 1 / scale ) );
        SAFE_POINT( dPos < seqLen, "Invalid render region end position!", res );
    }
    U2Region reg( pos - dPos, 1 + 2 * dPos );
    const QSet<AnnotationTableObject *> aObjs = ctx->getAnnotationObjects( true );
    foreach ( AnnotationTableObject *ao, aObjs ) {
        foreach ( const Annotation &a, ao->getAnnotationsByRegion( reg ) ) {
            const AnnotationData aData = a.getData( );
            QVector<U2Region> location = aData.getRegions( );
            for ( int i = 0, n = location.size( ); i < n; i++ ) {
                const U2Region &l = location[i];
                if ( l.intersects( reg ) || l.endPos( ) == reg.startPos ) {
                    bool ok = true;
                    if ( l.endPos( ) == pos || pos == l.startPos ) { //now check pixel precise coords for boundaries
                        int x1 = ra->posToCoord( l.startPos, true );
                        int x2 = ra->posToCoord( l.endPos( ), true );
                        ok = p.x( ) <= x2 && p.x( ) >= x1;
                    }
                    if ( ok ) {
                        AnnotationSettings *as = asr->getAnnotationSettings( aData );
                        if ( as->visible ) {
                            U2Region ry = ra->getAnnotationYRange( a, i, as );
                            if ( ry.contains( p.y( ) ) ) {
                                res.append( AnnotationSelectionData( a, i ) );
                            }
                        }
                    }
                }
            }
        }
    }
    return res;
}

void GSequenceLineViewAnnotated::mousePressEvent( QMouseEvent *me ) {
    setFocus( );
    const QPoint p = toRenderAreaPoint( me->pos( ) );
    const Qt::KeyboardModifiers km = QApplication::keyboardModifiers( );
    const bool singleBaseSelectionMode = km.testFlag( Qt::AltModifier );
    bool annotationEvent = false; // true if mouse pressed in some annotation area
    if ( renderArea->rect( ).contains( p ) && me->button( ) == Qt::LeftButton
        && !singleBaseSelectionMode )
    {
        const Qt::KeyboardModifiers usedModifiers = me->modifiers( );
        const bool expandAnnotationSelectionToSequence = usedModifiers.testFlag( Qt::ShiftModifier );
        if ( !( usedModifiers.testFlag( Qt::ControlModifier )
            || expandAnnotationSelectionToSequence ) )
        {
            ctx->getAnnotationsSelection( )->clear( );
        }
        QList<AnnotationSelectionData> selected = selectAnnotationByCoord( p );
        annotationEvent = !selected.isEmpty( );
        if ( annotationEvent ) {
            AnnotationSelectionData *asd = &selected.first( );
            if ( selected.size( ) > 1 ) {
                AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry( );
                QMenu popup;
                foreach ( const AnnotationSelectionData &as, selected ) {
                    const AnnotationData aData = as.annotation.getData( );
                    QVector<U2Region> location = aData.getRegions( );
                    const U2Region &r = location[qMax( 0, as.locationIdx )];
                    QString text = aData.name
                        + QString( " [%1, %2]" ).arg( QString::number( r.startPos + 1 ) )
                        .arg( QString::number( r.endPos( ) ) );
                    AnnotationSettings *asettings = asr->getAnnotationSettings( aData );
                    const QIcon icon = GUIUtils::createSquareIcon( asettings->color, 10 );
                    popup.addAction( icon, text );
                }
                QAction *a = popup.exec( QCursor::pos( ) );
                if ( NULL == a ) {
                    asd = NULL;
                } else {
                    int idx = popup.actions( ).indexOf( a );
                    asd = &selected[idx];
                }
            }
            if ( NULL != asd ) { //add to annotation selection
                AnnotationSelection *asel = ctx->getAnnotationsSelection( );
                if ( asel->contains( asd->annotation, asd->locationIdx ) ) {
                    asel->removeFromSelection( asd->annotation, asd->locationIdx );
                } else {
                    asel->addToSelection( asd->annotation, asd->locationIdx );
                }
                //select region
                if ( expandAnnotationSelectionToSequence ) {
                    U2Region regionToSelect;
                    foreach ( const AnnotationSelectionData &asd, asel->getSelection( ) ) {
                        AnnotationTableObject *aobj = asd.annotation.getGObject( );
                        const QSet<AnnotationTableObject *> aObjs = ctx->getAnnotationObjects( true );
                        if ( !aObjs.contains( aobj ) ) {
                            continue;
                        }
                        U2Region aregion;
                        if ( -1 == asd.locationIdx ) {
                            aregion = U2Region::containingRegion( asd.annotation.getRegions( ) );
                        } else {
                            aregion = asd.annotation.getRegions( ).at( asd.locationIdx );
                        }
                        regionToSelect = ( 0 == regionToSelect.length ) ? aregion
                            : U2Region::containingRegion( regionToSelect, aregion );
                    }
                    ctx->getSequenceSelection( )->setRegion( regionToSelect );
                }
            }
        }
    }
    // a hint to parent class: if mouse action leads to annotation selection -> skip selection handling for mouse press
    ignoreMouseSelectionEvents = annotationEvent;
    GSequenceLineView::mousePressEvent( me );
    ignoreMouseSelectionEvents = false;
}

QString GSequenceLineViewAnnotated::prepareAnnotationText( const AnnotationData &a, const AnnotationSettings *as ) {
    if ( !as->showNameQuals || as->nameQuals.isEmpty( ) ) {
        return a.name;
    }
    QVector<U2Qualifier> qs;
    foreach ( const QString &qn, as->nameQuals ) {
        qs.clear( );
        a.findQualifiers( qn, qs );
        if ( !qs.isEmpty( ) ) {
            QString res = qs[0].value;
            return res;
        }
    }
    return a.name;
}

QList<Annotation> GSequenceLineViewAnnotated::findAnnotationsInRange(const U2Region &range) const
{
    QList<Annotation> result;
    const QSet<AnnotationTableObject *> aObjs = ctx->getAnnotationObjects( true );
    foreach ( AnnotationTableObject *ao, aObjs) {
        result << ao->getAnnotationsByRegion( range );
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
/// Renderer

GSequenceLineViewAnnotatedRenderArea::GSequenceLineViewAnnotatedRenderArea(GSequenceLineViewAnnotated* d, bool overlap) : GSequenceLineViewRenderArea(d)
{
    annotationsCanOverlap = overlap;
    afNormal = new QFont("Courier", 10);
    afSmall = new QFont("Arial", 8);

    afmNormal = new QFontMetrics(*afNormal);
    afmSmall = new QFontMetrics(*afSmall);

    afNormalCharWidth  = afmNormal->width('w');
    afSmallCharWidth   = afmSmall->width('w');

    QLinearGradient gradient(0, 0, 0, 1); //vertical
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient.setColorAt(0.00, QColor(255, 255, 255, 120));
    gradient.setColorAt(0.50, QColor(  0,   0,   0,   0));
    gradient.setColorAt(0.70, QColor(  0,   0,   0,   0));
    gradient.setColorAt(1.00, QColor(  0,   0,   0,  70));
    gradientMaskBrush = QBrush(gradient);
}

GSequenceLineViewAnnotatedRenderArea::~GSequenceLineViewAnnotatedRenderArea() {
    delete afmNormal;
    delete afNormal;
    delete afmSmall;
    delete afSmall;
}

void GSequenceLineViewAnnotatedRenderArea::drawAnnotations( QPainter& p ) {
    GTIMER( c2, t2, "GSequenceLineViewAnnotatedRenderArea::drawAnnotations" );
    ADVSequenceObjectContext *ctx = view->getSequenceContext( );

    QPen pen1( Qt::SolidLine );
    pen1.setWidth( 1 );

    foreach ( const AnnotationTableObject *ao, ctx->getAnnotationObjects( true ) ) {
        foreach ( const Annotation &a, ao->getAnnotationsByRegion( view->getVisibleRange( ) ) ) {
            AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry( );
            AnnotationSettings *as = asr->getAnnotationSettings( a.getData( ) );
            drawAnnotation( p, DrawAnnotationPass_DrawFill, a, pen1, false, as );
            drawAnnotation( p, DrawAnnotationPass_DrawBorder, a, pen1, false, as );
        }
    }
}

void static addArrowPath(QPainterPath& path, const QRect& rect, bool leftArrow) {
    if (rect.width() <= FEATURE_ARROW_HLEN || rect.height() <= 0) {
        return;
    }
    int x = leftArrow ? rect.left() : rect.right();
    int dx = leftArrow ? -FEATURE_ARROW_HLEN : FEATURE_ARROW_HLEN;
    
    QPolygon arr;
    arr << QPoint(x - dx, rect.top()    - FEATURE_ARROW_VLEN);
    arr << QPoint(x + dx, rect.top()    + rect.height() / 2);
    arr << QPoint(x - dx, rect.bottom() + FEATURE_ARROW_VLEN);
    arr << QPoint(x - dx, rect.top()    - FEATURE_ARROW_VLEN);
    QPainterPath arrowPath;
    arrowPath.addPolygon(arr);

    QPainterPath dRectPath;
    dRectPath.addRect(leftArrow ? x : x - (FEATURE_ARROW_HLEN - 1), rect.top(), FEATURE_ARROW_HLEN, rect.height());

    path = path.subtracted(dRectPath);
    path = path.united(arrowPath);
}

GSequenceLineViewAnnotated * GSequenceLineViewAnnotatedRenderArea::getGSequenceLineViewAnnotated( ) const
{
    return static_cast<GSequenceLineViewAnnotated *>( view );
}

void GSequenceLineViewAnnotatedRenderArea::drawAnnotation( QPainter &p, DrawAnnotationPass pass,
    const Annotation &a, const QPen &borderPen, bool selected, const AnnotationSettings *as,
    U2Region predefinedy )
{
    const AnnotationData aData = a.getData( );
    if ( NULL == as ) {
        AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry( );
        as = asr->getAnnotationSettings( aData );
    }
    if ( !as->visible && ( DrawAnnotationPass_DrawFill == pass || !selected ) ) {
        return;
    }
    const GSequenceLineViewAnnotated::DrawSettings &drawSettings
        = getGSequenceLineViewAnnotated( )->getDrawSettings( );
    const U2Region &vr = view->getVisibleRange( );

    QVector<U2Region> location = aData.getRegions();
    bool simple = location.size( ) == 1;
    for ( int ri = 0, ln = location.size( ); ri < ln; ri++ ) {
        const U2Region &r = location.at( ri );
        if ( !r.intersects( vr ) || predefinedy.startPos < 0 ) {
            continue;
        }
        const U2Region visibleLocation = r.intersect( vr );
        const U2Region y = predefinedy.isEmpty( ) ? getAnnotationYRange( a, ri, as ) : predefinedy;
        if ( y.startPos < 0 ) {
            continue;
        }
        const float x1f = posToCoordF( visibleLocation.startPos );
        const float x2f = posToCoordF( visibleLocation.endPos( ) );
        assert( x2f >= x1f );

        const int rw = qMax( selected ? MIN_SELECTED_ANNOTATION_WIDTH : MIN_ANNOTATION_WIDTH,
            qRound( x2f - x1f ) );
        const int x1 = qRound( x1f );

        const QRect annotationRect( x1, y.startPos, rw, y.length );
        QPainterPath rectPath;
        rectPath.addRect( x1, y.startPos, rw, y.length );
        const bool leftTrim  = visibleLocation.startPos != r.startPos;
        const bool rightTrim = visibleLocation.endPos( ) != r.endPos( );
        const bool drawArrow = aData.getStrand( ).isCompementary( ) ? !leftTrim : !rightTrim;
        if ( drawSettings.drawAnnotationArrows && drawArrow ) {
            bool isLeft = false;
            if ( 1 == ri && aData.findFirstQualifierValue( "rpt_type" ) == "inverted" ) { //temporary solution for drawing inverted repeats correct
                isLeft = true;
            } else {
                isLeft = aData.getStrand( ).isCompementary( );
            }
            addArrowPath( rectPath, annotationRect, isLeft );
        }

        if ( DrawAnnotationPass_DrawFill == pass ) {
            rectPath.setFillRule( Qt::WindingFill );
            p.fillPath( rectPath, as->color );
            p.fillPath( rectPath, gradientMaskBrush );
        } else {
            SAFE_POINT( pass == DrawAnnotationPass_DrawBorder, "Invalid annotation painter!", );
            p.setPen( borderPen );
            if ( rw > MIN_ANNOTATION_WIDTH ) {
                p.drawPath( rectPath );
                if ( drawSettings.drawAnnotationNames
                    && annotationRect.width( ) >= MIN_ANNOTATION_TEXT_WIDTH )
                {
                    const QString aText = GSequenceLineViewAnnotated::prepareAnnotationText( aData, as );
                    drawBoundedText( p, annotationRect, aText );
                }
                if ( simple && annotationRect.width( ) > MIN_WIDTH_TO_DRAW_EXTRA_FEATURES ) {
                    if ( drawSettings.drawCutSites ) {
                        const QString cutStr
                            = aData.findFirstQualifierValue( GBFeatureUtils::QUALIFIER_CUT );
                        bool hasD = false;
                        bool hasC = false;
                        int cutD = 0;
                        int cutC = 0;
                        if ( !cutStr.isEmpty( ) ) {
                            int complSplit = cutStr.indexOf( '/' );
                            if ( -1 != complSplit ) {
                                cutD = cutStr.left( complSplit ).toInt( &hasD );
                                cutC = cutStr.mid( qMin( cutStr.length( ), complSplit + 1 ) )
                                    .toInt( &hasC );
                            } else {
                                cutD = cutStr.toInt( &hasD );
                                cutC = cutD;
                                hasC = hasD;
                            }
                        }

                        if ( hasD ) {
                            const int cutPosDirect = aData.getStrand( ).isDirect( ) ? r.startPos + cutD
                                : r.startPos + cutC;
                            drawCutSite( p, annotationRect, as->color, cutPosDirect , true );
                        }
                        if ( hasC ) {
                            const int cutPosCompl = aData.getStrand( ).isDirect( ) ? r.endPos( ) - cutC
                                : r.endPos( ) - cutD;
                            drawCutSite( p, annotationRect, as->color, cutPosCompl, false );
                        }
                    }
                }
            }
            drawAnnotationConnections( p, a, as );
        }
    }
}

void GSequenceLineViewAnnotatedRenderArea::drawAnnotationsSelection( QPainter &p ) {
    ADVSequenceObjectContext *ctx = view->getSequenceContext( );

    QPen pen1( Qt::SolidLine );
    pen1.setWidth( 1 );

    QPen pen2( Qt::SolidLine );
    pen2.setWidth( 2 );

    const AnnotationSelection *annSelection = ctx->getAnnotationsSelection( );
    foreach ( const AnnotationSelectionData &asd, annSelection->getSelection( ) ) {
        AnnotationTableObject *o = asd.annotation.getGObject( );
        if ( ctx->getAnnotationObjects( true ).contains( o ) ) {
            if ( annotationsCanOverlap ) {
                drawAnnotation( p, DrawAnnotationPass_DrawFill, asd.annotation, pen1, true );
            }
            drawAnnotation( p, DrawAnnotationPass_DrawBorder, asd.annotation, pen2, true );
        }
    }
}

void GSequenceLineViewAnnotatedRenderArea::drawBoundedText( QPainter &p, const QRect &r,
    const QString &text) const
{
    if ( afSmallCharWidth > r.width( ) ) {
        return;
    }
    const QFont *font = afNormal;
    const QFontMetrics* fm = afmNormal;
    if ( fm->width( text ) > r.width( ) ) {
        font = afSmall;
        fm = afmSmall;
    }
    p.setFont( *font );

    const int len = text.length( );
    int textWidth = 0;
    int prefixLen = 0;
    do {
        int cw = fm->width( text[prefixLen] );
        if ( textWidth + cw > r.width( ) ) {
            break;
        }
        textWidth += cw;
    } while ( ++prefixLen < len );
    
    if ( 0 == prefixLen ) {
        return;
    }
    p.drawText( r, Qt::TextSingleLine | Qt::AlignCenter, text.left( prefixLen ) );
}

void GSequenceLineViewAnnotatedRenderArea::drawCutSite(QPainter& p, const QRect& rect, const QColor& color, int pos, bool direct ) {
    int xCenter = posToCoord(pos, true);

    int xLeft = xCenter - CUT_SITE_HALF_WIDTH;
    int xRight= xCenter + CUT_SITE_HALF_WIDTH;
    int yFlat = direct ? rect.top() - CUT_SITE_HALF_HEIGHT : rect.bottom() + CUT_SITE_HALF_HEIGHT;
    int yPeak = direct ? rect.top() + CUT_SITE_HALF_HEIGHT : rect.bottom() - CUT_SITE_HALF_HEIGHT;
    
    QPolygon triangle;
    triangle << QPoint(xLeft, yFlat) << QPoint(xCenter, yPeak) << QPoint(xRight,yFlat) << QPoint(xLeft,yFlat);

    QPainterPath path;
    path.addPolygon(triangle);

    p.fillPath(path,color);
    p.drawPath(path);

}

void GSequenceLineViewAnnotatedRenderArea::drawAnnotationConnections(QPainter &p,
    const Annotation &a, const AnnotationSettings *as)
{
    const AnnotationData aData = a.getData( );
    if ( aData.location->isSingleRegion( ) ) {
        return;
    }

    const GSequenceLineViewAnnotated *sequenceLineView = getGSequenceLineViewAnnotated( );
    const U2SequenceObject *seqObject = sequenceLineView->getSequenceObject( );
    SAFE_POINT( NULL != seqObject, "Invalid sequence object occured!", );
    const qint64 seqLength = seqObject->getSequenceLength( );
    U2Region sRange( 0, seqLength );
    if ( U1AnnotationUtils::isSplitted( aData.location, sRange ) ) {
        return;
    }

    const GSequenceLineViewAnnotated::DrawSettings &drawSettings
        = sequenceLineView->getDrawSettings( );
    const U2Region &visibleRange = view->getVisibleRange( );
    int dx1 = 0;
    int dx2 = 0;
    if ( drawSettings.drawAnnotationArrows ) {
        if ( aData.getStrand().isCompementary( ) ) {
            dx2 = - FEATURE_ARROW_HLEN;
        } else {
            dx1 = FEATURE_ARROW_HLEN;
        }
    }
    QVector<U2Region> location = aData.getRegions( );
    for ( int ri = 0, ln = location.size( ); ri < ln; ri++ ) {
        const U2Region &r = location.at( ri );
        if ( ri > 0 ) {
            U2Region prev = location.at( ri - 1 );
            const int prevPos = prev.endPos( );
            const int pos = r.startPos;
            const int min = qMin( prevPos, pos );
            const int max = qMax( prevPos, pos );
            if ( visibleRange.intersects( U2Region( min, max - min ) ) ) {
                int x1 = posToCoord( prevPos, true ) + dx1;
                int x2 = posToCoord( pos, true ) + dx2;
                if ( qAbs( x2 - x1 ) > 1 ) {
                    x1 = qBound( -MAX_VIRTUAL_RANGE, x1, MAX_VIRTUAL_RANGE ); //qt4.4 crashes in line clipping alg for extremely large X values
                    x2 = qBound( -MAX_VIRTUAL_RANGE, x2, MAX_VIRTUAL_RANGE );
                    const int midX = ( x1 + x2 ) / 2;
                    const U2Region pyr = getAnnotationYRange( a, ri - 1, as );
                    const U2Region yr = getAnnotationYRange( a, ri, as );
                    const int y1 = pyr.startPos;
                    const int dy1 = pyr.length / 2;
                    const int y2 = yr.startPos;
                    const int dy2 = yr.length / 2;
                    const int midY = qMin( y1, y2 );
                    p.drawLine( x1, y1 + dy1, midX, midY );
                    p.drawLine( midX, midY, x2, y2 + dy2 );
                }
            }
        }
    }
}

void GSequenceLineViewAnnotated::registerAnnotations( const QList<Annotation> & ) {

}
void GSequenceLineViewAnnotated::unregisterAnnotations( const QList<Annotation> & ) {

}

void GSequenceLineViewAnnotated::ensureVisible( const AnnotationData &a, int locationIdx ) {
    QVector<U2Region> location = a.getRegions( );
    SAFE_POINT( locationIdx < location.size( ), "Invalid annotation location on the widget!", );
    if ( -1 == locationIdx ) {
        foreach (  const U2Region &r, location ) {
            if ( visibleRange.intersects( r ) ) {
                return;
            }
        }
    }
    const U2Region &region = location[qMax( 0, locationIdx )];
    if ( !visibleRange.intersects( region ) ) {
        const qint64 pos = a.getStrand( ).isCompementary( ) ? region.endPos( ) : region.startPos;
        setCenterPos( qBound( qint64( 0 ), pos, seqLen - 1 ) );
    }
}

bool GSequenceLineViewAnnotated::event(QEvent *e) {
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent* he = static_cast<QHelpEvent*>(e);
        QString tip = createToolTip(he);
        if (!tip.isEmpty()) {
            QToolTip::showText(he->globalPos(), tip);
        }
        return true;
    }
    return GSequenceLineView::event(e);
}

QString GSequenceLineViewAnnotated::createToolTip( QHelpEvent *e ) {
    const int ROWS_LIMIT = 25;
    QList<AnnotationSelectionData> la = selectAnnotationByCoord( e->pos( ) );
    QList<AnnotationData> annotationList;
    if ( la.isEmpty( ) ) {
        return QString( );
    } else {
        // fetch annotation data before further processing in order to improve performance
        foreach ( const AnnotationSelectionData &ad, la ) {
            annotationList << ad.annotation.getData( );
        }
    }
    QString tip = "<table>";
    int rows = 0;
    if ( annotationList.size( ) > 1 ) {
        foreach ( const AnnotationData &ad, annotationList ) {
            rows += ad.qualifiers.size( ) + 1;
        }
    }

    const bool skipDetails = ( rows > ROWS_LIMIT );
    rows = 0;
    foreach ( const AnnotationData &ad, annotationList ) {
        if ( ++rows > ROWS_LIMIT ) {
            break;
        }
        AnnotationSettingsRegistry *registry = AppContext::getAnnotationsSettingsRegistry( );
        const QColor acl = registry->getAnnotationSettings( ad.name )->color;
        tip += "<tr><td bgcolor=" + acl.name( ) + " bordercolor=black width=15></td><td><big>"
            + ad.name + "</big></td></tr>";
        
        if ( skipDetails ) {
            tip += "<tr><td/><td>...</td>";
            rows++;
        } else {
            tip += "<tr><td></td><td><b>Location</b> = "
                + Genbank::LocationParser::buildLocationString( &ad ) + "</td></tr>";
            tip += "<tr><td/><td>";
            tip += Annotation::getQualifiersTip( ad, ROWS_LIMIT - rows, getSequenceObject( ),
                ctx->getComplementTT( ), ctx->getAminoTT( ) );
            tip += "</td></tr>";
            rows += ad.qualifiers.size( );
        }
    }
    tip += "</table>";
    if ( rows > ROWS_LIMIT ) {
        tip += "<hr> <div align=center>" + tr( "etc ..." ) + "</div>";
    }
    return tip;
}

void GSequenceLineViewAnnotated::sl_onAnnotationsModified( const AnnotationModification& md )
{
    if (md.type == AnnotationModification_LocationChanged || md.type == AnnotationModification_NameChanged ) {
        addUpdateFlags(GSLV_UF_AnnotationsChanged);
        update();
    }
}
bool GSequenceLineViewAnnotatedRenderArea::isAnnotationSelectionInVisibleRange() const {
    const QSet<AnnotationTableObject*> aos = view->getSequenceContext()->getAnnotationObjects(true);
    AnnotationSelection* as = view->getSequenceContext()->getAnnotationsSelection();
    foreach(const AnnotationSelectionData& asd, as->getSelection()) {
        if (!aos.contains(asd.annotation.getGObject())) {
            continue;
        }
        if (getGSequenceLineViewAnnotated()->isAnnotationVisible(asd.annotation)) {
            return true;
        }
    }
    return false;
}


} // namespace
