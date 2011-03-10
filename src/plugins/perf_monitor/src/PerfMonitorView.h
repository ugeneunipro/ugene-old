#ifndef _U2_PERF_VIEW_H_
#define _U2_PERF_VIEW_H_

#include <U2Gui/MainWindow.h>

#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

namespace U2 {

class GCounter;
class PerfTreeItem;

class PerfMonitorView : public MWMDIWindow {
    Q_OBJECT
public:
    PerfMonitorView();

protected:
    void timerEvent(QTimerEvent *e);

private:
    void updateCounters();
    PerfTreeItem* findCounterItem(const GCounter* c) const; 
    QTreeWidget* tree;
};

class PerfTreeItem : public QTreeWidgetItem {
public:
    PerfTreeItem(GCounter* counter);
    GCounter* counter;
    void updateVisual();
};

} //namespace

#endif
