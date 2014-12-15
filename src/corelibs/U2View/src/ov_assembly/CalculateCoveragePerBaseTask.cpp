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

#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2AttributeUtils.h>
#include <U2Core/U2CoreAttributes.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SafePoints.h>

#include "CalculateCoveragePerBaseTask.h"

namespace U2 {

CalculateCoveragePerBaseOnRegionTask::CalculateCoveragePerBaseOnRegionTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId, const U2Region &region) :
    Task(tr("Calculate coverage per base for assembly %1 on region (%2, %3)"), TaskFlag_None),
    dbiRef(dbiRef),
    assemblyId(assemblyId),
    region(region),
    results(new QVector<CoveragePerBaseInfo>)
{
    SAFE_POINT_EXT(dbiRef.isValid(), setError(tr("Invalid database reference")), );
    SAFE_POINT_EXT(!assemblyId.isEmpty(), setError(tr("Invalid assembly ID")), );

    DbiConnection con(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    U2AssemblyDbi *assemblyDbi = con.dbi->getAssemblyDbi();
    SAFE_POINT_EXT(NULL != assemblyDbi, setError(tr("Assembly DBI is NULL")), );

    const U2Assembly assembly = assemblyDbi->getAssemblyObject(assemblyId, stateInfo);
    CHECK_OP(stateInfo, );
    setTaskName(getTaskName().arg(assembly.visualName).arg(region.startPos).arg(region.endPos()));
}

CalculateCoveragePerBaseOnRegionTask::~CalculateCoveragePerBaseOnRegionTask() {
    delete results;
}

void CalculateCoveragePerBaseOnRegionTask::run() {
    DbiConnection con(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    U2AssemblyDbi *assemblyDbi = con.dbi->getAssemblyDbi();
    SAFE_POINT_EXT(NULL != assemblyDbi, setError(tr("Assembly DBI is NULL")), );

    results->resize(region.length);

    QScopedPointer<U2DbiIterator<U2AssemblyRead> > readsIterator(assemblyDbi->getReads(assemblyId, region, stateInfo));
    while (readsIterator->hasNext()) {
        const U2AssemblyRead read = readsIterator->next();
        processRead(read);
        CHECK_OP(stateInfo, );
    }
}

const U2Region &CalculateCoveragePerBaseOnRegionTask::getRegion() const {
    return region;
}

QVector<CoveragePerBaseInfo> *CalculateCoveragePerBaseOnRegionTask::takeResult() {
    QVector<CoveragePerBaseInfo> *result = results;
    results = NULL;
    return result;
}

void CalculateCoveragePerBaseOnRegionTask::processRead(const U2AssemblyRead &read) {
    const qint64 startPos = qMax(read->leftmostPos, region.startPos);
    const qint64 endPos = qMin(read->leftmostPos + read->effectiveLen, region.endPos());
    const U2Region regionToProcess = U2Region(startPos, endPos - startPos);

    // we have used effective length of the read, so insertions/deletions are already taken into account
    // cigarString can be longer than needed
    QByteArray cigarString;
    foreach (const U2CigarToken &cigar, read->cigar) {
        cigarString += QByteArray(cigar.count, U2AssemblyUtils::cigar2Char(cigar.op));
    }

    if (read->leftmostPos < regionToProcess.startPos) {
        cigarString = cigarString.mid(regionToProcess.startPos - read->leftmostPos);
    }

    for (int positionOffset = 0, cigarOffset = 0, deletionsCount = 0, insertionsCount = 0; regionToProcess.startPos + positionOffset < regionToProcess.endPos(); positionOffset++) {
        char currentBase = 'N';
        CoveragePerBaseInfo &info = (*results)[regionToProcess.startPos + positionOffset - region.startPos];
        const U2CigarOp cigarOp = nextCigarOp(cigarString, cigarOffset, insertionsCount);
        CHECK_OP(stateInfo, );

        switch(cigarOp) {
        case U2CigarOp_I:
        case U2CigarOp_S:
            // skip the insertion
            continue;
        case U2CigarOp_D:
            // skip the deletion
            deletionsCount++;
            continue;
        case U2CigarOp_N:
            // skip the deletion
            deletionsCount++;
            break;
        default:
            currentBase = read->readSequence[positionOffset - deletionsCount + insertionsCount];
            break;
        }
        info.basesCount[currentBase] = info.basesCount[currentBase] + 1;
        info.coverage++;
    }
}

U2CigarOp CalculateCoveragePerBaseOnRegionTask::nextCigarOp(const QByteArray &cigarString, int &index, int &insertionsCount) {
    QString errString;
    U2CigarOp cigarOp = U2CigarOp_Invalid;

    do {
        SAFE_POINT_EXT(index < cigarString.length(), setError(tr("Cigar string: out of bounds")), U2CigarOp_Invalid);
        cigarOp = U2AssemblyUtils::char2Cigar(cigarString[index], errString);
        if (Q_UNLIKELY(!errString.isEmpty() && !hasError())) {
            setError(errString);
        }
        CHECK_OP(stateInfo, U2CigarOp_Invalid);
        index++;

        if (U2CigarOp_I == cigarOp || U2CigarOp_S == cigarOp) {
            insertionsCount++;
        }
    } while (U2CigarOp_I == cigarOp || U2CigarOp_S == cigarOp || U2CigarOp_P == cigarOp);

    return cigarOp;
}

CalculateCoveragePerBaseTask::CalculateCoveragePerBaseTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId, qint64 maxRegionLength) :
    Task(tr("Calculate coverage per base for assembly %1"), TaskFlags_NR_FOSE_COSC),
    dbiRef(dbiRef),
    assemblyId(assemblyId),
    maxRegionLength(maxRegionLength)
{
    SAFE_POINT_EXT(dbiRef.isValid(), setError(tr("Invalid database reference")), );
    SAFE_POINT_EXT(!assemblyId.isEmpty(), setError(tr("Invalid assembly ID")), );

    DbiConnection con(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    U2AssemblyDbi *assemblyDbi = con.dbi->getAssemblyDbi();
    SAFE_POINT_EXT(NULL != assemblyDbi, setError(tr("Assembly DBI is NULL")), );

    const U2Assembly assembly = assemblyDbi->getAssemblyObject(assemblyId, stateInfo);
    CHECK_OP(stateInfo, );
    setTaskName(getTaskName().arg(assembly.visualName));

    if (maxRegionLength <= 0) {
        maxRegionLength = DEFAULT_MAX_REGION_LENGTH;
    }
}

CalculateCoveragePerBaseTask::~CalculateCoveragePerBaseTask() {
    qDeleteAll(results.values());
}

void CalculateCoveragePerBaseTask::prepare() {
    DbiConnection con(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    U2AttributeDbi *attributeDbi = con.dbi->getAttributeDbi();
    SAFE_POINT_EXT(NULL != attributeDbi, setError(tr("Attribute DBI is NULL")), );

    const U2IntegerAttribute lengthAttribute = U2AttributeUtils::findIntegerAttribute(attributeDbi, assemblyId, U2BaseAttributeName::reference_length, stateInfo);
    CHECK_OP(stateInfo, );
    CHECK_EXT(lengthAttribute.hasValidId(), setError(tr("Can't get the assembly length: attribute is missing")), );

    const qint64 length = lengthAttribute.value;
    SAFE_POINT_EXT(0 < length, setError(tr("Assembly has zero length")), );

    qint64 tasksCount = length / maxRegionLength + (length % maxRegionLength > 0 ? 1 : 0);
    for (qint64 i = 0; i < tasksCount; i++) {
        const U2Region region(i * maxRegionLength, (i == tasksCount - 1 ? length % maxRegionLength : maxRegionLength));
        addSubTask(new CalculateCoveragePerBaseOnRegionTask(dbiRef, assemblyId, region));
    }
}

QList<Task *> CalculateCoveragePerBaseTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> res;
    CHECK_OP(stateInfo, res);

    CalculateCoveragePerBaseOnRegionTask *calculateTask = qobject_cast<CalculateCoveragePerBaseOnRegionTask *>(subTask);
    SAFE_POINT_EXT(NULL != calculateTask, setError(tr("An unexpected subtask")), res);

    results.insert(calculateTask->getRegion().startPos, calculateTask->takeResult());
    emit si_regionIsProcessed(calculateTask->getRegion().startPos);

    return res;
}

bool CalculateCoveragePerBaseTask::isResultReady(qint64 startPos) const {
    return results.contains(startPos);
}

bool CalculateCoveragePerBaseTask::areThereUnprocessedResults() const {
    return !results.isEmpty();
}

QVector<CoveragePerBaseInfo> *CalculateCoveragePerBaseTask::takeResult(qint64 startPos) {
    QVector<CoveragePerBaseInfo> *result = results.value(startPos, NULL);
    results.remove(startPos);
    return result;
}

}   // namespace U2
