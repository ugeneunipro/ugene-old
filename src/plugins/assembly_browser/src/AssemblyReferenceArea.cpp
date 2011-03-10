#include "AssemblyReferenceArea.h"

#include <QtGui/QPainter>
#include <U2Core/Timer.h>
#include "AssemblyBrowser.h"

namespace U2 {

AssemblyReferenceArea::AssemblyReferenceArea(AssemblyBrowserUi * ui_) : 
ui(ui_), window(ui_->getWindow()), model(ui_->getModel())
{
    setFixedHeight(FIXED_HEIGHT);
    connectSlots();
    sl_redraw();
}

void AssemblyReferenceArea::connectSlots() {
    connect(window, SIGNAL(si_zoomOperationPerformed()), SLOT(sl_redraw()));
    connect(window, SIGNAL(si_offsetsChanged()), SLOT(sl_redraw()));
}

void AssemblyReferenceArea::drawAll() {
    if(!model->isEmpty() && model->hasReference()) {
        if (redraw) {
            cachedView.fill(Qt::transparent);
            QPainter p(&cachedView);
            redraw = false;
            drawReference(p);
        }
        QPainter p(this);
        p.drawPixmap(0, 0, cachedView);
    }
}

void AssemblyReferenceArea::drawReference(QPainter & p) {
    GTIMER(c1, t1, "AssemblyReferenceArea::drawReference");

    if(window->areCellsVisible()) {
        p.fillRect(rect(), Qt::white);

        qint64 xOffsetInAss = window->getXOffsetInAssembly();
        U2Region visibleRegion(xOffsetInAss,  window->basesCanBeVisible());

        U2OpStatusImpl status;
        QByteArray visibleSequence = model->getReferenceRegion(visibleRegion, status);
        if(checkAndLogError(status)) {
            return;
        }

        int letterWidth = window->getCellWidth();
        int letterHeight = FIXED_HEIGHT;

        int x_pix_start = 0;
        const int y_pix_start = 0;

        bool text = window->areLettersVisible();
        QFont f = window->getFont();
        if(text) {
            int pointSize = qMin(letterWidth, letterHeight) / 2;
            if(pointSize) {
                f.setPointSize(pointSize); 
            } else {
                text = false;
            }
        }
        QVector<QImage> cells = cellRenderer.render(QSize(letterWidth, letterHeight), text, f);

        for(int i = 0; i < visibleSequence.length(); ++i, x_pix_start+=letterWidth) {
            QRect r(x_pix_start, y_pix_start, letterWidth, letterHeight);
            char c = visibleSequence.at(i);
            p.drawImage(r, cells[c]);
        }
    }
}

void AssemblyReferenceArea::paintEvent(QPaintEvent * e) {
    drawAll();
    QWidget::paintEvent(e);
}

void AssemblyReferenceArea::resizeEvent(QResizeEvent * e) {
    sl_redraw();
    QWidget::resizeEvent(e);
}

void AssemblyReferenceArea::sl_redraw() {
    cachedView = QPixmap(size());
    redraw = true;
    update();
}

} //ns
