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

#ifndef _U2_CALCULATE_COVERAGE_PER_BASE_TASK_H_
#define _U2_CALCULATE_COVERAGE_PER_BASE_TASK_H_

#include <U2Core/U2Assembly.h>
#include <U2Core/Task.h>
#include <U2Core/U2Type.h>

namespace U2 {

class CoveragePerBaseInfo {
public:
    CoveragePerBaseInfo() :
        coverage(0) {}

    int coverage;
    QMap<char, int> basesCount;
};

class CalculateCoveragePerBaseOnRegionTask : public Task {
    Q_OBJECT
public:
    CalculateCoveragePerBaseOnRegionTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId, const U2Region &region);
    ~CalculateCoveragePerBaseOnRegionTask();

    void run();

    const U2Region &getRegion() const;
    QVector<CoveragePerBaseInfo> *takeResult();

private:
    void processRead(const U2AssemblyRead &read);
    U2CigarOp nextCigarOp(const QByteArray &cigarString, int &index, int &insertionsCount);

    const U2DbiRef dbiRef;
    const U2DataId assemblyId;
    const U2Region region;
    QVector<CoveragePerBaseInfo> *results;
};

class CalculateCoveragePerBaseTask : public Task {
    Q_OBJECT
public:
    CalculateCoveragePerBaseTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId);
    ~CalculateCoveragePerBaseTask();

    void prepare();
    QList<Task *> onSubTaskFinished(Task *subTask);

    bool isResultReady(qint64 startPos) const;
    bool areThereUnprocessedResults() const;
    QVector<CoveragePerBaseInfo> *takeResult(qint64 startPos);

signals:
    void si_regionIsProcessed(qint64 startPos);

private slots:

private:
    const U2DbiRef dbiRef;
    const U2DataId assemblyId;
    QHash<qint64, QVector<CoveragePerBaseInfo> *> results;

    static const qint64 MAX_REGION_LENGTH = 100000;
};

}   // namespace U2

#endif // _U2_CALCULATE_COVERAGE_PER_BASE_TASK_H_
