/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GROUP_OPTIONS_WIDGET_H_
#define _U2_GROUP_OPTIONS_WIDGET_H_

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QtGui>
#else
#include <QtWidgets/QtWidgets>
#endif

namespace U2 {

/**
 * Widget with some options shown when a group header has been pressed.
 */
class GroupOptionsWidget : public QWidget
{
public:
    GroupOptionsWidget(const QString& groupId, const QString& title, const QString& documentationPage, QWidget* widget);

    inline const QString& getGroupId() { return groupId; }
    inline static int getWidgetWidth() { return WIDGET_WIDTH; }
    inline const QString& getTitle() { return title; }


private:
    QString     groupId;
    QWidget*    widget;
    QLabel*     titleWidget;
    QString     title;

    QVBoxLayout*        mainLayout;

    static const int TITLE_HEIGHT = 30;
    static const int WIDGET_WIDTH = 200;
};

} // namespace

#endif
