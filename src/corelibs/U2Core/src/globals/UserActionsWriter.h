/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_USER_ACTIONS_WRITER_H_
#define _U2_USER_ACTIONS_WRITER_H_

#include <QEvent>
#include <QFile>
#include <QMouseEvent>
#include <QObject>
#include <QMutex>

#include <U2Core/global.h>
#include <U2Core/AppContext.h>

namespace U2 {

class U2CORE_EXPORT UserActionsWriter : public QObject {
    Q_OBJECT
public:
     UserActionsWriter();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void generateMouseMeassage();
    void appendTreeWidgetInfo(QWidget* parent);
    void appendAdditionalWidgetInfo(QWidget* w);
    void appendMouseButtonInfo();
    QString getWidgetText(QWidget* w);

    void generateKeyMessage();
    void appendModifiersInfo();

    void appendDialogInfo();
    void filterMessages();
    void filterMouseMessages();
    void filterKeyboardMessages();

    QMutex guard;
    QMap<QEvent::Type, QString> typeMap;
    QMap<Qt::KeyboardModifier, QString> modMap;
    QMap<Qt::Key, QString> keys;
    QMouseEvent* m;
    QKeyEvent* k;
    QString message;
    QString prevMessage;
    QString buffer;
    int counter;
    QPoint windowSize;
};

}   // namespace U2

#endif // _U2_USER_ACTIONS_WRITER_H_
