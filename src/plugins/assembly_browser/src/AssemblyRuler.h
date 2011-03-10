#ifndef __ASSEMBLY_RULER__
#define __ASSEMBLY_RULER__


#include <QtGui/QWidget>

namespace U2 {

class AssemblyBrowserUi;
class AssemblyBrowserWindow;
class AssemblyModel;

class AssemblyRuler : public QWidget {
    Q_OBJECT
public:
    AssemblyRuler(AssemblyBrowserUi * ui);

protected:
    void paintEvent(QPaintEvent * e);
    void resizeEvent(QResizeEvent * e);
    void mouseMoveEvent(QMouseEvent * e);

private slots:
    void sl_redraw();

private:
    void connectSlots();
    void drawAll();
    void drawRuler(QPainter & p);
    void drawCursor(QPainter & p);

private:
    AssemblyBrowserUi * ui;
    AssemblyBrowserWindow * window;
    AssemblyModel * model;

    QPixmap cachedView;
    bool redraw;

//    qint64 cursorPos

    const static int FIXED_HEIGHT = 25;
};

} //ns

#endif