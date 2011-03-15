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


#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/DocumentModel.h>
#include <U2Gui/ObjectViewModel.h>

#include "UIndexViewerFactory.h"
#include "UIndexViewer.h"
#include "UIndexViewerTasks.h"

namespace U2 {

/**************************************************
****************open new view task*****************
*/

OpenUIndexViewerTask::OpenUIndexViewerTask( UIndexObject* obj )
: ObjectViewTask( UIndexViewerFactory::ID ), uindObj( obj ) {
    if( uindObj.isNull() ) {
        stateInfo.setError(tr( "Index object is null" ));
    }
}

OpenUIndexViewerTask::OpenUIndexViewerTask( Document* doc ) 
: ObjectViewTask( UIndexViewerFactory::ID ), uindObj( NULL ) {
    if( NULL == doc || doc->isLoaded() ) {
        stateInfo.setError(tr( "Document is null or loaded" ));
        return;
    }
    documentsToLoad.append( doc );
}

OpenUIndexViewerTask::OpenUIndexViewerTask( UnloadedObject* obj )
: ObjectViewTask( UIndexViewerFactory::ID ), unloadedReference( obj ) {
    assert( obj->getLoadedObjectType() == GObjectTypes::UINDEX );
    documentsToLoad.append( obj->getDocument() );
}

void OpenUIndexViewerTask::open() {
    if( stateInfo.hasErrors() ) {
        return;
    }
    if( uindObj.isNull() ) {
        Document* doc = documentsToLoad.first();
        QList< GObject* > objs = doc->findGObjectByType( GObjectTypes::UINDEX );
        if( objs.isEmpty() ) {
            return;
        }
        uindObj = qobject_cast< UIndexObject* >( objs.first() );
        assert( !uindObj.isNull() );
    }
    viewName = GObjectViewUtils::genUniqueViewName( uindObj->getDocument(), uindObj );
    uiLog.info(tr("Opening index viewer for object %1" ).arg( uindObj->getGObjectName()));
    
    QString viewName = GObjectViewUtils::genUniqueViewName( uindObj->getDocument(), uindObj );
    UIndexViewer* viewer = new UIndexViewer( viewName, uindObj );
    GObjectViewWindow* window = new GObjectViewWindow( viewer, viewName, false );
    MWMDIManager* mdiManager = 	AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow( window );
}

} // U2
