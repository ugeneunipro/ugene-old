#ifndef _NOTIFICATION_WIDGET_H
#define _NOTIFICATION_WIDGET_H

#include <assert.h>

#include <QtCore/QObject>
#include <QtCore/QPoint>

#include <QtGui/QScrollArea>
#include <QtGui/QLabel>
#include <QtGui/QMouseEvent>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QStyle>

namespace U2 {

#define TT_HEIGHT 50
#define TT_WIDTH 200

#define TS_HEIGHT 350
#define TS_WIDTH TT_WIDTH + 27

class Header: public QFrame {
Q_OBJECT

public:
    Header(QWidget *w = NULL); 
    bool isFixed();

protected: 
    void mousePressEvent(QMouseEvent *me) ; 
    void mouseMoveEvent(QMouseEvent *me) ;
    bool eventFilter(QObject *, QEvent *);

private:
    QLabel *close;
    QLabel *pin;

    bool fix;

    QPoint startPos;
    QPoint clickPos;
    QPoint offset;
};

class NotificationWidget: public QFrame {
Q_OBJECT

public:
    NotificationWidget(QWidget *w = NULL);
    Header *titleBar() const;
    void addNotification(QWidget *w);
    void removeNotification(QWidget *w);
    void setFixed(bool val);

protected:
    bool event(QEvent *event); 

private:
    QScrollArea *scrlArea;
    QVBoxLayout *layout;
    QFrame *frame;
    Header *header;

    QPoint m_old_pos; 
    bool m_mouse_down; 
    bool left, right, bottom; 

    bool isFixed;
};

} //namespace
#endif
