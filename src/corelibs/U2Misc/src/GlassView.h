#ifndef _U2_GLASS_VIEW_H_
#define _U2_GLASS_VIEW_H_

#include <U2Core/global.h>
#include <QtGui/QGraphicsView>

class QTextDocument;

namespace U2 {

class U2MISC_EXPORT GlassPane : public QWidget {
    Q_OBJECT
public:
    GlassPane(QWidget* parent = 0) : QWidget(parent) {}
    virtual void paint(QPainter*) = 0;
    virtual bool eventFilter(QObject*, QEvent* e) {
        return this->event(e);
    }
};


class U2MISC_EXPORT GlassView : public QGraphicsView {
    Q_OBJECT
public:
    GlassView(QGraphicsScene * scene) : QGraphicsView(scene), glass(NULL) {}

public slots:
    void setGlass(GlassPane*);

protected:
    bool viewportEvent(QEvent * event);
    void paintEvent(QPaintEvent *);
    void scrollContentsBy(int dx, int dy);
    
    GlassPane* glass;
};

}//namespace
#endif
