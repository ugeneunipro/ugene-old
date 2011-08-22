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

#ifndef _TOOL_TIP_H_
#define _TOOL_TIP_H_

#include <U2Core/global.h>
#include <U2Core/AppContext.h>
#include "MainWindow.h"
#include "NotificationWidget.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMainWindow>
#include <QtGui/QScrollArea>
#include <QtGui/QAction>
#include <QtGui/QMouseEvent>
#include <QtGui/QToolTip>
#include <QtGui/QHelpEvent>

#include <QtGui/QDialog>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>

#include <QtCore/QPoint>
#include <QtCore/QTimer>

namespace U2 {

#define MAX_NOTIFICATION 100

enum NotificationType {
    Info_Not,
    Error_Not,
    Report_Not
};

class U2GUI_EXPORT Notification: public QLabel {
    Q_OBJECT

public:
    Notification(const QString& message, NotificationType _type, QAction *_action = 0);
    ~Notification() {}

    void showNotification(int x, int y);
    QString getText() const;
    NotificationType getType() const;
    virtual bool eventFilter( QObject * watched, QEvent * event ); 
    void increaseCounter();

private slots:
    void sl_timeout();

private:
    void dissapear();
    void generateCSS(bool isHovered);
    void generateCSSforCloseButton(bool isHovered);

signals:
    void si_dissapear();
    void si_delete();

protected:
    bool event(QEvent *e);
    void mousePressEvent(QMouseEvent *ev);

private:
    QAction *action;
    QLabel *close;
    QTimer timer;

    QString text;
    NotificationType type;
    int timeCounter;
    //counter for duplicate notifications
    int counter;
};

class U2GUI_EXPORT NotificationStack: public QObject {
    Q_OBJECT
    
public:
    NotificationStack(QObject *o = NULL);
    ~NotificationStack();

    void addNotification(Notification *t);
    // a shortcut to addNotification for quick reporting errors
    void addError(const QString& errorMessage);
    int count() const;
    Notification *getNotification(int row) const;
    QList<Notification *>  getItems() const;
    void showStack();
    bool hasError() const;
    void setFixed(bool val);

 private slots:
     void sl_notificationDissapear();
     void sl_delete();

signals:
    void si_changed();

private:
    NotificationWidget *w;

    QList<Notification *> notifications;
    QList<Notification *> notificationsOnScreen;
    int notificationPosition;
    int notificationNumber;
};



}




#endif
