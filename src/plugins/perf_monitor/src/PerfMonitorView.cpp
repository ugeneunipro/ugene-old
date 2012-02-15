/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "PerfMonitorView.h"

#include <U2Core/Counter.h>
#include <U2Core/Timer.h>

#include <QtGui/QVBoxLayout>

namespace U2 {

static GCounter updateCounter("PerfMoninor::updateCounters", TimeCounter::getCounterSuffix(), TimeCounter::getCounterScale());

PerfMonitorView::PerfMonitorView() : MWMDIWindow(tr("Application counters")){
    tree = new QTreeWidget();
    tree->setColumnCount(3);
    tree->setSortingEnabled(true);
    tree->setColumnCount(0);
    
    tree->headerItem()->setText(0, tr("Name"));
    tree->headerItem()->setText(1, tr("Value"));
    tree->headerItem()->setText(2, tr("Scale"));

    QVBoxLayout* l = new QVBoxLayout();
    l->setMargin(0);
    l->addWidget(tree);
    setLayout(l);

    updateCounter.totalCount = 0;
    
    updateCounters();

    startTimer(1000);
}

void PerfMonitorView::timerEvent(QTimerEvent *) {
    TimeCounter c(&updateCounter);
    updateCounters();
}

void PerfMonitorView::updateCounters() {
    foreach(GCounter* c, GCounter::allCounters()) {
        PerfTreeItem* ci = findCounterItem(c);
        if (ci!=NULL) {
            ci->updateVisual();
        } else {
            ci = new PerfTreeItem(c);
            tree->addTopLevelItem(ci);
        }
    }
}

PerfTreeItem* PerfMonitorView::findCounterItem(const GCounter* c) const {
    for (int i=0, n = tree->topLevelItemCount(); i<n; i++) {
        PerfTreeItem* ci = static_cast<PerfTreeItem*>(tree->topLevelItem(i));
        if (ci->counter == c) {
            return ci;
        }
    }
    return NULL;
}

PerfTreeItem::PerfTreeItem(GCounter* c) : counter(c) {
    updateVisual();
}

void PerfTreeItem::updateVisual() {
    setText(0, counter->name);
    setText(1, QString::number(counter->scaledTotal()));
    setText(2, counter->suffix);
}

} //namespace
