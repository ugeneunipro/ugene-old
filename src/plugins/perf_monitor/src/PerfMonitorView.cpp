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
