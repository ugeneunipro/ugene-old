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

#include "GTWidget.h"
#include "GTMouseDriver.h"
#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QComboBox>
#else
#include <QtWidgets/QWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QComboBox>
#endif

// TODO: this is a fast fix
#include <U2View/ADVSingleSequenceWidget.h>

namespace U2 {

#define GT_CLASS_NAME "GTWidget"

#define GT_METHOD_NAME "click"
void GTWidget::click(U2OpStatus &os, QWidget *w, Qt::MouseButton mouseButton, QPoint p, bool safe) {

    GTGlobals::sleep(100);
    GT_CHECK(w != NULL, "widget is NULL");
    GT_CHECK(w->isEnabled() == true, "widget " + w->objectName() + "is not enabled");

    if (p.isNull()) {
        p = w->rect().center();

        // TODO: this is a fast fix
        QWidget *adv = qobject_cast<ADVSingleSequenceWidget*>(w);
        if (adv) {
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
void GTWidget::setFocus(U2OpStatus &os, QWidget *w) {

    GT_CHECK(w != NULL, "widget is NULL");

    GTWidget::click(os, w);
    GTGlobals::sleep(1000);

    if(!qobject_cast<QComboBox*>(w)){
        GT_CHECK(w->hasFocus(), "Can't set focus on widget " + w->objectName());
    }

}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findWidget"
QWidget* GTWidget::findWidget(U2OpStatus &os, const QString &widgetName, QWidget *parentWidget, const GTGlobals::FindOptions& options) {
    Q_UNUSED(os);

    if (parentWidget == NULL) {
        parentWidget = AppContext::getMainWindow()->getQMainWindow();
    }
    QWidget* widget = parentWidget->findChild<QWidget*>(widgetName);

    if (options.failIfNull) {
        GT_CHECK_RESULT(widget != NULL, "Widget " + widgetName + " not found", NULL);
    }

    return widget;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getWidgetCenter"
QPoint GTWidget::getWidgetCenter(U2OpStatus &os, QWidget *w){
    Q_UNUSED(os)
    return w->mapToGlobal(w->rect().center());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findButtonByText"
QAbstractButton* GTWidget::findButtonByText(U2OpStatus &os, const QString &text, QWidget *parentWidget, const GTGlobals::FindOptions& options) {

    if (parentWidget == NULL) {
        parentWidget = AppContext::getMainWindow()->getQMainWindow();
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
        GT_CHECK_RESULT(!foundButtonList.count()==0, QString("button with this text <%1> not found").arg(text), NULL);
    }

    return foundButtonList.takeFirst();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findWidget"
void GTWidget::getAllWidgetsInfo(U2OpStatus &os, QWidget *parent){
    if(parent == NULL){
        parent = qobject_cast<QWidget*>(AppContext::getMainWindow()->getQMainWindow());
    }

    QList<QWidget*> list= parent->findChildren<QWidget*>();
    QString actStr;

    foreach(QWidget* act, list){
        actStr.append(act->objectName()+ "  " + act->metaObject()->className() + "  " + QString("%1").arg(act->isVisible()) + "\n");
    }
    CHECK_SET_ERR(false, actStr);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getColor"
QColor GTWidget::getColor(U2OpStatus &os, QWidget *w, const QPoint &p) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(NULL != w, "Widget is NULL", QColor());
    QPixmap pixmap = QPixmap::grabWidget(w, w->rect());
    QImage img = pixmap.toImage();
    QRgb rgb = img.pixel(p);
    QColor result = QColor(rgb);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickLabelLink"
void GTWidget::clickLabelLink(U2OpStatus &os, QWidget *label, int step){

    QRect r = label->rect();

    int left = r.left();
    int right = r.right();
    int top = r.top();
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

#undef GT_CLASS_NAME

} //namespace
