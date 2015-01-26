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

#include <U2Core/DNASequenceObject.h>
#include <U2Core/L10n.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>

#include "ConvertPrimersToSequenceTask.h"

namespace U2 {

ConvertPrimersToSequenceTask::ConvertPrimersToSequenceTask(const QList<Primer> &primers, const U2DbiRef &dbiRef ) :
    Task(tr("Convert primers to sequences"), TaskFlag_None),
    primers(primers),
    dbiRef(dbiRef),
    dbiSequences(dbiRef, stateInfo)
{
    SAFE_POINT_EXT(!primers.isEmpty(), setError(L10N::badArgument("empty primers list")), );
    SAFE_POINT_EXT(dbiRef.isValid(), setError(L10N::badArgument("database reference")), );
}

void ConvertPrimersToSequenceTask::run() {
    U2SequenceImporter importer;
    importer.startSequence(dbiRef, U2ObjectDbi::ROOT_FOLDER);
}

QList<U2SequenceObject *> ConvertPrimersToSequenceTask::takeObjects() {
    QList<U2SequenceObject *> result;
    while (!sequences.objects.isEmpty()) {
        const U2DataId sequenceId = sequences.objects.takeFirst();
        result << new U2SequenceObject(id2name[sequenceId], U2EntityRef(dbiRef, sequenceId));
    }
    return result;
}

}   // namespace U2
