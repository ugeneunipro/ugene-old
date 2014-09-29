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

#ifndef _U2_EXPORT_COVERAGE_TASK_H_
#define _U2_EXPORT_COVERAGE_TASK_H_

#include <U2Core/GUrl.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Task.h>

#include "CalculateCoveragePerBaseTask.h"

namespace U2 {

class IOAdapter;

class U2VIEW_EXPORT ExportCoverageSettings {
public:
    ExportCoverageSettings() :
        compress(false),
        exportCoverage(true),
        exportBasesCount(true),
        threshold(0) {}

    QString url;
    bool compress;
    bool exportCoverage;
    bool exportBasesCount;
    int threshold;
};

class U2VIEW_EXPORT ExportCoverageTask : public Task {
    Q_OBJECT
public:
    ExportCoverageTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId, const ExportCoverageSettings &settings);

    void prepare();
    ReportResult report();

    const QString &getUrl() const;

private slots:
    void sl_regionIsProcessed(qint64 startPos);

private:
    void writeComments();
    void exportToTabDelimitedPileup(const QVector<CoveragePerBaseInfo> *data);

    const U2DbiRef dbiRef;
    const U2DataId assemblyId;
    const ExportCoverageSettings settings;

    QScopedPointer<IOAdapter> ioAdapter;
    qint64 alreadyProcessed;

    CalculateCoveragePerBaseTask *calculateTask;
};

}   // namespace U2

#endif // _U2_EXPORT_COVERAGE_TASK_H_
