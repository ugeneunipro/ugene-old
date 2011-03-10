#include <QtGui/QPainter>

#include "AssemblyDensityGraph.h"

namespace U2 {

AssemblyDensityGraph::AssemblyDensityGraph(AssemblyBrowserUi * ui_) : ui(ui_)
{
    setFixedHeight(50);
}

void AssemblyDensityGraph::drawAll() {

}

void AssemblyDensityGraph::paintEvent(QPaintEvent * e) {
    drawAll();
    QWidget::paintEvent(e);
}

} //ns