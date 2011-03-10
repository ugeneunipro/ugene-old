
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
