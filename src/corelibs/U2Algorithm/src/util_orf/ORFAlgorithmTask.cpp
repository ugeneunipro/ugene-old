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

#include "ORFAlgorithmTask.h"

#include <U2Core/TextUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/Counter.h>

namespace U2 {

#define MAX_RESULTS 100000

ORFFindTask::ORFFindTask(const ORFAlgorithmSettings& s, const QByteArray& seq) 
: Task (tr("ORF find"), TaskFlag_None), config(s), sequence(seq)
{
    GCOUNTER( cvar, tvar, "ORFFindTask" );
    tpm = Progress_Manual;
    assert(config.proteinTT && config.proteinTT->isThree2One());
}

void ORFFindTask::run() {
    ORFFindAlgorithm::find(dynamic_cast<ORFFindResultsListener*>(this),
        config,
        sequence.constData(),
        sequence.size(),
        stateInfo.cancelFlag,
        stateInfo.progress);
}

void ORFFindTask::onResult(const ORFFindResult& r) {
    QMutexLocker locker(&lock);
    if (newResults.size() > MAX_RESULTS) {
        if (!isCanceled()) {
            stateInfo.setError(  tr("Number of results exceeds %1").arg(MAX_RESULTS) );
            cancel();
        }
        return;
    }
    assert((r.region.length + r.joinedRegion.length) % 3 == 0);
    newResults.append(r);
}

QList<ORFFindResult> ORFFindTask::popResults() {
    lock.lock();
    QList<ORFFindResult> res = newResults;
    newResults.clear();
    lock.unlock();
    return res;
}

} //namespace



