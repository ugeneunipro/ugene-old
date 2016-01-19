/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_WELCOME_PAGE_WIDGET_H_
#define _U2_WELCOME_PAGE_WIDGET_H_

#include "utils/MultilingualHtmlView.h"

namespace U2 {

class WelcomePageController;

class WelcomePageWidget : public MultilingualHtmlView {
    Q_OBJECT
public:
    WelcomePageWidget(QWidget *parent, WelcomePageController *controller);

    void updateRecent(const QStringList &recentProjects, const QStringList &recentFiles);
    bool eventFilter(QObject *watched, QEvent *event);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

private slots:
    void sl_loaded(bool ok);

private:
    void updateRecentFilesContainer(const QString &id, const QStringList &files, const QString &message);
    void addController();

private:
    bool loaded;
    WelcomePageController *controller;
};

} // U2

#endif // _U2_WELCOME_PAGE_WIDGET_H_
