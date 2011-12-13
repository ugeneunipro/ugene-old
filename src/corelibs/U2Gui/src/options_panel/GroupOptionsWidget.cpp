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

#include "GroupOptionsWidget.h"


namespace U2 {


GroupOptionsWidget::GroupOptionsWidget(const QString& title, QWidget* _widget)
    : widget(_widget)
{
    titleWidget = new QLabel(title);
    titleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleWidget->setMinimumWidth(WIDGET_WIDTH);

    titleWidget->setStyleSheet(
        "background: palette(midlight);"
        "border-style: solid;"
        "border-width: 1px;"
        "border-color: palette(mid);"
        "padding: 2px;"
        "margin: 5px;");

    widget->setContentsMargins(10, 5, 5, 5);
    widget->setStyleSheet("border-style: none;");

    // Layout and "parent" the widgets
    mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(widget);
    setLayout(mainLayout);
}

} // namespace
