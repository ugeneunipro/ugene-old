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

#ifndef TMPDIRCHECKER_H
#define TMPDIRCHECKER_H

#include <U2Core/Task.h>
#include <QtCore/QDir>

namespace U2 {

class U2CORE_EXPORT TmpDirChecker : public Task {
    Q_OBJECT
public:
    TmpDirChecker();
    void run();
    ReportResult report();
    bool checkPath(QString &path);

    static bool checkWritePermissions(const QString &dirPath);
    static QString getNewFilePath(const QString &dirPath, const QString &baseName);

signals:
    void si_checkFailed(QString path);

private:
    QString commonTempDirPath;
};

} // namespace
#endif // TMPDIRCHECKER_H
