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

#ifndef _U2_ANNOT_HIGHLIGHT_WIDGET_FACTORY_H_
#define _U2_ANNOT_HIGHLIGHT_WIDGET_FACTORY_H_

#include <U2Core/global.h>

#include <U2Gui/OPWidgetFactory.h>

#include <U2View/AnnotatedDNAView.h>

#include <QtGui/QPixmap>


namespace U2 {

class U2VIEW_EXPORT AnnotHighlightWidgetFactory : public OPWidgetFactory
{
    Q_OBJECT
public:
    AnnotHighlightWidgetFactory();
    virtual ~AnnotHighlightWidgetFactory(){}

    virtual QWidget* createWidget(GObjectView* objView);

    virtual OPGroupParameters getOPGroupParameters();

private:
    static const QString GROUP_ID;
    static const QString GROUP_ICON_STR;
    static const QString GROUP_TITLE;
};

} // namespace

#endif
