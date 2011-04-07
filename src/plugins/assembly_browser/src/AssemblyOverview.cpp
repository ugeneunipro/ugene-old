/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "AssemblyOverview.h"

#include <math.h>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QLabel>

#include <U2Core/Log.h>
#include <U2Core/AppContext.h>

#include "AssemblyBrowser.h"
#include "AssemblyReadsArea.h"
#include "AssemblyBrowserSettings.h"

namespace U2 {

//==============================================================================
// AssemblyOverview
//==============================================================================

AssemblyOverview::AssemblyOverview(AssemblyBrowserUi * ui_): ui(ui_), browser(ui->getWindow()), model(ui_->getModel()), 
redrawSelection(true), bgrRenderer(model, this), scribbling(false), scaleType(AssemblyBrowserSettings::getOverviewScaleType())
{
    setFixedHeight(FIXED_HEIGHT);
    connectSlots();
    initSelectionRedraw();
    //bgrRenderer.render(size());
}

void AssemblyOverview::connectSlots() {
    connect(&bgrRenderer, SIGNAL(si_rendered()), SLOT(sl_redraw()));
    connect(browser, SIGNAL(si_zoomOperationPerformed()), SLOT(sl_visibleAreaChanged()));
    connect(browser, SIGNAL(si_offsetsChanged()), SLOT(sl_redraw()));
}

void AssemblyOverview::initSelectionRedraw() {
    redrawSelection = true;
    cachedView = QPixmap(size());
}

void AssemblyOverview::drawAll() {
    if(!model->isEmpty()) {
        QImage bgr = bgrRenderer.getImage();
        if(bgr.isNull()) {
            cachedBackground = QPixmap(size());
            QPainter p(&cachedBackground);
            p.fillRect(cachedBackground.rect(), Qt::gray);
            p.drawText(cachedBackground.rect(), Qt::AlignCenter, tr("Background is rendering..."));
        } else {
            cachedBackground = QPixmap(size());
            cachedBackground = QPixmap::fromImage(bgr);
        }
        if (redrawSelection) {
            cachedView = cachedBackground;
            QPainter p(&cachedView);
            drawSelection(p);
            drawCoordLabels(p);
            redrawSelection = false;
        }
        QPainter p(this);
        p.drawPixmap(0, 0, cachedView);
    }
}

static inline bool onePixelRect(const QRect & r) {
    return r.width() <=2 && r.height() <= 15;
}

static const int CROSS_HALF_SIZE = 3;
static const QPoint CROSS_LEFT_CORNER(CROSS_HALF_SIZE, 0);
static const QPoint CROSS_RIGHT_CORNER(0, CROSS_HALF_SIZE);

void AssemblyOverview::drawSelection(QPainter & p) {
    cachedSelection = calcCurrentSelection();
    if(onePixelRect(cachedSelection)) {
        QPoint c = cachedSelection.center();
        QPen oldPen = p.pen();
        p.setPen(Qt::red);
        p.drawLine(c - CROSS_LEFT_CORNER, c + CROSS_LEFT_CORNER);
        p.drawLine(c - CROSS_RIGHT_CORNER, c + CROSS_RIGHT_CORNER);
        p.setPen(oldPen);
    } else {
        p.fillRect(cachedSelection, QColor(230, 230, 230, 180));
        p.drawRect(cachedSelection.adjusted(0, 0, -1, -1));
    }
}

static void insertSpaceSeparators(QString & str) {
    for(int i = str.length()-3; i > 0; i-=3) {
        str.insert(i, " ");
    }
}

void AssemblyOverview::drawCoordLabels(QPainter & p) {
    const static int xoffset = 4;
    const static int yoffset = 3;

    U2OpStatusImpl status;
    int modelLength = model->getModelLength(status);

    QString modelLengthText = QString::number(modelLength);
    insertSpaceSeparators(modelLengthText);

    QFont font;
    font.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
    QFontMetrics fontMetrics(font);

    QString globalRegionText = tr("1 to %1 bp").arg(modelLengthText); //TODO: custom start for custom overview
    QRect grtRect = QRect(0, 0, fontMetrics.width(globalRegionText), fontMetrics.height());
    grtRect.translate(xoffset, rect().height() - yoffset - grtRect.height());
    if(rect().contains(grtRect)) {
        p.setPen(Qt::gray);
        p.drawText(grtRect, Qt::AlignCenter, globalRegionText);
    }

    qint64 from = browser->getXOffsetInAssembly();
    qint64 to = qMin(browser->getXOffsetInAssembly() + browser->basesCanBeVisible(), model->getModelLength(status));

    QString fromText = QString::number(from + 1); // because of 1-based coords
    QString toText = QString::number(to);
    QString diff = QString::number(to - from);

    insertSpaceSeparators(fromText);
    insertSpaceSeparators(toText);
    insertSpaceSeparators(diff);

    QString selectedRegionText = tr("%1 to %2 (%3 bp)").arg(fromText, toText, diff);
    QRect srtRect = QRect(0, 0, fontMetrics.width(selectedRegionText), fontMetrics.height());
    srtRect.translate(rect().width() - srtRect.width() - xoffset, rect().height() - yoffset - grtRect.height());
    if(rect().contains(srtRect) && !srtRect.intersects(grtRect)) {
        p.setPen(Qt::gray);
        p.drawText(srtRect, /*Qt::AlignCenter, */selectedRegionText);
    }
}

qint64 AssemblyOverview::calcXAssemblyCoord(int x) {
    U2OpStatusImpl status;
    qint64 result = double(model->getModelLength(status)) / width() * x + 0.5;
    return result;
}

qint64 AssemblyOverview::calcYAssemblyCoord(int y) {
    U2OpStatusImpl status;
    qint64 result = double(model->getModelHeight(status)) / height() * y + 0.5;
    return result;
}

QRect AssemblyOverview::calcCurrentSelection() const {
    U2OpStatusImpl status;
    int w = rect().width();
    int h = rect().height();

    int x_pix_start = double(w) / model->getModelLength(status) * browser->getXOffsetInAssembly() + 0.5;
    int y_pix_start = double(h) / model->getModelHeight(status) * browser->getYOffsetInAssembly() + 0.5;
    int pix_width = double(w) / model->getModelLength(status) * browser->basesVisible() + 0.5;
    int pix_height = double(h) / model->getModelHeight(status) * browser->rowsVisible() + 0.5;
    
    return QRect(x_pix_start, y_pix_start, pix_width, pix_height);
}

//prevents selection from crossing widget borders. 
//Tries to move selection center to pos.
void AssemblyOverview::moveSelectionToPos( QPoint pos, bool moveModel )
{
    const QRect & thisRect = rect();
    QRect newSelection(cachedSelection);
    newSelection.moveCenter(pos);
    
    int dy = 0;
    int dx = 0;

    if(!thisRect.contains(newSelection/*, true /*entirely inside*/)) {
        QRect uneeon = rect().united(newSelection);
        if(uneeon.bottom() > thisRect.height()) {
            dy = uneeon.bottom() - thisRect.height();
        } else if(uneeon.top() < 0) {
            dy = uneeon.top();
        }
        if(uneeon.right() > thisRect.right()) {
            dx = uneeon.right() - thisRect.right();
        } else if(uneeon.left() < 0) {
            dx = uneeon.left();
        }
        newSelection.translate(-dx, -dy);
    }

    U2OpStatusImpl status;
    qint64 newXoffset = 0;
    qint64 newYoffset = 0;
    if(dx) {
        newXoffset = (dx < 0) ? 0 : model->getModelLength(status) - browser->basesVisible();
        moveModel = true;
    } else {
        newXoffset = calcXAssemblyCoord(newSelection.x());
    }
    if(dy) {
        newYoffset = (dy < 0) ? 0 : model->getModelHeight(status) - browser->rowsVisible();
        moveModel = true;
    } else {
        newYoffset = calcYAssemblyCoord(newSelection.y());
    }
    
    if(moveModel) {
        browser->setOffsetsInAssembly(newXoffset, newYoffset);
    }
}

void AssemblyOverview::paintEvent(QPaintEvent * e) {
    drawAll();
    QWidget::paintEvent(e);
}

void AssemblyOverview::resizeEvent(QResizeEvent * e) {
    cachedSelection = calcCurrentSelection();
    moveSelectionToPos(cachedSelection.center(), false);
    bgrRenderer.render(size());
    sl_redraw();
    QWidget::resizeEvent(e);
}

void AssemblyOverview::mousePressEvent(QMouseEvent * me) {
    if (me->button() == Qt::LeftButton) {
        scribbling = true;
        moveSelectionToPos(me->pos());
    }

    QWidget::mousePressEvent(me);
}

void AssemblyOverview::mouseMoveEvent(QMouseEvent * me) {
    if((me->buttons() & Qt::LeftButton) && scribbling) {
        moveSelectionToPos(me->pos());
    }
    QWidget::mouseMoveEvent(me);
}

void AssemblyOverview::mouseReleaseEvent(QMouseEvent * me) {
    if(me->button() == Qt::LeftButton && scribbling) {
        scribbling = false;
    }
    QWidget::mouseReleaseEvent(me);
}

void AssemblyOverview::sl_visibleAreaChanged() {
    cachedSelection = calcCurrentSelection();
    moveSelectionToPos(cachedSelection.center(), false);
    sl_redraw();
}

void AssemblyOverview::sl_redraw() {
    initSelectionRedraw();
    update();
}

void AssemblyOverview::setScaleType(AssemblyBrowserSettings::OverviewScaleType t) {
    AssemblyBrowserSettings::setOverviewScaleType(t);
    if(scaleType != t) {
        scaleType = t;
        bgrRenderer.render(size());
    }
}

AssemblyBrowserSettings::OverviewScaleType AssemblyOverview::getScaleType() const {
    return scaleType;
}

//==============================================================================
// AssemblyOverviewRenderTask
//==============================================================================

AssemblyOverviewRenderTask::AssemblyOverviewRenderTask(QSharedPointer<AssemblyModel> model_, QSize imageSize, 
                                                       AssemblyBrowserSettings::OverviewScaleType scT) :
Task(tr("Assembly overview renderer"), TaskFlag_None), model(model_), result(imageSize, QImage::Format_ARGB32_Premultiplied), scaleType(scT) {
    tpm = Progress_Manual;
}

void AssemblyOverviewRenderTask::run() {
    QPainter p(&result);
    p.fillRect(result.rect(), Qt::white);

    U2OpStatusImpl status;
    qint64 alignmentLen = model->getModelLength(status);
    if(status.hasError()) {
        stateInfo.setError(status.getError());
        return;
    }

    qint64 widgetWidth = result.width();
    qint64 widgetHeight = result.height();

    //FIXME can be zero
    double lettersPerXPixel = double(alignmentLen) / widgetWidth;

    QVector<quint64> readsPerXPixels(widgetWidth);
    qint64 maxReadsPerXPixels = 0;
    qint64 start = 0;

    for(int i = 0 ; i < widgetWidth; ++i) {
        if(stateInfo.cancelFlag) {
            return;
        }
        stateInfo.progress = double(i) / widgetWidth * 100.;
        qint64 readsPerXPixel = model->countReadsInAssembly(0, U2Region(start, qRound64(lettersPerXPixel)), status);
        if(status.hasError()) {
            stateInfo.setError(status.getError());
            return;
        }
        readsPerXPixels[i] = readsPerXPixel;
        start = lettersPerXPixel * i;
        if(maxReadsPerXPixels < readsPerXPixel) {
            maxReadsPerXPixels = readsPerXPixel;
        }
    }
    
    static double logMax = .0;
    double readsPerYPixel = .0;
    switch(scaleType) {
    case AssemblyBrowserSettings::Scale_Linear:
        readsPerYPixel = double(maxReadsPerXPixels) / widgetHeight; 
        break;
    case AssemblyBrowserSettings::Scale_Logarithmic:
        logMax = log((double)maxReadsPerXPixels);
        readsPerYPixel = double(logMax) / widgetHeight; 
        break;
    default:
        assert(false);
    }
    
    for(int i = 0 ; i < widgetWidth; ++i) {
        quint64 columnPixels = 0;
        int grayCoeff = 0;
        switch(scaleType) {
        case AssemblyBrowserSettings::Scale_Linear:
            columnPixels = qint64(double(readsPerXPixels[i]) / readsPerYPixel + 0.5);
            grayCoeff = 255 - int(double(255) / maxReadsPerXPixels * readsPerXPixels[i] + 0.5);
            break;
        case AssemblyBrowserSettings::Scale_Logarithmic:
            columnPixels = qint64(double(log((double)readsPerXPixels[i])) / readsPerYPixel + 0.5);
            grayCoeff = 255 - int(double(255) / logMax * log((double)readsPerXPixels[i]) + 0.5);
            break;
        }
        
        p.setPen(QColor(grayCoeff, grayCoeff, grayCoeff));
        p.drawLine(i, 0, i, columnPixels);
    }
    p.setPen(Qt::gray);
    p.drawRect(result.rect().adjusted(0,0,-1,-1));
}

//==============================================================================
// BackgroundRenderer
//==============================================================================

BackgroundRenderer::BackgroundRenderer(QSharedPointer<AssemblyModel> model_, AssemblyOverview * p) :
renderTask(0), model(model_), redrawRunning(false), redrawNeeded(true), parent(p)
{
}

void BackgroundRenderer::render(const QSize & size_)  {
    size = size_;
    if(!model->isEmpty()) {
        if(redrawRunning) {
            assert(renderTask);
            redrawNeeded = true;
            return;
        }
        redrawRunning = true;
        redrawNeeded = false;
        renderTask = new AssemblyOverviewRenderTask(model, size, parent->getScaleType());
        connect(renderTask, SIGNAL(si_stateChanged()), SLOT(sl_redrawFinished()));
        AppContext::getTaskScheduler()->registerTopLevelTask(renderTask);
    }
}

QImage BackgroundRenderer::getImage() const {
    if(redrawRunning) {
        return QImage();
    }
    assert(!renderTask);
    return result;
}

void BackgroundRenderer::sl_redrawFinished() {
    assert(renderTask == sender());
    if(Task::State_Finished != renderTask->getState()) {
        return;
    }
    assert(redrawRunning);
    redrawRunning = false;
    if(redrawNeeded) {
        render(size);
        redrawRunning = true;
        redrawNeeded = false;
    } else {
        result = renderTask->getResult();
        emit(si_rendered());
        renderTask = 0;
    }
}

} //ns
