/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef GUITESTINGWINDOWIMPL_H
#define GUITESTINGWINDOWIMPL_H

#include <U2Gui/MainWindow.h>
#include <U2Core/AppContext.h>
#include <ui/ui_GUITestingWindow.h>

#include <QtGui/QtGui>

namespace U2 {
class EventFilter;

class GUITestingWindow:public QWidget, public Ui_GUITestWindow
{
    Q_OBJECT
public:
    GUITestingWindow();
    void updateTable();
private:
    void prepareGUI();
    EventFilter* filter;
private slots:
    void sl_getParentInfo();
};

class EventFilter : public QObject
{
    Q_OBJECT
    friend class GUITestingWindow;

public:
    EventFilter(GUITestingWindow* _w);
    const QString& getClassName() { return className; }
    const QString& getObjName() { return objName; }
    const QString& getActionText() { return text; }
    const QString& getActionName() { return actionName; }
    QObject* getBufferObject() { return bufferObj; }
    void setBufferObject(QPointer<QObject> o) { bufferObj = o; }

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:
    void generateMouseMeassage();
    void getInfo(QWidget* w);

    QString className;
    QString objName;
    QString actionName;
    QString text;

    QMouseEvent* m;
    GUITestingWindow* gtw;
    QPointer<QObject> bufferObj;
};

}
#endif // GUITESTINGWINDOWIMPL_H
