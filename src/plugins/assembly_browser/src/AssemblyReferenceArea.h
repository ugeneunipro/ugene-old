#ifndef __ASSEMBLY_REFERENCE_AREA_H__
#define __ASSEMBLY_REFERENCE_AREA_H__

#include <QtGui/QWidget>

#include "AssemblyCellRenderer.h"

namespace U2 {

class AssemblyBrowserUi;
class AssemblyBrowserWindow;
class AssemblyModel;

class AssemblyReferenceArea: public QWidget {
    Q_OBJECT
public:
    AssemblyReferenceArea(AssemblyBrowserUi * ui);

protected:
    void paintEvent(QPaintEvent * e);
    void resizeEvent(QResizeEvent * e);

private slots:
    void sl_redraw();

private:
    void connectSlots();
    void drawAll();
    void drawReference(QPainter & p);

private:
    AssemblyBrowserUi * ui;
    AssemblyBrowserWindow * window;
    AssemblyModel * model;

    QPixmap cachedView;
    bool redraw;
    AssemblyCellRenderer cellRenderer;

    const static int FIXED_HEIGHT = 25;
};

} //ns

#endif 
