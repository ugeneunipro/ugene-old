#ifndef __ASSEMBLY_DENSITY_GRAPH_H__
#define __ASSEMBLY_DENSITY_GRAPH_H__

#include <QtGui/QWidget>

namespace U2 {

class AssemblyBrowserUi;

class AssemblyDensityGraph: public QWidget {
    Q_OBJECT
public:
    AssemblyDensityGraph(AssemblyBrowserUi * ui);
protected:
    void paintEvent(QPaintEvent * e);
private:
    void drawAll();

    AssemblyBrowserUi * ui;
};

} //ns

#endif 
