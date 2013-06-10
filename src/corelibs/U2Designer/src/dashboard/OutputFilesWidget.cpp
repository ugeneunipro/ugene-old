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

#include <QDir>
#include <QFileInfo>
#include <QTableView>
#include <QVBoxLayout>

#include <U2Core/U2SafePoints.h>

#include "OutputFilesWidget.h"

namespace U2 {

static const int MAX_FILES_COUNT = 10;

OutputFilesWidget::OutputFilesWidget(const QWebElement &content, Dashboard *parent)
: TableWidget(content, parent), collapsed(false)
{
    createTable();

    connect(dashboard->monitor(), SIGNAL(si_newOutputFile(const Monitor::FileInfo &)),
        SLOT(sl_newOutputFile(const Monitor::FileInfo &)));
}

void OutputFilesWidget::sl_newOutputFile(const Monitor::FileInfo &info) {
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, );
    if (m->getOutputFiles().size() > MAX_FILES_COUNT && !collapsed) {
        collapse();
        return;
    }
    if (collapsed && rows.contains(id(info))) {
        addFileMenu(info);
    } else {
        addRow(id(info), createRow(info));
    }
}

QList<int> OutputFilesWidget::widths() {
    return QList<int>() << 50 << 50;
}

QStringList OutputFilesWidget::header() {
    return QStringList() << tr("File") << tr("Producer");
}

QList<QStringList> OutputFilesWidget::data() {
    QList<QStringList> result;
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, result);
    foreach (const Monitor::FileInfo &file, m->getOutputFiles()) {
        QStringList row;
        row << file.url; // id
        row << createRow(file);
        result << row;
    }
    return result;
}

QString OutputFilesWidget::createActionsSubMenu(const QString &url, bool fullWidth) const {
    QFileInfo info(url);
    return QString(
        "<ul class=\"dropdown-menu %1\">"
            "<li><a href=\"#\" onclick=\"agent.openByOS('%2')\">%3</a></li>"
            "<li><a href=\"#\" onclick=\"agent.openByOS('%4')\">%5</a></li>"
        "</ul>"
        )
        .arg(fullWidth ? "full-width" : "")
        .arg(info.dir().absolutePath())
        .arg(tr("Open containing directory"))
        .arg(url)
        .arg(tr("Open by operating system"));
}

static const int MAX_LEN = 25;
static QString fileName(const QString &url) {
    QFileInfo info(url);
    QString name = info.fileName();
    if (name.length() > MAX_LEN) {
        return name.left(MAX_LEN - 3) + "...";
    }
    return name;
}

QString OutputFilesWidget::createFileButton(const QString &url) const {
    return QString(
        "<div class=\"file-button-ctn\">"
        "<div class=\"btn-group full-width file-btn-group\">"
            "<button class=\"btn full-width long-text\" onclick=\"agent.openUrl('%1')\" title=\"%1\">%2</button>"
            "<button class=\"btn dropdown-toggle\" data-toggle=\"dropdown\">"
                "<span class=\"caret\"></span>"
            "</button>"
            "%3"
        "</div>"
        "</div>"
    )
    .arg(url)
    .arg(fileName(url))
    .arg(createActionsSubMenu(url, true));
}

QString OutputFilesWidget::createFileSubMenu(const QString &url) const {
    return QString(
        "<li class=\"file-sub-menu dropdown-submenu left-align\">"
        "<a tabindex=\"-1\" href=\"#\" onclick=\"agent.openUrl('%1')\" title=\"%1\">%2</a>"
            "%3"
        "</li>"
    )
    .arg(url)
    .arg(fileName(url))
    .arg(createActionsSubMenu(url, false));
}

QStringList OutputFilesWidget::createRow(const Monitor::FileInfo &info) const {
    QStringList result;
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, result);

    result << createFileButton(info.url);
    result << wrapLongText(m->actorName(info.actor));
    return result;
}

void OutputFilesWidget::createFileListButton(const QString &actorId) {
    QString filesButton = QString(
        "<div class=\"btn-group full-width\">"
            "<button class=\"files-btn btn dropdown-toggle full-width\" data-toggle=\"dropdown\" href=\"#\">"
                "%1"
            "</button>"
            "<ul class=\"files-menu dropdown-menu full-width\"/>"
        "</div>")
        .arg(buttonLabel(0));

    updateRow(id(actorId), QStringList() << filesButton << wrapLongText(dashboard->monitor()->actorName(actorId)));
}

void OutputFilesWidget::createFilesButton(const QString &actorId, const QList<Monitor::FileInfo> &files) {
    createFileListButton(actorId);
    foreach (const Monitor::FileInfo &info, files) {
        addFileMenu(info);
    }
}

void OutputFilesWidget::collapse() {
    collapsed = true;
    createTable();

    QMap< QString, QList<Monitor::FileInfo> > filesMap =
        MonitorUtils::filesByActor(dashboard->monitor());
    foreach (const QString &actorId, filesMap.keys()) {
        QList<Monitor::FileInfo> files = filesMap[actorId];
        if (1 == files.size()) {
            addRow(id(files.first()), createRow(files.first()));
        } else {
            createFilesButton(actorId, files);
        }
    }
}

static bool isFileButton(const QWebElement &row) {
    return row.findAll(".file-button-ctn").count() > 0;
}

static int filesCount(const QWebElement &menu) {
    return menu.findAll(".file-sub-menu").count();
}

void OutputFilesWidget::addFileMenu(const Monitor::FileInfo &info) {
    SAFE_POINT(collapsed, "Not collapsed mode", );
    SAFE_POINT(rows.contains(id(info)), "The menu is not created", );

    QWebElement row = rows[id(info)];
    if (isFileButton(row)) {
        QList<Monitor::FileInfo> files = MonitorUtils::filesByActor(dashboard->monitor())[info.actor];
        createFilesButton(info.actor, files);
    } else {
        QWebElement button = row.findFirst(".files-btn");
        QWebElement menu = row.findFirst(".files-menu");
        int count = filesCount(menu) + 1;
        button.setInnerXml(buttonLabel(count));
        menu.appendInside(createFileSubMenu(info.url));
    }
}

QString OutputFilesWidget::buttonLabel(int filesCount) const {
    return QString("%1 %2 %3").arg(filesCount).arg(tr("files")).arg("<span class=\"caret\"></span>");
}

QString OutputFilesWidget::id(const QString &actorId) const {
    return ":;" + actorId + ":;";
}

QString OutputFilesWidget::id(const Monitor::FileInfo &info) const {
    if (collapsed) {
        return id(info.actor);
    }
    return info.url;
}

} // U2
