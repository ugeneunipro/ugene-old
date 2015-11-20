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


#include "drivers/GTMouseDriver.h"
#include "primitives/GTMainWindow.h"
#include "primitives/GTWidget.h"
#include "utils/GTThread.h"

#include <QApplication>
#include <QComboBox>
#include <QMainWindow>
#include <QStyle>

namespace HI {
#define GT_CLASS_NAME "GTWidget"

#define GT_METHOD_NAME "click"
void GTWidget::click(U2::U2OpStatus &os, QWidget *w, Qt::MouseButton mouseButton, QPoint p, bool safe) {

    GTGlobals::sleep(100);
    GT_CHECK(w != NULL, "widget is NULL");
//    GT_CHECK(w->isEnabled() == true, "widget " + w->objectName() + "is not enabled");

    if (p.isNull()) {
        p = w->rect().center();

        // TODO: this is a fast fix
        if (w->objectName().contains("ADV_single_sequence_widget")) {
            p += QPoint(0, 8);
        }
    }
    GTMouseDriver::moveTo(os, w->mapToGlobal(p));
    if(safe){
        GTMouseDriver::click(os, mouseButton);
    }else{
        //sometimes GTGlobals::sleep(os) should not be used after clicking
        GTMouseDriver::press(os, mouseButton);
        GTMouseDriver::release(os, mouseButton);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setFocus"
void GTWidget::setFocus(U2::U2OpStatus &os, QWidget *w) {

    GT_CHECK(w != NULL, "widget is NULL");

    GTWidget::click(os, w);
    GTGlobals::sleep(1000);

    if(!qobject_cast<QComboBox*>(w)){
        GT_CHECK(w->hasFocus(), "Can't set focus on widget " + w->objectName());
    }

}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findWidget"
QWidget* GTWidget::findWidget(U2::U2OpStatus &os, const QString &widgetName, QWidget *parentWidget, const GTGlobals::FindOptions& options) {
    Q_UNUSED(os);

    if (parentWidget == NULL) {
        parentWidget = GTMainWindow::getMainWindowAsWidget(os);
    }
    QWidget* widget = parentWidget->findChild<QWidget*>(widgetName);

    if (options.failIfNull) {
        GT_CHECK_RESULT(widget != NULL, "Widget " + widgetName + " not found", NULL);
    }

    return widget;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getWidgetCenter"
QPoint GTWidget::getWidgetCenter(U2::U2OpStatus &os, QWidget *w){
    Q_UNUSED(os)
    return w->mapToGlobal(w->rect().center());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findButtonByText"
QAbstractButton* GTWidget::findButtonByText(U2::U2OpStatus &os, const QString &text, QWidget *parentWidget, const GTGlobals::FindOptions& options) {

    if (parentWidget == NULL) {
        parentWidget = GTMainWindow::getMainWindowAsWidget(os);
    }
    QList<QAbstractButton*> buttonList = parentWidget->findChildren<QAbstractButton*>();
    QList<QAbstractButton*> foundButtonList;
    QStringList sL;

    foreach(QAbstractButton* but, buttonList){
        sL.append(but->text());
        if (but->text().contains(text, Qt::CaseInsensitive)){
            foundButtonList << but;
        }
    }

    GT_CHECK_RESULT(foundButtonList.count()<=1, QString("there are %1 buttons with such text").arg(foundButtonList.count()), NULL);

    if (options.failIfNull) {
        GT_CHECK_RESULT(foundButtonList.count() != 0, QString("button with this text <%1> not found").arg(text), NULL);
    }

    return foundButtonList.takeFirst();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findWidget"
void GTWidget::getAllWidgetsInfo(U2::U2OpStatus &os, QWidget *parent){

    if(parent == NULL){
        parent = GTMainWindow::getMainWindowAsWidget(os);
    }

    QList<QObject*> list= parent->findChildren<QObject*>();
    QString actStr;
    actStr.append("Getting all info about widget\n");

    foreach(QObject* act, list){
        actStr.append(act->objectName()+ "  " + act->metaObject()->className() + "  " + /*QString("%1").arg(act->isVisible()) + " " + QString("%1").arg(act->geometry().width()) +*/ "\n");
    }
    CHECK_SET_ERR(false, actStr);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "close"
void GTWidget::close(U2::U2OpStatus &os, QWidget *widget) {
#ifndef Q_OS_MAC
    GT_CHECK(NULL != widget, "Widget is NULL");

    class Scenario : public U2::CustomScenario {
    public:
        Scenario(QWidget *widget) :
            widget(widget)
        {

        }

        void run(U2::U2OpStatus &os) {
            Q_UNUSED(os);
            CHECK_SET_ERR(NULL != widget, "Widget is NULL");
            widget->close();
            GTGlobals::sleep(100);
        }

    private:
        QWidget *widget;
    };

    GTThread::runInMainThread(os, new Scenario(widget));
#else
    const QPoint closeButtonPos = GTWidget::getWidgetGlobalTopLeftPoint(os, widget) + QPoint(10, 5);
    GTMouseDriver::moveTo(os, closeButtonPos);
    GTMouseDriver::click(os);
    GTGlobals::sleep(100);
#endif
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "showMaximized"
void GTWidget::showMaximized(U2::U2OpStatus &os, QWidget *widget) {
    GT_CHECK(NULL != widget, "Widget is NULL");

    class Scenario : public U2::CustomScenario {
    public:
        Scenario(QWidget *widget) :
            widget(widget)
        {

        }

        void run(U2::U2OpStatus &os) {
            Q_UNUSED(os);
            CHECK_SET_ERR(NULL != widget, "Widget is NULL");
            widget->showMaximized();
            GTGlobals::sleep(100);
        }

    private:
        QWidget *widget;
    };

    GTThread::runInMainThread(os, new Scenario(widget));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "showNormal"
void GTWidget::showNormal(U2::U2OpStatus &os, QWidget *widget) {
    GT_CHECK(NULL != widget, "Widget is NULL");

    class Scenario : public U2::CustomScenario {
    public:
        Scenario(QWidget *widget) :
            widget(widget)
        {

        }

        void run(U2::U2OpStatus &os) {
            Q_UNUSED(os);
            CHECK_SET_ERR(NULL != widget, "Widget is NULL");
            widget->showNormal();
            GTGlobals::sleep(100);
        }

    private:
        QWidget *widget;
    };

    GTThread::runInMainThread(os, new Scenario(widget));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getColor"
QColor GTWidget::getColor(U2::U2OpStatus &os, QWidget *widget, const QPoint &point) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(NULL != widget, "Widget is NULL", QColor());

    return QColor(getImage(os, widget).pixel(point));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPixmap"
QPixmap GTWidget::getPixmap(U2::U2OpStatus &os, QWidget *widget) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(NULL != widget, "Widget is NULL", QPixmap());

    class Scenario : public U2::CustomScenario {
    public:
        Scenario(QWidget *widget, QPixmap &pixmap) :
            widget(widget),
            pixmap(pixmap)
        {

        }

        void run(U2::U2OpStatus &os) {
            Q_UNUSED(os);
            CHECK_SET_ERR(NULL != widget, "Widget to grab is NULL");
            pixmap = widget->grab(widget->rect());
        }

    private:
        QWidget *widget;
        QPixmap &pixmap;
    };

    QPixmap pixmap;
    GTThread::runInMainThread(os, new Scenario(widget, pixmap));
    return pixmap;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getImage"
QImage GTWidget::getImage(U2::U2OpStatus &os, QWidget *widget) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(NULL != widget, "Widget is NULL", QImage());

    return getPixmap(os, widget).toImage();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickLabelLink"
void GTWidget::clickLabelLink(U2::U2OpStatus &os, QWidget *label, int step, int indent){

    QRect r = label->rect();

    int left = r.left();
    int right = r.right();
    int top = r.top() + indent;
    int bottom = r.bottom();
    for(int i = left; i < right; i+=step){
        for(int j = top; j < bottom; j+=step){
            GTMouseDriver::moveTo(os, label->mapToGlobal(QPoint(i,j)));
            if(label->cursor().shape() == Qt::PointingHandCursor){
                GTGlobals::sleep(500);
                GTMouseDriver::click(os);
                return;
            }
        }
    }
    GT_CHECK(false, "label does not contain link");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickWindowTitle"
void GTWidget::clickWindowTitle(U2::U2OpStatus &os, QWidget *window) {
    GT_CHECK(NULL != window, "Window is NULL");

    QStyleOptionTitleBar opt;
    opt.initFrom(window);
    const QRect titleLabelRect = window->style()->subControlRect(QStyle::CC_TitleBar, &opt, QStyle::SC_TitleBarLabel);
    GTMouseDriver::moveTo(os, getWidgetGlobalTopLeftPoint(os, window) + titleLabelRect.center());
    GTMouseDriver::click(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "moveWidgetTo"
void GTWidget::moveWidgetTo(U2::U2OpStatus &os, QWidget *window, const QPoint &point){
    //QPoint(window->width()/2,3) - is hack
    GTMouseDriver::moveTo(os, getWidgetGlobalTopLeftPoint(os, window) + QPoint(window->width()/2,3));
    const QPoint p0 = getWidgetGlobalTopLeftPoint(os, window) + QPoint(window->width()/2,3);
    const QPoint p1 = point + QPoint(window->width()/2,3);
    GTMouseDriver::dragAndDrop(os, p0, p1);
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "resizeWidget"
void GTWidget::resizeWidget(U2::U2OpStatus &os, QWidget *widget, const QSize &size) {
    GT_CHECK(NULL != widget, "Widget is NULL");
    QPoint topLeftPos = getWidgetGlobalTopLeftPoint(os, widget);
    for (int i=0; i<5; i++){
        GTMouseDriver::moveTo(os, topLeftPos);
        if (widget->cursor().shape() == Qt::SizeFDiagCursor){
            break;
        }else{
            topLeftPos += QPoint(1,0);
        }
    }
    QPoint newTopLeftPos = topLeftPos + QPoint(widget->frameGeometry().width() - 1, widget->frameGeometry().height() - 1) - QPoint(size.width(), size.height());
    GTMouseDriver::dragAndDrop(os, topLeftPos, newTopLeftPos);
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getWidgetGlobalTopLeftPoint"
QPoint GTWidget::getWidgetGlobalTopLeftPoint(U2::U2OpStatus &os, QWidget *widget) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(NULL != widget, "Widget is NULL", QPoint());
    return (widget->isWindow() ? widget->pos() : widget->parentWidget()->mapToGlobal(QPoint(0, 0)));
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} //namespace
