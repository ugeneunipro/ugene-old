#include "MSAEditorOffsetsView.h"
#include "MSAEditorBaseOffsetsCache.h"
#include "MSAEditorSequenceArea.h"
#include "MSAEditor.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/MAlignmentObject.h>

#include <QtGui/QPainter>
#include <math.h>

namespace U2 {

#define SETTINGS_ROOT QString("msaeditor/")

#define SETTINGS_SHOW_OFFSETS "show_offsets"
#define MIN_LENGTH_TO_SHOW_OFFSETS 3000

MSAEditorOffsetsViewController::MSAEditorOffsetsViewController(QObject* p, MSAEditor* ed, MSAEditorSequenceArea* sa)
: QObject(p)
{
    seqArea = sa;
    editor = ed;
    MAlignmentObject *mobj = editor->getMSAObject();
    assert(mobj);
    MSAEditorBaseOffsetCache* cache = new MSAEditorBaseOffsetCache(this, mobj);
    lw = new MSAEditorOffsetsViewWidget(ed, seqArea, cache, true);
    rw = new MSAEditorOffsetsViewWidget(ed, seqArea, cache, false);

    connect(seqArea, SIGNAL(si_startChanged(const QPoint&,const QPoint&)), SLOT(sl_startChanged(const QPoint&,const QPoint&)));
    connect(editor, SIGNAL(si_fontChanged(const QFont&)), SLOT(sl_fontChanged()));
    
    connect(mobj, SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)), 
        SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
    
    sa->installEventFilter(this);
    
    Settings* s = AppContext::getSettings();
    bool showOffsets = s->getValue(SETTINGS_ROOT + SETTINGS_SHOW_OFFSETS, true).toBool();

    viewAction = new QAction(tr("Show offsets"), this);
    viewAction->setCheckable(true);
    viewAction->setChecked(showOffsets);
    connect(viewAction, SIGNAL(triggered(bool)), SLOT(sl_showOffsets(bool)));

    updateOffsets();
}

bool MSAEditorOffsetsViewController::eventFilter(QObject* o, QEvent* e) {
    if (o == seqArea) {
        if (e->type() == QEvent::Resize || e->type() == QEvent::Show) {
            updateOffsets();
        } 
    }
    return false;
}

void MSAEditorOffsetsViewController::sl_showOffsets(bool show) {
    updateOffsets();
    Settings* s = AppContext::getSettings();
    s->setValue(SETTINGS_ROOT + SETTINGS_SHOW_OFFSETS, show);
}

void MSAEditorOffsetsViewController::updateOffsets() {
    bool enableOffsets = editor->getAlignmentLen() < MIN_LENGTH_TO_SHOW_OFFSETS;
    viewAction->setEnabled(enableOffsets);
   
    if (lw->parentWidget()!=NULL) {
        bool vis = viewAction->isChecked() & enableOffsets;
        lw->setVisible(vis);
        rw->setVisible(vis);
    }

    lw->updateView();
    rw->updateView();
}

MSAEditorOffsetsViewWidget::MSAEditorOffsetsViewWidget(MSAEditor *ed, MSAEditorSequenceArea* sa, MSAEditorBaseOffsetCache* c, bool sp) 
:seqArea(sa), editor(ed), cache(c), showStartPos(sp), completeRedraw(true)
{
    cachedView = new QPixmap();
}

MSAEditorOffsetsViewWidget::~MSAEditorOffsetsViewWidget() {
    delete cachedView;
}

#define OFFS_WIDGET_BORDER 3
void MSAEditorOffsetsViewWidget::updateView() {
    int aliLen = cache->getMSAObject()->getMAlignment().getLength();
    QFont f = getOffsetsFont();
    QFontMetrics fm(f);
    int aliLenStrLen = int(log10((double)aliLen)) + 1; 
    int w = OFFS_WIDGET_BORDER + fm.width('X') * aliLenStrLen + OFFS_WIDGET_BORDER;
    w += (showStartPos ? fm.width('[') : fm.width(']'));
    setFixedWidth(w);
    completeRedraw = true;
    update();
}

void MSAEditorOffsetsViewWidget::paintEvent(QPaintEvent*) {
    assert(isVisible());
    QSize s = size();
    if (s != cachedView->size()) {
        delete cachedView;
        cachedView = new QPixmap(s);
        completeRedraw = true;
    }
    if (completeRedraw) {
        QPainter pCached(cachedView);
        drawAll(pCached);
        completeRedraw = false;
    }
    QPainter p(this);
    p.drawPixmap(0, 0, *cachedView);
}

QFont MSAEditorOffsetsViewWidget::getOffsetsFont() {
    QFont f = editor->getFont();
    f.setPointSize(qMax(f.pointSize()-1, 6));
    return f;
}

void MSAEditorOffsetsViewWidget::drawAll(QPainter& p) {
    QLinearGradient gradient(0, 0, width(), 0);
    QColor lg(0xDA, 0xDA, 0xDA);
    QColor dg(0x4A, 0x4A, 0x4A);
    gradient.setColorAt(0.00, lg);
    gradient.setColorAt(0.25, Qt::white);
    gradient.setColorAt(0.75, Qt::white); 
    gradient.setColorAt(1.00, lg); 
    p.fillRect(rect(), QBrush(gradient));

    int w = width();
    
    QFont f = getOffsetsFont();
    QFontMetrics fm(f);
    p.setFont(f);

    int nSeqVisible = seqArea->getNumVisibleSequences(true);
    int startSeq = seqArea->getFirstVisibleSequence();
    int aliLen = cache->getMSAObject()->getMAlignment().getLength();
    int lbw = fm.width('[');
    int rbw = fm.width(']');
    int pos = showStartPos ? seqArea->getFirstVisibleBase() : seqArea->getLastVisibleBase(true, true);
    for(int i=0; i< nSeqVisible; i++) {
        U2Region yRange = seqArea->getSequenceYRange(startSeq + i, true);
        int offs = cache->getBaseCounts(startSeq + i, pos, !showStartPos);
        int seqSize = cache->getBaseCounts(startSeq + i, aliLen - 1, true);
        QString  offset = QString::number(offs+1);
        if (showStartPos && offs == 0) {
            p.setPen(Qt::black);
            QRect lbr(OFFS_WIDGET_BORDER, yRange.startPos, lbw, yRange.length);
            p.drawText(lbr, Qt::AlignCenter, "[");
        } else if (!showStartPos && offs == seqSize) {
            p.setPen(Qt::black);
            QRect rbr(w - OFFS_WIDGET_BORDER - rbw, yRange.startPos, rbw, yRange.length);
            p.drawText(rbr, Qt::AlignCenter, "]");
            offset = QString::number(offs);
        } else {
            p.setPen(dg);
        }
        QRect tr(OFFS_WIDGET_BORDER + (showStartPos ? lbw : 0), yRange.startPos, w - 2 * OFFS_WIDGET_BORDER - (showStartPos ? lbw : rbw), yRange.length);
        p.drawText(tr, Qt::AlignRight | Qt::AlignVCenter, offset);
    }
}


}//namespace

