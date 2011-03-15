/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "FindAlgorithmTask.h"

#include <U2Core/TextUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/Counter.h>

namespace U2 {

FindAlgorithmTask::FindAlgorithmTask(const FindAlgorithmTaskSettings& s) 
: Task (tr("Find in sequence task"), TaskFlag_None), config(s), currentPos(0)
{
    GCOUNTER(cvar, tvar, "FindAlgorithmTask");
    tpm = Progress_Manual;
    complementRun = false;
    currentPos = s.searchRegion.startPos;
    assert(config.strand == FindAlgorithmStrand_Direct || config.complementTT!=NULL);
}

void FindAlgorithmTask::run() {
    FindAlgorithm::find(dynamic_cast<FindAlgorithmResultsListener*>(this),
        config.proteinTT,
        config.complementTT,
        config.strand,
        config.insDelAlg,
        config.sequence.constData(),
        config.sequence.size(),
        config.searchRegion,
        config.pattern.constData(),
        config.pattern.length(),
        config.singleShot,
        config.maxErr,
        stateInfo.cancelFlag,
        stateInfo.progress,
        currentPos);
}

void FindAlgorithmTask::onResult(const FindAlgorithmResult& r) {
    lock.lock();
    newResults.append(r);
    lock.unlock();
}

QList<FindAlgorithmResult> FindAlgorithmTask::popResults() {
    lock.lock();
    QList<FindAlgorithmResult> res = newResults;
    newResults.clear();
    lock.unlock();
    return res;
}

} //namespace



