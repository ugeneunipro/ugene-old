/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "OptionsPanel.h"

#include <U2Core/U2SafePoints.h>


namespace U2 {


OptionsPanel::OptionsPanel(QObject* parent) : QObject(parent)
{
    widget = new OptionsPanelWidget();
}

OptionsPanel::~OptionsPanel()
{
    if (0 == widget->parentWidget()) {
        delete widget;
    }
}


QWidget* OptionsPanel::getMainWidget()
{
    return widget;
}


void OptionsPanel::addGroup(const QPixmap& headerImage, const QString& title, QWidget* _widget)
{
    // Create the widgets to show the group
    GroupHeaderImageWidget* headerImageWidget = widget->createHeaderImageWidget(headerImage);
    GroupOptionsWidget* optionsWidget = widget->createOptionsWidget(title, _widget);

    // Listen for signals from the header image widget
    connect(headerImageWidget, SIGNAL(si_groupHeaderPressed(GroupHeaderImageWidget*, bool)),
        this, SLOT(sl_groupHeaderPressed(GroupHeaderImageWidget*, bool)));

    // Create the group object
    OptionsPanelGroup* group = new OptionsPanelGroup(headerImageWidget, optionsWidget);
    groups.append(group);

    // Hide the options group
    optionsWidget->hide();
}


void OptionsPanel::openGroupByTitle(QString groupTitle)
{
    foreach (OptionsPanelGroup* group, groups)
    {
        if (group->getOptionsWidget()->getTitle() == groupTitle) {
            widget->openOptionsPanel();
            openOptionsGroup(group);
        }
    }
}


void OptionsPanel::sl_groupHeaderPressed(GroupHeaderImageWidget* _headerImageWidget, bool ctrlHold)
{
    // Find the options group and get the corresponding options widget
    OptionsPanelGroup* group = findGroupByHeader(_headerImageWidget);

    // Implement the logic of the groups opening/closing
    if (activeGroups.empty())
    {
        widget->openOptionsPanel();
        openOptionsGroup(group);
        return;
    }

    // There are active groups and Ctrl is pressed down
    if (ctrlHold)
    {
        // The group has already been opened
        if (activeGroups.contains(group))
        {
            // And this is the only opened group
            if (activeGroups.count() == 1)
            {
                widget->closeOptionsPanel();
            }

            // Close the already opened group in any case
            closeOptionsGroup(group);
            return;
        }
        // The group hasn't yet been opened
        else
        {
            openOptionsGroup(group);
            return;
        }
    }

    // There are active groups and Ctrl is not pressed down
    // The only active group is the currently selected one
    if (activeGroups.contains(group) && (activeGroups.count() == 1))
    {
        widget->closeOptionsPanel();
        closeOptionsGroup(group);
        return;
    }
    // There are more than 1 groups opened
    else
    {
        foreach (OptionsPanelGroup* activeGroup, activeGroups)
        {
            closeOptionsGroup(activeGroup);
        }

        openOptionsGroup(group);
    }
}


void OptionsPanel::openOptionsGroup(OptionsPanelGroup* group)
{
    SAFE_POINT((group != 0), "Internal error: options panel group equals to 0.",);

    if (activeGroups.contains(group))
    {
        return;
    }

    GroupOptionsWidget* optionsWidget = group->getOptionsWidget();
    SAFE_POINT((optionsWidget != 0), "Internal error: an options group doesn't contain a widget.",);

    GroupHeaderImageWidget* headerWidget = group->getHeaderImageWidget();
    SAFE_POINT((headerWidget != 0), "Internal error: an options group doesn't contain a header widget.",);

    optionsWidget->show();
    headerWidget->setHeaderSelected();

    activeGroups.append(group);
}


void OptionsPanel::closeOptionsGroup(OptionsPanelGroup* group)
{
    SAFE_POINT((group != 0), "Internal error: options panel group equals to 0.",);

    if (!activeGroups.contains(group))
    {
        return;
    }

    GroupOptionsWidget* optionsWidget = group->getOptionsWidget();
    SAFE_POINT((optionsWidget != 0), "Internal error: an options group doesn't contain a widget.",);

    GroupHeaderImageWidget* headerWidget = group->getHeaderImageWidget();
    SAFE_POINT((headerWidget != 0), "Internal error: an options group doesn't contain a header widget.",);

    optionsWidget->hide();
    headerWidget->setHeaderDeselected();

    activeGroups.removeOne(group);
}


OptionsPanelGroup* OptionsPanel::findGroupByHeader(GroupHeaderImageWidget* _headerImageWidget)
{
    foreach (OptionsPanelGroup* group, groups)
    {
        if (group->getHeaderImageWidget() == _headerImageWidget)
            return group;
    }
    return 0;
}


} // namespace
