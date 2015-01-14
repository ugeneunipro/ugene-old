/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "WelcomePageWidget.h"

#include "WelcomePageMdi.h"

namespace U2 {

WelcomePageMdi::WelcomePageMdi(const QString &title, WelcomePageController *controller)
: MWMDIWindow(title), controller(controller)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);

    widget = new WelcomePageWidget(this, controller);
    l->addWidget(widget);
}

void WelcomePageMdi::updateRecent(const QStringList &recentProjects, const QStringList &recentFiles) {
    widget->updateRecent(recentProjects, recentFiles);
}

bool WelcomePageMdi::isLoaded() const {
    return widget->isLoaded();
}

} // U2
