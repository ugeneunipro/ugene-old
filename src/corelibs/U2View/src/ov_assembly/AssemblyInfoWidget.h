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

#ifndef __ASSEMBLY_INFO_WIDGET_H__
#define __ASSEMBLY_INFO_WIDGET_H__

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <U2Gui/OPWidgetFactory.h>

#include <QtGui/QtGui>


namespace U2 {

class AssemblyBrowser;

class AssemblyInfoWidget : public QWidget {
    Q_OBJECT
public:
    AssemblyInfoWidget(AssemblyBrowser * browser, QWidget *p = NULL);
};

class U2VIEW_EXPORT AssemblyInfoWidgetFactory : public OPWidgetFactory
{
    Q_OBJECT
public:
    AssemblyInfoWidgetFactory();
    virtual ~AssemblyInfoWidgetFactory(){}

    virtual QWidget* createWidget(GObjectView* objView);

    virtual OPGroupParameters getOPGroupParameters();

private:
    static const QString GROUP_ID;
    static const QString GROUP_ICON_STR;
    static const QString GROUP_TITLE;
};

} // namespace

#endif // #ifndef __ASSEMBLY_INFO_WIDGET_H__
