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

#include "CircularViewSplitter.h"
#include "CircularView.h"
#include "RestrictionMapWidget.h"
#include "ExportImageCircularViewDialog.h"

#include <U2Core/L10n.h>
#include <U2Core/GObject.h>
#include <U2Core/Settings.h>
#include <U2Core/GObjectTypes.h>

#include <U2Gui/HBar.h>
#include <U2Gui/DialogUtils.h>

#include <QtGui/QPixmap>
#include <QtGui/QPainter>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPrinter>
#include <QtGui/QMessageBox>
#include <QtGui/QTreeWidget>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTreeWidget>
#endif

namespace U2 {

CircularViewSplitter::CircularViewSplitter(AnnotatedDNAView* view) : ADVSplitWidget(view) {
    tbZoomIn = new QToolButton(this);
    tbZoomIn->setIcon(QIcon(":/core/images/zoom_in.png"));
    tbZoomIn->setToolTip(tr("Zoom In"));
    tbZoomIn->setFixedSize(20,20);

    tbZoomOut = new QToolButton(this);
    tbZoomOut->setIcon(QIcon(":/core/images/zoom_out.png"));
    tbZoomOut->setToolTip(tr("Zoom Out"));
    tbZoomOut->setFixedSize(20,20);

    tbFitInView = new QToolButton(this);
    tbFitInView->setIcon(QIcon(":/core/images/zoom_whole.png"));
    tbFitInView->setToolTip(tr("Fit To Full View"));
    tbFitInView->setFixedSize(20,20);

    tbExport = new QToolButton(this);
    tbExport->setIcon(QIcon(":/core/images/cam2.png"));
    tbExport->setToolTip(tr("Save circular view as image"));
    tbExport->setFixedSize(20,20);

    tbToggleRestrictionMap = new QToolButton(this);
    tbToggleRestrictionMap->setIcon(QIcon(":/circular_view/images/side_list.png"));
    tbToggleRestrictionMap->setToolTip(tr("Show/hide restriction sites map"));
    tbToggleRestrictionMap->setFixedSize(20,20);
    tbToggleRestrictionMap->setCheckable(true);
    tbToggleRestrictionMap->setChecked(true);
    connect(tbToggleRestrictionMap, SIGNAL(toggled(bool)),SLOT(sl_toggleRestrictionMap(bool)));

    toolBar = new HBar(this);
    toolBar->setOrientation(Qt::Vertical);

    toolBar->addWidget(tbZoomIn);
    toolBar->addWidget(tbZoomOut);
    toolBar->addWidget(tbFitInView);
    toolBar->addWidget(tbExport);
    toolBar->addWidget(tbToggleRestrictionMap);

    connect(tbExport, SIGNAL(pressed()), SLOT(sl_export()));

    splitter = new QSplitter(Qt::Horizontal);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,3,0);
    layout->addWidget(toolBar);
    layout->addWidget(splitter);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setBaseSize(600,600);
    setAcceptDrops(false);

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setSpacing(0);
    outerLayout->setContentsMargins(0,0,0,0);

    horScroll = new QScrollBar(Qt::Horizontal, this);
    horScroll->setMinimum(0);
    horScroll->setMaximum(360);
    horScroll->setSingleStep(5);
    connect(horScroll, SIGNAL(valueChanged(int)), SLOT(sl_horSliderMoved(int)));

    outerLayout->addLayout(layout);
    outerLayout->insertWidget(-1, horScroll);
}

void CircularViewSplitter::updateState( const QVariantMap& m) {
    Q_UNUSED(m);
    //TODO:
}

void CircularViewSplitter::saveState( QVariantMap& m ) {
    Q_UNUSED(m);
    //TODO:
}

void CircularViewSplitter::addView(CircularView* view, RestrctionMapWidget* rmapWidget) {
    tbFitInView->setDisabled(true);
    tbZoomOut->setDisabled(true);
    connect(tbZoomIn, SIGNAL(pressed()), view, SLOT(sl_zoomIn()));
    connect(tbZoomOut, SIGNAL(pressed()), view, SLOT(sl_zoomOut()));
    connect(tbFitInView, SIGNAL(pressed()), view, SLOT(sl_fitInView()));

    connect(view, SIGNAL(si_zoomInDisabled(bool)), SLOT(sl_updateZoomInAction(bool)));
    connect(view, SIGNAL(si_zoomOutDisabled(bool)), SLOT(sl_updateZoomOutAction(bool)));
    connect(view, SIGNAL(si_fitInViewDisabled(bool)), SLOT(sl_updateFitInViewAction(bool)));

    circularViewList.append(view);
    restrictionMapWidgets.append(rmapWidget);

    splitter->addWidget(view);
    splitter->addWidget(rmapWidget);
    
    splitter->setStretchFactor(splitter->indexOf(view), 10);
    splitter->setStretchFactor(splitter->indexOf(rmapWidget), 1);
    
    connect(view, SIGNAL(si_wheelMoved(int)), SLOT(sl_moveSlider(int)));
}

void CircularViewSplitter::sl_moveSlider(int delta) {
    delta*=-1;
    int oldPos = horScroll->sliderPosition();
    int step = qMin(QApplication::wheelScrollLines() * horScroll->singleStep(), horScroll->pageStep());
    int offset = delta/120 * step;
    if (qAbs(offset)<1) {
        return;
    }
    int newPos = oldPos + offset;
    horScroll->setSliderPosition(newPos);
}

void CircularViewSplitter::removeView(CircularView* view, RestrctionMapWidget* rmapWidget) {
    circularViewList.removeAll(view);
    restrictionMapWidgets.removeAll(rmapWidget);
}

bool CircularViewSplitter::isEmpty() {
    return circularViewList.isEmpty();
}

bool noValidExtension(const QString& url) {
    QFileInfo fi(url);
    if (fi.suffix().isEmpty()) {
        return true;
    }

    QStringList validExtensions;
    validExtensions << "png" << "bmp" << "jpg" << "jpeg" << "ppm" <<
        "xbm" << "xpm" << "svg" << "pdf" << "ps";

    if (!validExtensions.contains(fi.suffix())) {
        return true;
    }

    return false;
}

void CircularViewSplitter::sl_export() {
    CircularView* cv = circularViewList.last();
    ExportImageCVDialog dialog(cv);
    dialog.exec();
    tbExport->setDown(false);
}

void CircularViewSplitter::sl_horSliderMoved(int newVal) {
    foreach(CircularView* cv, circularViewList) {
        cv->setAngle(newVal);
    }
}

void CircularViewSplitter::adaptSize() {

    QWidget* widget = parentWidget();
    
    Q_ASSERT(widget != NULL);
    QSplitter* parentSplitter = qobject_cast<QSplitter* > (widget);

    int index = parentSplitter->indexOf(this);
    QList<int> sizes = parentSplitter->sizes();

    int splitterSize = 0;

    int psH = parentSplitter->height();
    int psW = parentSplitter->width();

    if (parentSplitter->orientation() == Qt::Horizontal) {
        splitterSize = psH;
    }
    else {
        splitterSize = psW;
    }

    if (!splitterSize) {
        return;
    }

    int midSize = splitterSize;

    if (splitterSize > CircularViewRenderArea::MIDDLE_ELLIPSE_SIZE) {
        sizes[index] = CircularViewRenderArea::MIDDLE_ELLIPSE_SIZE;

        midSize -= CircularViewRenderArea::MIDDLE_ELLIPSE_SIZE;
        if (sizes.count() > 1) {
            midSize /= (sizes.count() - 1);
        }
    }
    else {
        midSize /= sizes.count();
        sizes[index] = midSize;
    }

    for (int i=0; i<sizes.count(); i++) {
        if (i != index) {
            sizes[i] = midSize;
        }
    }
    parentSplitter->setSizes(sizes);
}

void CircularViewSplitter::sl_updateZoomInAction( bool disabled) {
    tbZoomIn->setDisabled(disabled);
}

void CircularViewSplitter::sl_updateZoomOutAction( bool disabled) {
    tbZoomOut->setDisabled(disabled);
}

void CircularViewSplitter::sl_updateFitInViewAction( bool disabled) {
    tbFitInView->setDisabled(disabled);
}

void CircularViewSplitter::sl_toggleRestrictionMap( bool toggle)
{
    foreach (QWidget* w, restrictionMapWidgets) {
        w->setVisible(toggle);
    }
}

} //namespace U2
