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

#include "OptionsPanelWidget.h"

#include <U2Core/U2SafePoints.h>


namespace U2 {


OptionsScrollArea::OptionsScrollArea() {
    setObjectName("OP_SCROLL_AREA");
    setWidgetResizable(true);
    setStyleSheet("QWidget#OP_SCROLL_AREA { "
        "border-style: none;"
        " }");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    hide();
}


QSize OptionsScrollArea::sizeHint() const
{
    int BORDERS_APPROX_SIZE = 15;
    return QSize(GroupOptionsWidget::getWidgetWidth() + BORDERS_APPROX_SIZE, 0);
}


OptionsPanelWidget::OptionsPanelWidget()
{
    setObjectName("OP_MAIN_WIDGET");

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    setStyleSheet("QWidget#OP_MAIN_WIDGET { "
        "border-style: solid;"
        "border-color: palette(shadow);"
        "border-top-width: 1px;"
        "border-bottom-width: 1px;"
        " }");

    // Initialize the layout of the whole widget
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Initialize the layout of the options panel
    optionsLayout = new QVBoxLayout();
    optionsLayout->setContentsMargins(0, 0, 0, 0);
    optionsLayout->setSpacing(0);

    // Initialize the layout of the groups panel
    // External groups layout is used to add a spacer below the image headers
    groupsLayout = new QVBoxLayout();
    groupsLayout->setContentsMargins(0, 60, 0, 0);
    groupsLayout->setSpacing(0);

    QVBoxLayout* externalGroupsLayout = new QVBoxLayout();
    externalGroupsLayout->setContentsMargins(0, 0, 0, 0);
    externalGroupsLayout->setSpacing(0);

    QSpacerItem* spacer = new QSpacerItem(0, 0,
        QSizePolicy::Minimum, QSizePolicy::Expanding);
    externalGroupsLayout->addLayout(groupsLayout);
    externalGroupsLayout->addItem(spacer);

    // The widget used to add decoration and scroll to the options widgets
    optionsScrollArea = new OptionsScrollArea();

    QWidget* optionsWidget = new QWidget();
    optionsWidget->setObjectName("OP_OPTIONS_WIDGET");
    optionsWidget->setLayout(optionsLayout);

    optionsWidget->setStyleSheet("QWidget#OP_OPTIONS_WIDGET { "
        "background: palette(window);"
        "border-style: none;"
        "border-color: palette(shadow);"
        " }");

    optionsScrollArea->setWidget(optionsWidget);

    // The widget is used to add additional decoration to the groups panel
    QWidget* groupsWidget = new QWidget();
    groupsWidget->setLayout(externalGroupsLayout);

    groupsWidget->setStyleSheet(
        "background: palette(mid);"
        "border-style: solid;"
        "border-left-width: 1px;"
        "border-top-width: 0px;"
        "border-right-width: 1px;"
        "border-bottom-width: 0px;"
        "border-color: palette(shadow);");

    // This prevents blinking when the options panel has been opened/closed
    mainLayout->setAlignment(Qt::AlignRight);

    // Set the layout of the whole widget
    mainLayout->addWidget(optionsScrollArea);
    mainLayout->addWidget(groupsWidget);
    setLayout(mainLayout);

    // Init the state
    opMainWidgetState = OPMainWidgetState_Closed;
}


GroupHeaderImageWidget* OptionsPanelWidget::createHeaderImageWidget(const QString& groupId, const QPixmap& image)
{
    GroupHeaderImageWidget* widget = new GroupHeaderImageWidget(groupId, image);

    // Add widget to the layout and "parent" it
    groupsLayout->addWidget(widget);

    headerWidgets.append(widget);

    return widget;
}


GroupOptionsWidget* OptionsPanelWidget::createOptionsWidget(const QString& groupId, const QString& title, QWidget* _widget)
{
    GroupOptionsWidget* widget = new GroupOptionsWidget(groupId, title, _widget);

    // Add widget to the layout and "parent" it
    optionsLayout->addWidget(widget);

    optionsWidgets.append(widget);

    widget->setFocus();

    return widget;
}


void OptionsPanelWidget::openOptionsPanel()
{
    optionsScrollArea->show();
    opMainWidgetState = OPMainWidgetState_Opened;
}


void OptionsPanelWidget::closeOptionsPanel()
{
    optionsScrollArea->hide();
    opMainWidgetState = OPMainWidgetState_Closed;
}


GroupHeaderImageWidget* OptionsPanelWidget::findHeaderWidgetByGroupId(const QString& groupId)
{
    foreach (GroupHeaderImageWidget* widget, headerWidgets) {
        if (widget->getGroupId() == groupId) {
            return widget;
        }
    }

    return NULL;
}


GroupOptionsWidget* OptionsPanelWidget::findOptionsWidgetByGroupId(const QString& groupId)
{
    foreach (GroupOptionsWidget* widget, optionsWidgets) {
        if (widget->getGroupId() == groupId) {
            return widget;
        }
    }

    return NULL;
}


void OptionsPanelWidget::deleteOptionsWidget(const QString& groupId)
{
    GroupOptionsWidget* optionsWidget = findOptionsWidgetByGroupId(groupId);
    SAFE_POINT(NULL != optionsWidget,
        QString("Internal error: failed to find an options widget for group '%1' to delete it.").arg(groupId),);

    optionsLayout->removeWidget(optionsWidget);
    delete optionsWidget;
    optionsWidgets.removeAll(optionsWidget);
}


} // namespace
