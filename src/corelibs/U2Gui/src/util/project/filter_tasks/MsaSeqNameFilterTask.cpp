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

#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2SafePoints.h>

#include "../ProjectFilterNames.h"

#include "MsaSeqNameFilterTask.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// MsaSeqNameFilterTask
//////////////////////////////////////////////////////////////////////////

MsaSeqNameFilterTask::MsaSeqNameFilterTask(const ProjectTreeControllerModeSettings &settings, const QList<QPointer<Document> > &docs)
    : AbstractProjectFilterTask(settings, ProjectFilterNames::MSA_SEQ_NAME_FILTER_NAME, docs)
{

}

bool MsaSeqNameFilterTask::filterAcceptsObject(GObject *obj) {
    MAlignmentObject *msaObj = qobject_cast<MAlignmentObject *>(obj);
    CHECK(NULL != msaObj, false);

    for (int i = 0, n = msaObj->getNumRows(); i < n; ++i) {
        const MAlignmentRow &row = msaObj->getRow(i);
        if (settings.nameFilterAcceptsString(row.getName())) {
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
/// MsaSeqNameFilterTaskFactory
//////////////////////////////////////////////////////////////////////////

AbstractProjectFilterTask * MsaSeqNameFilterTaskFactory::createNewTask(const ProjectTreeControllerModeSettings &settings,
    const QList<QPointer<Document> > &docs) const
{
    return new MsaSeqNameFilterTask(settings, docs);
}


} // namespace U2
