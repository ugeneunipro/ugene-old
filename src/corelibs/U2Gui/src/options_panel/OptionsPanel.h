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

#ifndef _U2_OPTIONS_PANEL_H_
#define _U2_OPTIONS_PANEL_H_

#include "OptionsPanelGroup.h"
#include "OptionsPanelWidget.h"

#include <U2Core/global.h>


namespace U2 {

/**
 * Provides interface between instances that want to use an options panel and
 * low-level implementation of the panel (including widgets, their styles, etc.)
 * To use this class add the Options Panel's main widget to a layout and add the required groups.
 */
class U2GUI_EXPORT OptionsPanel : public QObject
{
    Q_OBJECT
public:
    OptionsPanel(QObject* parent = 0);

    /** Add a new options panel group instance and corresponding widgets*/
    void addGroup(const QPixmap& headerImage, const QString& title, QWidget* optionsWidget);

    /** Returns the main Options Panel widget */
    QWidget* getMainWidget();


public slots:
    /** Catches signals that a group header has been pressed
        and implements the behavior of groups selection (i.e. one group at a time) */
    void sl_groupHeaderPressed(GroupHeaderImageWidget* headerImageWidget, bool ctrlHold);


private:
    /** Shows the options widget and adds the group to the active groups */
    void openOptionsGroup(OptionsPanelGroup*);

    /** Hides the options widget and removes the group from the active groups */
    void closeOptionsGroup(OptionsPanelGroup*);

    /** Returns the group object or zero if the group is not found */
    OptionsPanelGroup* findGroupByHeader(GroupHeaderImageWidget*);

    /** Groups to show on the Options Panel */
    QList<OptionsPanelGroup*> groups;

    /** Opened groups */
    QList<OptionsPanelGroup*> activeGroups;

    /** The widget that displays options groups */
    OptionsPanelWidget* widget;
};

} // namespace


#endif
