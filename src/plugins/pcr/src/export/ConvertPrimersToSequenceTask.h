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

#ifndef _U2_CONVERT_PRIMERS_TO_SEQUENCE_TASK_H_
#define _U2_CONVERT_PRIMERS_TO_SEQUENCE_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/U2DbiUtils.h>

#include "Primer.h"

namespace U2 {

class U2SequenceObject;

class ConvertPrimersToSequenceTask : public Task {
public:
    ConvertPrimersToSequenceTask(const QList<Primer> &primers, const U2DbiRef &dbiRef);

    void run();

    QList<U2SequenceObject *> takeObjects();

private:
    const QList<Primer> primers;
    const U2DbiRef &dbiRef;

    TmpDbiObjects sequences;
    QHash<U2DataId, QString> id2name;
};

}   // namespace U2

#endif // _U2_CONVERT_PRIMERS_TO_SEQUENCE_TASK_H_
