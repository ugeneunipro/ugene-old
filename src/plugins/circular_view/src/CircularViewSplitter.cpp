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

#include "CircularViewSplitter.h"
#include "CircularView.h"
#include "RestrictionMapWidget.h"

#include <U2Core/L10n.h>
#include <U2Core/GObject.h>
#include <U2Core/Settings.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/AnnotationTableObject.h>

#include <U2Misc/HBar.h>
#include <U2Misc/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtSvg/QSvgGenerator>
#include <QtGui/QPrinter>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/QMessageBox>
#include <QtGui/QTreeWidget>

#include <QtGui/QApplication>
#include <QDomDocument>


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

void CircularViewSplitter::addView(CircularView* view) {
    tbFitInView->setDisabled(true);
    tbZoomOut->setDisabled(true);
    connect(tbZoomIn, SIGNAL(pressed()), view, SLOT(sl_zoomIn()));
    connect(tbZoomOut, SIGNAL(pressed()), view, SLOT(sl_zoomOut()));
    connect(tbFitInView, SIGNAL(pressed()), view, SLOT(sl_fitInView()));

    connect(view, SIGNAL(si_zoomInDisabled(bool)), SLOT(sl_updateZoomInAction(bool)));
    connect(view, SIGNAL(si_zoomOutDisabled(bool)), SLOT(sl_updateZoomOutAction(bool)));
    connect(view, SIGNAL(si_fitInViewDisabled(bool)), SLOT(sl_updateFitInViewAction(bool)));

    circularViewList.append(view);
    splitter->insertWidget(0, view);
    
    RestrctionMapWidget* rmapWidget = new RestrctionMapWidget(view->getSequenceContext(),this);
    splitter->insertWidget(1, rmapWidget);
    restrictionMapWidgets.append(rmapWidget);
    
    splitter->setStretchFactor(0,10);
    splitter->setStretchFactor(1,1);
    
    adaptSize();
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

void CircularViewSplitter::removeView(CircularView* view) {
    circularViewList.removeAll(view);
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
    QString rasterExt = tr("Raster image (*.png *.bmp *.jpg *.jpeg *.ppm *.xbm *.xpm)");
    QString vectorExt = tr("Vector image (*.svg)");
    QString pdfExt = tr("Portable document (*.pdf *.ps)");
    QString filter = rasterExt + ";;" + vectorExt + ";;" + pdfExt;
    LastOpenDirHelper lod("image");
    QString selectedFilter;
    lod.url = QFileDialog::getSaveFileName(this, tr("Export circular view to image"), lod.dir, filter, &selectedFilter);
    QPainter painter;
    CircularView* cv = circularViewList.last();
    if (!lod.url.isEmpty()) {
        bool result = false;
        if (noValidExtension(lod.url)) {
            if (selectedFilter == vectorExt) {
                lod.url += ".svg";
            } else if (selectedFilter == pdfExt) {
                lod.url += ".pdf";
            } else {
                lod.url += ".png";
            }
        }
        if (lod.url.endsWith(".svg", Qt::CaseInsensitive)) {
            QSvgGenerator generator;
            generator.setFileName(lod.url);
            generator.setSize(cv->size());
            generator.setViewBox(rect());
            
            painter.begin(&generator);
            cv->paint(painter);
            result = painter.end();
            //fix for UGENE-76
            QDomDocument doc("svg");
            QFile file(lod.url);
            bool ok=file.open(QIODevice::ReadOnly);
            if (!ok && !result){
               result=false;
            }
            ok=doc.setContent(&file);
            if (!ok && !result) {
                file.close();
                result=false;
            }
            if(result){
                file.close();
                QDomNodeList radialGradients=doc.elementsByTagName("radialGradient");
                for(uint i=0;i<radialGradients.length();i++){
                    if(radialGradients.at(i).isElement()){
                        QDomElement tag=radialGradients.at(i).toElement();
                        if(tag.hasAttribute("xml:id")){
                            QString id=tag.attribute("xml:id");
                            tag.removeAttribute("xml:id");
                            tag.setAttribute("id",id);
                        }
                    }
                }
                file.open(QIODevice::WriteOnly);
                file.write(doc.toByteArray());
                file.close();
            }
            //end of fix UGENE-76
        } else if (lod.url.endsWith(".pdf", Qt::CaseInsensitive) || lod.url.endsWith(".ps", Qt::CaseInsensitive)) {
            QPrinter printer;
            printer.setOutputFileName(lod.url);

            painter.setRenderHint(QPainter::Antialiasing);
            painter.begin(&printer);
            cv->paint(painter);
            result = painter.end();
        } else {
            QPixmap pixmap(cv->size());
            painter.fillRect(pixmap.rect(), Qt::white);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.begin(&pixmap);
            cv->paint(painter);
            result = painter.end() & pixmap.save(lod.url);
        }
        if (!result) {
            QMessageBox::critical(this, L10N::errorTitle(), tr("Unexpected error while exporting image!"));
        }
    }
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
