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

#include "OptionsPanelWidget.h"

#include <U2Core/U2SafePoints.h>


namespace U2 {


OptionsScrollArea::OptionsScrollArea(QObject* parent)
{
    setWidgetResizable(true);
    setStyleSheet("border-style: none;");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    hide();
}


QSize OptionsScrollArea::sizeHint() const
{
    int BORDERS_APPROX_SIZE = 15;
    return QSize(GroupOptionsWidget::getWidgetWidth() + BORDERS_APPROX_SIZE, 0);
}


OptionsPanelWidget::OptionsPanelWidget(QObject* parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    setStyleSheet(
        "border-style: solid;"
        "border-color: palette(shadow);"
        "border-top-width: 1px;"
        "border-bottom-width: 1px;");

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
    optionsWidget->setLayout(optionsLayout);

    optionsWidget->setStyleSheet(
        "background: palette(window);"
        "border-style: none;"
        "border-color: palette(shadow);");

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
}


GroupHeaderImageWidget* OptionsPanelWidget::createHeaderImageWidget(const QPixmap& image)
{
    GroupHeaderImageWidget* widget = new GroupHeaderImageWidget(image);

    // Add widget to the layout and "parent" it
    groupsLayout->addWidget(widget);

    return widget;
}


GroupOptionsWidget* OptionsPanelWidget::createOptionsWidget(const QString& title, QWidget* _widget)
{
    GroupOptionsWidget* widget = new GroupOptionsWidget(title, _widget);

    // Add widget to the layout and "parent" it
    optionsLayout->addWidget(widget);

    return widget;
}


void OptionsPanelWidget::openOptionsPanel()
{
    optionsScrollArea->show();
}


void OptionsPanelWidget::closeOptionsPanel()
{
    optionsScrollArea->hide();
}


} // namespace
