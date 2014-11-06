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
    enum Format {
        Histogram = 0,
        PerBase = 1,
        Bedgraph = 2
    };

    ExportCoverageSettings() :
        compress(false),
        exportCoverage(true),
        exportBasesCount(true),
        threshold(0) {}

    static QString getFormat(Format format);
    static QString getFormatExtension(Format format);

    QString url;
    bool compress;
    bool exportCoverage;
    bool exportBasesCount;
    int threshold;

    static const QString HISTOGRAM;
    static const QString PER_BASE;
    static const QString BEDGRAPH;
    static const QString HISTOGRAM_EXTENSION;
    static const QString PER_BASE_EXTENSION;
    static const QString BEDGRAPH_EXTENSION;
    static const QString COMPRESSED_EXTENSION;
};

class U2VIEW_EXPORT ExportCoverageTask : public Task {
    Q_OBJECT
public:
    ExportCoverageTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId, const ExportCoverageSettings &settings, TaskFlags flags = TaskFlags_NR_FOSE_COSC);

    void prepare();
    ReportResult report();

    const QString &getUrl() const;

private slots:
    void sl_regionIsProcessed(qint64 startPos);

protected:
    virtual void processRegion(qint64 startPos) = 0;
    void write(const QByteArray &dataToWrite);

    const U2DbiRef dbiRef;
    const U2DataId assemblyId;
    const ExportCoverageSettings settings;

    QString assemblyName;
    CalculateCoveragePerBaseTask *calculateTask;
    QScopedPointer<IOAdapter> ioAdapter;
    qint64 alreadyProcessed;

    static const QByteArray SEPARATOR;
};

class U2VIEW_EXPORT ExportCoverageHistogramTask : public ExportCoverageTask {
public:
    ExportCoverageHistogramTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId, const ExportCoverageSettings &settings);

    void run();

protected:
    void processRegion(qint64 startPos);

private:
    QByteArray toByteArray(int coverage, qint64 assemblyLength) const;

    QMap<int, qint64> histogramData;
};

class U2VIEW_EXPORT ExportCoveragePerBaseTask : public ExportCoverageTask {
public:
    ExportCoveragePerBaseTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId, const ExportCoverageSettings &settings);

    void prepare();

protected:
    void processRegion(qint64 startPos);

private:
    void writeHeader();
    QByteArray toByteArray(const CoveragePerBaseInfo &info, int pos) const;         // pos - 1-based position
    void writeResult(const QVector<CoveragePerBaseInfo> *data);
};

class U2VIEW_EXPORT ExportCoverageBedgraphTask : public ExportCoverageTask {
public:
    ExportCoverageBedgraphTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId, const ExportCoverageSettings &settings);

    void prepare();
    QList<Task *> onSubTaskFinished(Task *subTask);

protected:
    void processRegion(qint64 startPos);

private:
    void writeHeader();
    QByteArray toByteArray() const;         // startpos - 0-based position, endpos - next after real end
    void writeRegion();

    QPair<U2Region, int> currentCoverage;
};

}   // namespace U2

#endif // _U2_EXPORT_COVERAGE_TASK_H_
