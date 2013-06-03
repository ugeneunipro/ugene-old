/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_OUTPUTFILESWIDGET_H_
#define _U2_OUTPUTFILESWIDGET_H_

#include "TableWidget.h"

namespace U2 {

using namespace Workflow::Monitor;

class OutputFilesWidget : public TableWidget {
    Q_OBJECT
public:
    OutputFilesWidget(const QWebElement &content, Dashboard *parent);

    virtual QList<int> widths();
    virtual QStringList header();
    virtual QList<QStringList> data();

private slots:
    void sl_newOutputFile(const Monitor::FileInfo &info);

private:
    QString createFileButton(const QString &url) const;
    void createFileListButton(const QString &actorId);
    void createFilesButton(const QString &actorId, const QList<Monitor::FileInfo> &files);
    QString createFileSubMenu(const QString &url) const;
    QString createActionsSubMenu(const QString &url, bool fullWidth) const;
    QStringList createRow(const Monitor::FileInfo &info) const;
    void collapse();
    QString id(const QString &actorId) const;
    QString id(const Monitor::FileInfo &info) const;
    void addFileMenu(const Monitor::FileInfo &info);
    QString buttonLabel(int filesCount) const;

private:
    bool collapsed;
};

} // U2

#endif // _U2_OUTPUTFILESWIDGET_H_
