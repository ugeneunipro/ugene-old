#include "GlassView.h"

#include <QtCore/QEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QPainter>

namespace U2 {

void GlassView::setGlass(GlassPane* glp) {
    glass = glp;
    if (glass) {
        glass->resize(viewport()->size());
    }
    //scene()->update();
    viewport()->update();
}

void GlassView::paintEvent( QPaintEvent * e)
{
    QGraphicsView::paintEvent(e);
    if (glass) {
        QPainter painter;
        painter.begin(viewport());
        //painter.setClipRect(e->rect());
        glass->paint(&painter);
    }
}

void GlassView::scrollContentsBy(int dx, int dy) {
    QGraphicsView::scrollContentsBy(dx,dy);
    if (glass) {
        viewport()->update();
    }
}

bool GlassView::viewportEvent( QEvent * e )
{
    if (glass) {

        switch (e->type())
        {
        case QEvent::Resize:
            glass->resize(((QResizeEvent*)e)->size());
        case QEvent::Paint:
            return QGraphicsView::viewportEvent(e);

        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Enter :
        case QEvent::Leave:
        case QEvent::Wheel:
        case QEvent::DragEnter:
        case QEvent::DragMove:
        case QEvent::DragLeave:
        case QEvent::Drop:
        case QEvent::HoverEnter:
        case QEvent::HoverLeave:
        case QEvent::HoverMove:

            if (glass->eventFilter(this, e)) {
                return true;    
            }
            break;
        default: break;
        }
    }
    // else 
    return QGraphicsView::viewportEvent(e);

}

}//namespace
