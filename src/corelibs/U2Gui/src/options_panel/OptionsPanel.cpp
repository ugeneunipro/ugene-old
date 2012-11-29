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


OptionsPanel::OptionsPanel(GObjectView* _objView)
    : objView(_objView)
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


void OptionsPanel::addGroup(OPWidgetFactory* factory)
{
    // Create a widget with icon at the right side
    OPGroupParameters groupParameters = factory->getOPGroupParameters();
    GroupHeaderImageWidget* headerImageWidget =
        widget->createHeaderImageWidget(groupParameters.getGroupId(), groupParameters.getIcon());

    headerImageWidget->setObjectName(groupParameters.getGroupId());

    // Listen for signals from the header image widget
    connect(headerImageWidget, SIGNAL(si_groupHeaderPressed(QString, bool)),
        this, SLOT(sl_groupHeaderPressed(QString, bool)));

    // Add the factory
    opWidgetFactories.append(factory);
}


void OptionsPanel::openGroupById(const QString& groupId)
{
    if (OPMainWidgetState_Closed == widget->getState()) {
        widget->openOptionsPanel();
        openOptionsGroup(groupId);
    }
    else {
        openOptionsGroup(groupId);
    }
}


void OptionsPanel::sl_groupHeaderPressed(QString groupId, bool ctrlHold)
{
    OPWidgetFactory* opWidgetFactory = findFactoryByGroupId(groupId);
    SAFE_POINT(NULL != opWidgetFactory,
        QString("Internal error: can't open a group with ID '%1' on the Options Panel.").arg(groupId),);

    // Implement the logic of the groups opening/closing
    //
    if (OPMainWidgetState_Closed == widget->getState()) {
        widget->openOptionsPanel();
        openOptionsGroup(groupId);
        return;
    }
    
    // There are active groups and Ctrl has been pressed down
    if (ctrlHold)
    {
        // The group has already been opened
        if (activeGroupsIds.contains(groupId))
        {
            // And this is the only opened group
            if (1 == activeGroupsIds.count()) {
                widget->closeOptionsPanel();
            }

            // Close the already opened group in any case
            closeOptionsGroup(groupId);
            return;
        }
        // The group hasn't yet been opened
        else
        {
            openOptionsGroup(groupId);
            return;
        }
    }

    // There are active groups and Ctrl is not pressed down
    // The only active group is the currently selected one
    if (activeGroupsIds.contains(groupId) &&
        (1 == activeGroupsIds.count()))
    {
        widget->closeOptionsPanel();
        closeOptionsGroup(groupId);
        return;
    }
    // There are more than 1 groups opened
    else
    {
        foreach (QString groupId, activeGroupsIds) {
            closeOptionsGroup(groupId);
        }
        openOptionsGroup(groupId);
    }
}


void OptionsPanel::openOptionsGroup(const QString& groupId)
{
    if (activeGroupsIds.contains(groupId)) {
        widget->focusOptionsWidget(groupId);
        return;
    }

    OPWidgetFactory* opWidgetFactory = findFactoryByGroupId(groupId);
    SAFE_POINT(NULL != opWidgetFactory,
        QString("Internal error: can't open a group with ID '%1' on the Options Panel.").arg(groupId),);

    GroupHeaderImageWidget* headerWidget = widget->findHeaderWidgetByGroupId(groupId);
    SAFE_POINT(NULL != headerWidget,
        QString("Internal error: can't find a header widget for group '%1'").arg(groupId),);

    OPGroupParameters parameters = opWidgetFactory->getOPGroupParameters();
    widget->createOptionsWidget(groupId, parameters.getTitle(), opWidgetFactory->createWidget(objView));
    headerWidget->setHeaderSelected();
    activeGroupsIds.append(groupId);
}


void OptionsPanel::closeOptionsGroup(const QString& groupId)
{
    if (!activeGroupsIds.contains(groupId)) {
        return;
    }

    OPWidgetFactory* opWidgetFactory = findFactoryByGroupId(groupId);
    SAFE_POINT(NULL != opWidgetFactory,
        QString("Internal error: can't open a group with ID '%1' on the Options Panel.").arg(groupId),);

    GroupHeaderImageWidget* headerWidget = widget->findHeaderWidgetByGroupId(groupId);
    SAFE_POINT(NULL != headerWidget,
        QString("Internal error: can't find a header widget for group '%1'").arg(groupId),);

    widget->deleteOptionsWidget(groupId);
    headerWidget->setHeaderDeselected();
    activeGroupsIds.removeAll(groupId);
}


OPWidgetFactory* OptionsPanel::findFactoryByGroupId(const QString& groupId)
{
    foreach (OPWidgetFactory* factory, opWidgetFactories) {
        OPGroupParameters parameters = factory->getOPGroupParameters();
        if (parameters.getGroupId() == groupId) {
            return factory;
        }
    }

    return NULL;
}


} // namespace
