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

#include <U2Core/DocumentModel.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AppContext.h>
#include "AddDocumentTask.h"

namespace U2
{

AddDocumentTask::AddDocumentTask( Document * _d ) :
Task( tr("Add document to the project: %1").arg(_d->getURLString()), TaskFlag_NoRun), d(_d)
{
    d->checkMainThreadModel();
}

Task::ReportResult AddDocumentTask::report() {
    Project * p = AppContext::getProject();
    if( p == NULL ) { // no project is opened
        setError(tr("No project is opened"));
        return ReportResult_Finished;
    }
    
    if( p->isStateLocked() ) {
        return ReportResult_CallMeAgain;
    } else if (d!= NULL) {
        Document* sameURLDoc = p->findDocumentByURL(d->getURL());
        if (sameURLDoc!=NULL) {
            stateInfo.setError(tr("Document is already added to the project %1").arg(d->getURL().getURLString()));
        } else {
            p->addDocument( d );
        }
    } else {
        stateInfo.setError(stateInfo.getError() + tr("Document was removed"));
    }
    return ReportResult_Finished;
}

}//namespace
