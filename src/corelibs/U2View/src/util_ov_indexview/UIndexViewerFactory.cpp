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
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>

#include <U2Core/SelectionUtils.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/UIndexObject.h>
#include <U2Core/UnloadedObject.h>

#include <U2Core/Task.h>

#include "UIndexViewerFactory.h"
#include "UIndexViewerTasks.h"

namespace U2 {

const GObjectViewFactoryId UIndexViewerFactory::ID( "uindex_viewer" );

UIndexViewerFactory::UIndexViewerFactory() : GObjectViewFactory( ID, tr( "Index viewer" ) ) {
}

bool UIndexViewerFactory::canCreateView(const MultiGSelection& multiSelection) {
    bool hasUIndexDocs = !SelectionUtils::findDocumentsWithObjects( GObjectTypes::UINDEX, &multiSelection,
                            UOF_LoadedAndUnloaded, true ).isEmpty();
    return hasUIndexDocs;
}

bool UIndexViewerFactory::isStateInSelection(const MultiGSelection& mSelection, const QVariantMap& stData ) {
    Q_UNUSED(mSelection);Q_UNUSED(stData);
    return false;
}

// similar to MSAEditorFactory::createViewTask
Task* UIndexViewerFactory::createViewTask(const MultiGSelection& mSel, bool single ) {
    QList<GObject*> indObjs = SelectionUtils::findObjects( GObjectTypes::UINDEX, &mSel, UOF_LoadedAndUnloaded );
    QSet<Document*> docsWithUInd = SelectionUtils::findDocumentsWithObjects( GObjectTypes::UINDEX, &mSel,
                                        UOF_LoadedAndUnloaded, false );
    QList< OpenUIndexViewerTask* > resTasks;
    
    foreach(Document* doc, docsWithUInd ) {
        QList<GObject*> docObjs = doc->findGObjectByType( GObjectTypes::UINDEX, UOF_LoadedAndUnloaded );
        if ( !docObjs.isEmpty() ) {
            indObjs += docObjs;
        } else {
            resTasks.append( new OpenUIndexViewerTask( doc ) );
        }
    }
    
    if( !indObjs.isEmpty() ) {
        foreach( GObject* o, indObjs ) {
            if( o->getGObjectType() == GObjectTypes::UNLOADED ){
                resTasks.append( new OpenUIndexViewerTask( qobject_cast< UnloadedObject* >( o ) ) );
            } else {
                assert( o->getGObjectType() == GObjectTypes::UINDEX );
                resTasks.append( new OpenUIndexViewerTask( qobject_cast< UIndexObject* >( o ) ) );
            }
        }
    }
    
    if (resTasks.isEmpty()) {
        return NULL;
    }
    if( resTasks.size() == 1 || single ) {
        return resTasks.first();
    }
    Task* result = new Task( tr( "Open multiple views" ), TaskFlag_NoRun );
    foreach( Task* t, resTasks ) {
        result->addSubTask( t );
    }
    return result;
}

Task* UIndexViewerFactory::createViewTask( const QString& viewName, const QVariantMap& stateData ) {
    Q_UNUSED(viewName);Q_UNUSED(stateData);
    return NULL;
}

bool UIndexViewerFactory::supportsSavedStates() const {
    return false;
}

} // U2
