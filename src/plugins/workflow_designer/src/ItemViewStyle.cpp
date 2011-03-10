#include <qmath.h>
#include <QtGui/QBitmap>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QGraphicsView>
#include <QtGui/QRadialGradient>
#include <QtGui/QTextDocument>
#include <QtGui/QColorDialog>
#include <QtGui/QFontDialog>
#include <QtGui/QFontDialog>
#include <QtXml/QDomDocument>

#include <U2Core/Log.h>
#include <U2Lang/ActorModel.h>
#include <U2Lang/WorkflowSettings.h>
#include <U2Core/QVariantUtils.h>

#include "WorkflowViewItems.h"
#include "WorkflowViewController.h"
#include "ItemViewStyle.h"

namespace U2 {

const StyleId ItemStyles::SIMPLE = "simple";
const StyleId ItemStyles::EXTENDED = "ext";

#define BGC QString("-bgc")
#define FONT QString("-font")

ItemViewStyle::ItemViewStyle(const QString& id) : active(false), defFont(WorkflowSettings::defaultFont()), id(id) {
    bgColorAction = new QAction(tr("Background color"), this);
    connect(bgColorAction, SIGNAL(triggered()), SLOT(selectBGColor()));

    fontAction = new QAction(tr("Font"), this);
    connect(fontAction, SIGNAL(triggered()), SLOT(selectFont()));
}

void ItemViewStyle::selectBGColor() {
    QColor res = QColorDialog::getColor(bgColor, owner->scene()->views().first());
    if (res.isValid()) {
        bgColor = res;
        WorkflowScene* sc = qobject_cast<WorkflowScene*>(owner->scene());
        if(sc != NULL) {
            sc->setModified(true);
        }
    }
}

void ItemViewStyle::selectFont() {
    bool ok;
    QFont res = QFontDialog::getFont(&ok, defFont, owner->scene()->views().first());
    if (ok) {
        defFont = res;
        WorkflowScene* sc = qobject_cast<WorkflowScene*>(owner->scene());
        if(sc != NULL) {
            sc->setModified(true);
        }
    }
}

void ItemViewStyle::saveState(QDomElement& el) const {
    if (bgColor != defaultColor())
        el.setAttribute(id+BGC, QVariantUtils::var2String(bgColor));
    if (defFont != QFont())
        el.setAttribute(id+FONT, defFont.toString());
}

void ItemViewStyle::loadState(QDomElement& el) {
    if (el.hasAttribute(id+BGC)) {
        QColor bgc = QVariantUtils::String2Var(el.attribute(id+BGC)).value<QColor>();
        if (bgc.isValid()) {
            bgColor = bgc;
        }
    }
    if (el.hasAttribute(id+FONT)) {
        defFont.fromString(el.attribute(id+FONT));
    }
}

SimpleProcStyle::SimpleProcStyle(WorkflowProcessItem* pit) : ItemViewStyle(ItemStyles::SIMPLE) {
    owner = (pit);
    owner->connect(owner->getProcess(), SIGNAL(si_labelChanged()), SLOT(sl_update()));
    bgColor = defaultColor();
}

QColor SimpleProcStyle::defaultColor() const {
    /*QColor ret(Qt::darkCyan);
    ret.setAlpha(200);*/
    QColor ret = WorkflowSettings::getBGColor();
    return ret;
}

QRectF SimpleProcStyle::boundingRect(void) const {
    // extra space added for clean antialiased painting
    return QRectF(-R-2, -R-2, R*2+4, R*2+4);
}

QPainterPath SimpleProcStyle::shape () const {
    QPainterPath contour;
    contour.addEllipse(QPointF(0,0), R, R);
    return contour;
}

void SimpleProcStyle::paint(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    //painter->fillRect(boundingRect(), QBrush(Qt::magenta, Qt::Dense6Pattern));
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath contour;
    contour.addEllipse(QPointF(0,0), R, R);
    if (owner->isSelected()) {
        QPen pen;
        pen.setWidthF(2);
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
    }

    QRadialGradient rg(R/2, -R/2, R*2);
    rg.setColorAt(1, bgColor);
    rg.setColorAt(0, QColor(Qt::white));
    QBrush procBrush(rg);
    painter->drawPath(contour);
    painter->fillPath(contour, procBrush);
    
    painter->save();
    QTextDocument d;
    d.setDefaultFont(defFont);
    d.setHtml("<center>" + Qt::escape(owner->getProcess()->getLabel()) + "</center>");
    d.setTextWidth(R*2);
    //d.setDefaultTextOption(QTextOption(Qt::AlignHCenter));
    painter->translate(-d.size().width()/2, -d.size().height()/2);
    //painter->translate(-R, -R);
    d.drawContents(painter, QRectF(0,0,2*R,2*R));
    painter->restore();
}

//QPainterPath shape () const;

#define MARGIN 5

ExtendedProcStyle::ExtendedProcStyle(WorkflowProcessItem* pit) : ItemViewStyle(ItemStyles::EXTENDED),
autoResize(true), resizing(NoResize) {
    owner = (pit);
    Actor* process = pit->getProcess();

    doc = process->getDescription();
    if (doc) {
        owner->connect(doc, SIGNAL(contentsChanged()), SLOT(sl_update()));
    } else {
        doc = new QTextDocument(pit);
        doc->setHtml(QString("<center><b>%1</b></center><hr>%2<br>aLSKDJALSK LASDJ LASKD LASJD ALSKDJ XCKLJSLC Jas dlkjsdf sdlkjsdlfj sdlkfjlsdkfjs dlkfjsdlkfjsld flsdkjflsd kfjlsdkfj lsdkfjlsd flskfjsldkfjsldf jsdlkfjsdlkfjsdlfkjsdlfj")
            .arg(process->getLabel()).arg(process->getProto()->getDocumentation()));
    }
    owner->connect(fontAction, SIGNAL(triggered()), SLOT(sl_update()));
    refresh();

    resizeModeAction = new QAction(tr("Auto-resize to text"), this);
    resizeModeAction->setCheckable(true);
    resizeModeAction->setChecked(autoResize);
    connect(resizeModeAction, SIGNAL(toggled(bool)), SLOT(setAutoResizeEnabled(bool)));

    bgColor = defaultColor();
    snap2GridFlag = WorkflowSettings::snap2Grid();
}

QColor ExtendedProcStyle::defaultColor() const {
    return WorkflowSettings::getBGColor();
}

#define MINW 2*R
#define MAXW 6*R

void ExtendedProcStyle::refresh() {
    doc->setDefaultFont(defFont);
    if (autoResize) {
        bool snap2grid = WorkflowSettings::snap2Grid();
        qreal w,h;
        int cycle = 0;
        do {
            QSizeF docFrame = doc->size();
            w = docFrame.width() + MARGIN*2;
            if (snap2grid) {
                w = roundUp(w, GRID_STEP);
                doc->setTextWidth(w - MARGIN*2);
                docFrame = doc->size();
            }
            h = qMax(2*R, docFrame.height()) + MARGIN*2;
            if (snap2grid) {
                h = roundUp(h, GRID_STEP);
            }
            //printf("ideal=%f, actual=%f\n",doc->idealWidth(),w);
            
            // try to improve docFrame proportions
            if (++cycle > 2) {
                break;
            }
            if ((h/w < 0.6 && w > (MINW+MAXW)/2)  //doc is disproportionately wide
                || (h/w > 1.6 && w < MAXW)        //doc is disproportionately long and can be widen
                || (w < MINW || w > MAXW)) {      //width is out of bounds
                doc->setTextWidth(qBound(MINW, (qreal)(h/1.6), MAXW-MARGIN*2));            
            }
        } while (true);

        bounds = QRectF(-R, -R, w, h);
    } else {
        //bounds.setSize(bounds.size().expandedTo(doc->size() + QSizeF(MARGIN*2,MARGIN*2)));
    }
}

QPainterPath ExtendedProcStyle::shape () const {
    QPainterPath contour;
    contour.addRoundedRect(bounds, MARGIN, MARGIN);
    return contour;
}


void ExtendedProcStyle::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *)
{
    if (owner->isSelected()) {
        ((QStyleOptionGraphicsItem*)option)->state |= QStyle::State_Selected;
    }
    bgColor.setAlpha(64);
    QRectF tb = boundingRect();
    painter->fillRect(tb, QBrush(bgColor));

    painter->save();
    painter->translate(-R + MARGIN, -R + MARGIN);
    doc->drawContents(painter, QRectF(QPointF(),bounds.size() - QSizeF(MARGIN*2,MARGIN*2)));
    painter->restore();

    
    painter->setRenderHint(QPainter::Antialiasing);

    if (doc->pageCount() > 1) {
        QPointF tp = bounds.bottomRight();

        // draw a page corner
        //QPen pen;
        //pen.setWidthF(1.2);
        //painter->setPen(pen);
        //qreal len = 6;
        //painter->drawLine(tp.x() - len, tp.y() - len, tp.x(), tp.y() - len);
        //painter->drawLine(tp.x() - len, tp.y() - len, tp.x() - len, tp.y());

        // draw 3 dots at corner
        QPointF dt(tp.x() - 7, tp.y() - 5);
        QPainterPath dot; dot.addEllipse(dt, 1.2,1.2); 
        dot.addEllipse(dt - QPointF(4,0), 1.2,1.2); 
        dot.addEllipse(dt - QPointF(8,0), 1.2,1.2);
        painter->fillPath(dot, QBrush(QColor(0x33, 0x33, 0x33)));
    }

    QPen pen;
    pen.setWidthF(1.3);
    if (owner->isSelected()) {
        pen.setStyle(Qt::DashLine);
    }
    painter->setPen(pen);
    painter->drawRoundedRect(tb, MARGIN, MARGIN);
}

#define RESIZE_AREA 4
bool ExtendedProcStyle::sceneEventFilter( QGraphicsItem * watched, QEvent * event )
{
    assert(watched == owner);
    Q_UNUSED(watched);

    bool ret = false;

    switch (event->type()) {
    case QEvent::GraphicsSceneHoverEnter:
    case QEvent::GraphicsSceneHoverMove:
        {
            snap2GridFlag = WorkflowSettings::snap2Grid();
            resizing = NoResize;
            QGraphicsSceneHoverEvent* he = (static_cast<QGraphicsSceneHoverEvent *>(event));
            QPointF p = he->pos();
            qreal dx = qAbs(bounds.right() - p.x());
            qreal dy = qAbs(bounds.bottom() - p.y());
            if (dx < RESIZE_AREA) {
                resizing |= RightResize;
            }
            if(dx > (bounds.width() - RESIZE_AREA)) {
                resizing |= LeftResize;
            }
            if (dy < RESIZE_AREA) {
                resizing |= BottomResize;
            }
            if(dy > (bounds.height() - RESIZE_AREA)) {
                resizing |= TopResize;
            }

            switch (resizing)
            {
            case NoResize:
                owner->unsetCursor();
                break;
            case RightResize:
            case LeftResize:
                owner->setCursor(Qt::SizeHorCursor);
                break;
            case BottomResize:
            case TopResize:
                owner->setCursor(Qt::SizeVerCursor);
                break;
            case RBResize:
            case LTResize:
                owner->setCursor(Qt::SizeFDiagCursor);
                break;
            case LBResize:
            case RTResize:
                owner->setCursor(Qt::SizeBDiagCursor);
                break;
            }
            ret = (resizing != NoResize);
        }
        break;
    case QEvent::GraphicsSceneHoverLeave:
    case QEvent::GraphicsSceneMouseRelease:
        if (resizing) 
        {
            owner->unsetCursor();
            WorkflowSettings::setSnap2Grid(snap2GridFlag);
        }
        resizing = NoResize;
        break;
    case QEvent::GraphicsSceneMouseMove:
        if (resizing) {
            QGraphicsSceneMouseEvent* me = (static_cast<QGraphicsSceneMouseEvent *>(event));
            WorkflowSettings::setSnap2Grid(false);
            QPointF newPos;
            if ((me->buttons() & Qt::LeftButton)) {
                ret = true;
                QRectF b2 = bounds;
                QPointF p = me->pos();
                QPointF p2 = p - me->lastPos();
                
                if (resizing & RightResize 
                    &&  // border is either "pulled" or "pushed" by mouse pointer
                        // in the latter case pointer should be close to the border
                        ((p2.x() < 0 && b2.right() > p.x()) 
                        || (p2.x() > 0 && b2.right() < p.x())
                        || (qAbs(b2.right() - p.x()) < RESIZE_AREA)))
                {
                    
                    qreal rb = b2.right() + p2.x();
                    b2.setRight(rb);
                    
                    if (b2.width() < MARGIN*2 + R) {
                        return true;
                    }
                }

                if (resizing & LeftResize 
                    &&  ((p2.x() < 0 && b2.left() > p.x()) 
                        || (p2.x() > 0 && b2.left() < p.x())
                        || (qAbs(b2.left() - p.x()) < RESIZE_AREA)))
                {
                    b2.setWidth(b2.width() - p2.x());
                    newPos = owner->scenePos();
                    newPos.setX(newPos.x() - (b2.width() - bounds.width()));

                    if (b2.width() < MARGIN*2 + R) {
                        return true;
                    }

                    setFixedBounds(b2);
                    owner->setPos(newPos);
                }

                if(resizing & TopResize &&
                    ((p2.y() < 0 && b2.top() > p.y()) 
                    || (p2.y() > 0 && b2.top() < p.y())
                    || (qAbs(b2.top() - p.y()) < RESIZE_AREA))) {

                    b2.setHeight(b2.height() - p2.y());

                    newPos = owner->scenePos();
                    newPos.setY(newPos.y() - (b2.height() - bounds.height()));

                    qreal minHeight = R + MARGIN*2;
                    if (b2.height() < minHeight) {
                        return true;
                    }

                    setFixedBounds(b2);
                    owner->setPos(newPos);
                }

                if (resizing & BottomResize 
                    && ((p2.y() < 0 && b2.bottom() > p.y()) 
                        || (p2.y() > 0 && b2.bottom() < p.y())
                        || (qAbs(b2.bottom() - p.y()) < RESIZE_AREA)))
                {
                    b2.setBottom(b2.bottom() + p2.y());
                }
                //qreal minHeight = qMax(doc->size().height(), R) + MARGIN*2;
                qreal minHeight = R + MARGIN*2;
                if (b2.height() < minHeight) {
                    b2.setHeight(minHeight);
                }

                setFixedBounds(b2);
                
                WorkflowScene * sc = qobject_cast<WorkflowScene*>(owner->scene());
                if(sc != NULL) {
                    sc->setModified(true);
                }
            }
        }
        break;
    /*case QEvent::GraphicsSceneMousePress:
        mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseDoubleClick:
        mouseDoubleClickEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneWheel:
        wheelEvent(static_cast<QGraphicsSceneWheelEvent *>(event));
        break;
    case QEvent::KeyPress:
        keyPressEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::KeyRelease:
        keyReleaseEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::InputMethod:
        inputMethodEvent(static_cast<QInputMethodEvent *>(event));
        break;*/
    default:
        return false;
    }

    return ret;
}

void ExtendedProcStyle::setFixedBounds( const QRectF& b)
{
    doc->setPageSize(b.size() - QSizeF(MARGIN*2,MARGIN*2));
    if (bounds != b) {
        bounds = b;
        owner->prepareUpdate();
        
        foreach(WorkflowPortItem* pit, owner->getPortItems()) {
            pit->adaptOwnerShape();
        }
    }
    owner->update();
    resizeModeAction->setChecked(false);
}

void ExtendedProcStyle::setActive( bool v )
{
    /*autoResize =*/ active = v;
}

void ExtendedProcStyle::setAutoResizeEnabled(bool b) {
    autoResize = b;
    if (autoResize) {
        doc->setPageSize(QSizeF(-1,-1));
        owner->sl_update();
    }
}

QList<QAction*> ExtendedProcStyle::getContextMenuActions() const
{
    QList<QAction*> ret;
    ret << resizeModeAction << bgColorAction << fontAction;
    return ret;
}

#define ARM QString("arm")
#define BOUNDS QString("bounds")

void ExtendedProcStyle::saveState(QDomElement& el) const {
    //el.setAttribute(ARM, autoResize);
    if (!autoResize) {
        el.setAttribute(BOUNDS, QVariantUtils::var2String(bounds));
    }
    ItemViewStyle::saveState(el);
}

void ExtendedProcStyle::loadState(QDomElement& el) {
    if (el.hasAttribute(BOUNDS)) {
        QRectF b = QVariantUtils::String2Var(el.attribute(BOUNDS)).toRectF();
        if (!b.isNull()) {
            setFixedBounds(b);
        }
    }
    ItemViewStyle::loadState(el);
}


HintItem::HintItem( const QString & text, QGraphicsItem * parent) : QGraphicsTextItem(text, parent) 
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    document()->setDefaultTextOption(QTextOption(Qt::AlignCenter));
    setTextWidth(qMin(3*R, document()->idealWidth()));
    QRectF tb = boundingRect();
    setPos(-tb.width()/2, -tb.height()- 3);
    setDefaultTextColor(QColor(Qt::gray).darker());
    QFont f = font();
    f.setWeight(QFont::Light);
    setFont(f);
}

QVariant HintItem::itemChange( GraphicsItemChange change, const QVariant & value )
{
    if (change == ItemSelectedChange && value.toBool()) {
        parentItem()->setSelected(true);
        return false;
    }
    if (change == ItemPositionHasChanged) {
        parentItem()->update();
    }
    return QGraphicsItem::itemChange(change, value);
}

}//namespace
