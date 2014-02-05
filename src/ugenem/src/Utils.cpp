/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QStringList>

#include "Utils.h"

const QString Utils::SESSION_DB_UGENE_ARG = "--session-db=";

const QString REPORT_FILE_ARG = "-f";
const QString SESSION_DB_FILE_ARG = "-d";
const int MAX_FILE_SIZE = 512000; // 500 Kb

bool Utils::hasReportUrl() {
    return hasArgument(REPORT_FILE_ARG);
}

QString Utils::getReportUrl() {
    return getArgumentValue(REPORT_FILE_ARG);
}

bool Utils::hasDatabaseUrl() {
    return hasArgument(SESSION_DB_FILE_ARG);
}

QString Utils::getDatabaseUrl() {
    return getArgumentValue(SESSION_DB_FILE_ARG);
}

QString Utils::loadReportFromUrl(const QString &url) {
    QFile file(url);
    if (!file.exists()) {
        return "";
    }
    bool opened = file.open(QIODevice::ReadOnly);
    if (!opened) {
        return "";
    }
    QByteArray data = file.read(MAX_FILE_SIZE);
    file.close();
    file.remove();
    return QString::fromUtf8(data);
}

bool Utils::hasArgument(const QString &key) {
    QStringList args = QCoreApplication::arguments();
    int idx = args.indexOf(key);
    if (-1 == idx) {
        return false;
    }
    if ((args.size() - 1) == idx) { // last item
        return false;
    }
    return true;
}

QString Utils::getArgumentValue(const QString &key) {
    if (!hasArgument(key)) {
        return "";
    }
    QStringList args = QCoreApplication::arguments();
    return args[args.indexOf(key) + 1];
}
