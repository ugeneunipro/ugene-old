/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "DotPlotSplitter.h"
#include "DotPlotWidget.h"
#include "DotPlotFilterDialog.h"

#include <U2Gui/HBar.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QString>
#include <QtCore/QPair>
#include <QtCore/QSet>

#include <QtGui/QVBoxLayout>
#include <QtGui/QAction>
#include <QtGui/QMenu>

#include <QtGui/QToolButton>

namespace U2 {

DotPlotSplitter::DotPlotSplitter(AnnotatedDNAView* a)
: ADVSplitWidget(a), locked(false)
{
    QLayout *layout = new QHBoxLayout;
    if (!layout) {
        return;
    }

    layout->setSpacing(0);
    layout->setContentsMargins(0,0,3,0);


    syncLockButton = createToolButton(":core/images/sync_lock.png", tr("Multiple view synchronization lock"), SLOT(sl_toggleSyncLock(bool)));
    filterButton = createToolButton(":dotplot/images/filter.png", tr("Filter results"), SLOT(sl_toggleFilter()), false);
    aspectRatioButton = createToolButton(":dotplot/images/aspectRatio.png", tr("Keep aspect ratio"), SLOT(sl_toggleAspectRatio(bool)));
    zoomInButton = createToolButton(":core/images/zoom_in.png", tr("Zoom in (<b> + </b>)"), SLOT(sl_toggleZoomIn()), false);
    zoomOutButton = createToolButton(":core/images/zoom_out.png", tr("Zoom out (<b> - </b>)"), SLOT(sl_toggleZoomOut()), false);
    resetZoomingButton = createToolButton(":core/images/zoom_whole.png", tr("Reset zooming (<b>0</b>)"), SLOT(sl_toggleZoomReset()), false);
    selButton = createToolButton(":dotplot/images/cursor.png", tr("Select tool (<b>S</b>)"), SLOT(sl_toggleSel()));
    handButton = createToolButton(":dotplot/images/hand_icon.png", tr("Hand tool (<b>H</b>)"), SLOT(sl_toggleHand())); 
    

    syncLockButton->setAutoRaise(true);
    syncLockButton->setAutoFillBackground(true);

    filterButton->setAutoRaise(true);
    filterButton->setAutoFillBackground(true);

    aspectRatioButton->setAutoRaise(true);

    zoomInButton->setAutoRaise(true);
    zoomOutButton->setAutoRaise(true);
    resetZoomingButton->setAutoRaise(true);

    selButton->setAutoRaise(true);
    selButton->setChecked(true);

    handButton->setAutoRaise(true);

    buttonToolBar = new HBar(this);
    if (!buttonToolBar) {
        return;
    }

    buttonToolBar->setOrientation(Qt::Vertical);
    buttonToolBar->setFloatable(false);
    buttonToolBar->setMovable(false);
    buttonToolBar->setStyleSheet("background: ");

    buttonToolBar->addWidget(filterButton);
    buttonToolBar->addWidget(syncLockButton);
//  buttonToolBar->addWidget(aspectRatioButton); // todo: not implemented yet
    buttonToolBar->addWidget(zoomInButton);
    buttonToolBar->addWidget(zoomOutButton);
    buttonToolBar->addWidget(resetZoomingButton);
    buttonToolBar->addWidget(selButton);
    buttonToolBar->addWidget(handButton);
    

    splitter = new QSplitter(Qt::Horizontal);
    if (!splitter) {
        return;
    }

    layout->addWidget(buttonToolBar);
    layout->setAlignment(buttonToolBar, Qt::AlignTop);
    layout->addWidget(splitter);
    setAcceptDrops(false);

    setLayout(layout);
    setFocus();
}

bool DotPlotSplitter::onCloseEvent() {

    foreach (DotPlotWidget* w, dotPlotList) {
        bool canClose = w->onCloseEvent();
        if (!canClose) {
            return false;
        }
    }

    return true;
}

QToolButton *DotPlotSplitter::createToolButton(const QIcon& ic, const QString& toolTip, const char *slot, bool checkable) {

    QToolButton *toolButton = new QToolButton(this);
    if (toolButton) {
        toolButton->setIcon(ic);
        toolButton->setToolTip(toolTip);
        toolButton->setFixedWidth(20);
        toolButton->setFixedHeight(20);
        toolButton->setCheckable(checkable);
        if (checkable) {
            connect(toolButton, SIGNAL(toggled(bool)), this, slot);
        }
        else {
            connect(toolButton, SIGNAL(clicked()), this, slot);
        }
    }

    return toolButton;
}

QToolButton *DotPlotSplitter::createToolButton(const QString& iconPath, const QString& toolTip, const char *slot, bool checkable) {

    return createToolButton(QIcon(iconPath), toolTip, slot, checkable);
}

DotPlotSplitter::~DotPlotSplitter() {

    delete syncLockButton;
    delete filterButton;
    delete aspectRatioButton;
    delete zoomInButton;
    delete zoomOutButton;
    delete resetZoomingButton;
    delete handButton;
    delete selButton;

    delete buttonToolBar;
    delete splitter;
}

void DotPlotSplitter::addView(DotPlotWidget* view) {

    dotPlotList.append(view);
    SAFE_POINT(splitter, "splitter is NULL", );
    splitter->addWidget(view);

    connect(view,
        SIGNAL(si_dotPlotChanged(ADVSequenceObjectContext*, ADVSequenceObjectContext*, float, float, QPointF)),
        SLOT(sl_dotPlotChanged(ADVSequenceObjectContext*, ADVSequenceObjectContext*, float, float, QPointF)));

    connect(view,
        SIGNAL(si_dotPlotSelecting()),
        SLOT(sl_dotPlotSelecting()));

    checkLockButtonState();
    view->setFocus();
    updateButtonState();
}

void DotPlotSplitter::removeView(DotPlotWidget* view) {

    dotPlotList.removeAll(view);
    checkLockButtonState();
}

typedef QPair<QString,QString> QStringPair;

void DotPlotSplitter::checkLockButtonState() {

    QSet<QStringPair> dotPlots;

    bool enableLockButton = false;

    int dotPlotCount = dotPlotList.count();
    foreach (DotPlotWidget *dpWidget, dotPlotList) {

        QString nameX = dpWidget->getXSequenceName();
        QString nameY = dpWidget->getYSequenceName();

        if (!nameX.isEmpty() && !nameY.isEmpty()) {

            QStringPair p;
            p.first = nameX;
            p.second = nameY;

            if (dotPlots.contains(p)) {
                enableLockButton = true;
            }
            else {
                dotPlots.insert(p);
            }

            dpWidget->setIgnorePanView(dotPlotCount > 1);
        }
    }

    if (syncLockButton) {
        syncLockButton->setEnabled(enableLockButton);
        if (!enableLockButton) { // disabled button should not be checked
            syncLockButton->setChecked(false);
        }
    }
}

bool DotPlotSplitter::isEmpty() const{

    return dotPlotList.isEmpty();
}

// build popup menu for all DotPlotWidgets
// each DotPlotWidget defines if it should add menu
void DotPlotSplitter::buildPopupMenu(QMenu *m) {
    foreach (DotPlotWidget *w, dotPlotList) {
        SAFE_POINT(w, "w is NULL", );
        w->buildPopupMenu(m);
    }
}

void DotPlotSplitter::sl_toggleSyncLock(bool l) {

    locked = l;
}

void DotPlotSplitter::sl_toggleFilter(){
    foreach (DotPlotWidget *w, dotPlotList) {
        SAFE_POINT(w, "w is NULL", );
        w->sl_filter();
        break; //todo: support several widgets
    }
}

void DotPlotSplitter::sl_toggleAspectRatio(bool aspectRatio) {
    bool noFocus = true;
    foreach (DotPlotWidget *dpWidget, dotPlotList) {
        if(dpWidget->hasFocus()){
            dpWidget->setKeepAspectRatio(aspectRatio);
            noFocus = false;
            break;
        }
    }
    if(noFocus){
        foreach (DotPlotWidget *dpWidget, dotPlotList) {
            dpWidget->setKeepAspectRatio(aspectRatio);
        }
    }
}

void DotPlotSplitter::sl_toggleZoomIn() {
    bool noFocus = true;
    foreach (DotPlotWidget *dpWidget, dotPlotList) {
        if(dpWidget->hasFocus()){
            dpWidget->zoomIn();
            noFocus = false;
            break;
        }
    }
    if(noFocus){
        foreach (DotPlotWidget *dpWidget, dotPlotList) {
            dpWidget->zoomIn();
        }
    }
    updateButtonState();
}

void DotPlotSplitter::sl_toggleZoomOut() {
    bool noFocus = true;
    foreach (DotPlotWidget *dpWidget, dotPlotList) {
        if(dpWidget->hasFocus()){
            dpWidget->zoomOut();
            noFocus = false;
            break;
        }
    }
    if(noFocus){
        foreach (DotPlotWidget *dpWidget, dotPlotList) {
            dpWidget->zoomOut();
        }
    }
    updateButtonState();
}

void DotPlotSplitter::sl_toggleZoomReset() {
    bool noFocus = true;
    foreach (DotPlotWidget *dpWidget, dotPlotList) {
        if(dpWidget->hasFocus()){
            dpWidget->zoomReset();
            noFocus = false;
            break;
        }
    }
    if(noFocus){
        foreach (DotPlotWidget *dpWidget, dotPlotList) {
            dpWidget->zoomReset();
        }
    }
    updateButtonState();
}

void DotPlotSplitter::sl_toggleSel(){
    handButton->setChecked(!selButton->isChecked());

    foreach (DotPlotWidget *dpWidget, dotPlotList) {
        dpWidget->setSelActive(selButton->isChecked());
    }
}
void DotPlotSplitter::sl_toggleHand(){
    selButton->setChecked(!handButton->isChecked());

    foreach (DotPlotWidget *dpWidget, dotPlotList) {
        dpWidget->setSelActive(selButton->isChecked());
    }
}

void DotPlotSplitter::sl_dotPlotChanged(ADVSequenceObjectContext* sequenceX, ADVSequenceObjectContext* sequenceY, float shiftX, float shiftY, QPointF zoom) {

    checkLockButtonState();

    if (locked) {
        foreach (DotPlotWidget* w, dotPlotList) {
            w->setShiftZoom(sequenceX, sequenceY, shiftX, shiftY, zoom);
        }
        update();
        
    }
    updateButtonState();    
    
}

void DotPlotSplitter::sl_dotPlotSelecting() {
    bool selecting = false;
    foreach (DotPlotWidget* w, dotPlotList) {
        selecting = w->hasSelection();

        if (selecting) {
            break;
        }
    }
}

void DotPlotSplitter::updateButtonState(){
    bool noFocus = true;
    foreach (DotPlotWidget *dpWidget, dotPlotList) {
        if (dpWidget->hasFocus()){
            zoomInButton->setEnabled(dpWidget->canZoomIn());
            zoomOutButton->setEnabled(dpWidget->canZoomOut());
            resetZoomingButton->setEnabled(dpWidget->canZoomOut());
            noFocus = false;
            break;
        }
    }   
    if (noFocus && !dotPlotList.isEmpty()){
        DotPlotWidget *dpWidget = dotPlotList.first();
        zoomInButton->setEnabled(dpWidget->canZoomIn());
        zoomOutButton->setEnabled(dpWidget->canZoomOut());
        resetZoomingButton->setEnabled(dpWidget->canZoomOut());
        handButton->setShortcut(QKeySequence());
        selButton->setShortcut(QKeySequence());
        zoomInButton->setShortcut(QKeySequence());
        zoomOutButton->setShortcut(QKeySequence());
        resetZoomingButton->setShortcut(QKeySequence());
    } else {
        handButton->setShortcut('H');
        selButton->setShortcut('S');
        zoomInButton->setShortcut('+');
        zoomOutButton->setShortcut('-');
        resetZoomingButton->setShortcut('0');
    }
}

} // namespace
