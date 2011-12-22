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

#ifndef _U2_OPTIONS_PANEL_WIDGET_H_
#define _U2_OPTIONS_PANEL_WIDGET_H_

#include "GroupHeaderImageWidget.h"
#include "GroupOptionsWidget.h"
#include "OptionsPanelGroup.h"

#include <QtGui/QtGui>


namespace U2 {


/**
 * Is used to add vertical scroll bar to the option groups (when needed)
 */
class OptionsScrollArea : public QScrollArea
{
public:
    OptionsScrollArea();

    /** Ensures that the scroll area would have an appropriate width */
    virtual QSize sizeHint() const;
};


/** 
 * Serves as a parent widget for all header image and option widgets.
 * Provides general layout and style of the widgets (but it DOESN'T handle the behavior of selecting a group!).
 */
class OptionsPanelWidget : public QFrame
{
    Q_OBJECT
public:
    /** Constructor. Initializes the layout and style of the widget */
    OptionsPanelWidget();

    /** Creates a new header image widget and owns it by putting it into the layout */
    GroupHeaderImageWidget* createHeaderImageWidget(const QPixmap& image);

    /** Creates a new options widget and owns it by putting into the layout */
    GroupOptionsWidget* createOptionsWidget(const QString& title, QWidget* widget);

    /** Shows/hides the options scroll area widget */
    void openOptionsPanel();
    void closeOptionsPanel();

private:
    /** Layouts */
    QVBoxLayout* optionsLayout;
    QVBoxLayout* groupsLayout;

    OptionsScrollArea* optionsScrollArea;
};


} // namespace

#endif
