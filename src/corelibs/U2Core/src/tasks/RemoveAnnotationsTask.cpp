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

#include "RemoveAnnotationsTask.h"

#include "LoadDocumentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/Timer.h>
#include <U2Core/GObjectUtils.h>

namespace U2 {

RemoveAnnotationsTask::RemoveAnnotationsTask( AnnotationTableObject* ao, const QString& gName )
:Task("RemoveAnnotationsTask",TaskFlag_NoRun), aobj(ao), groupName(gName), pos(0)
{

    assert(ao != NULL);
}

void RemoveAnnotationsTask::prepare()
{
    if (aobj == NULL) {
        stateInfo.setError(tr("Annotation object is not valid"));
        return;
    }

    subGroup = aobj->getRootGroup()->getSubgroup(groupName, false);
    if (subGroup == NULL) {
        stateInfo.setError(tr("Subgroup %1 is not found").arg(groupName));
        return;
    }
    
    QSet<Annotation*> annotations;
    subGroup->findAllAnnotationsInGroupSubTree(annotations);
    toDelete = annotations.toList();

    GTIMER(c1,t1,"RemoveAnnotationsTask::report");
    
    if (hasError() || isCanceled() )  {
        return ;
    }
    
    
    if (aobj->isStateLocked()) {
        stateInfo.setDescription(tr("Waiting for object lock released"));
        return ;
    }
    stateInfo.setDescription("");
    
    int size = toDelete.size();
    if (size == 0) {
        return ;
    }

    aobj->removeAnnotationsInGroup(toDelete, subGroup);
}




Task::ReportResult RemoveAnnotationsTask::report() {
    if(aobj->isLocked()) {
        return ReportResult_CallMeAgain;
    }
    return ReportResult_Finished;
}





} // namespace
