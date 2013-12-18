/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include <U2Core/FeaturesTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/ExportAnnotations2CSVTask.h>
#include <U2Gui/ExportAnnotationsDialog.h>
#include <U2Gui/ExportDocumentDialogController.h>
#include <U2Gui/LastUsedDirHelper.h>

#include "ExportObjectUtils.h"

namespace U2 {

void ExportObjectUtils::exportAnnotations( const QList<__Annotation> &inputAnnotations, const GUrl &dstUrl ) {
    QList<__Annotation> annotations = inputAnnotations; // copy for further modification
    if ( annotations.isEmpty( ) ) {
        QMessageBox::warning( QApplication::activeWindow( ), QObject::tr( "Export annotations..." ),
            QObject::tr( "Selected object doesn't have annotations" ) );
        return;
    }

    QString fileName = GUrlUtils::rollFileName( dstUrl.dirPath( ) + "/" + dstUrl.baseFileName( )
        + "_annotations.csv", DocumentUtils::getNewDocFileNameExcludesHint( ) );

    ExportAnnotationsDialog d( fileName, QApplication::activeWindow( ) );
    d.setExportSequenceVisible(false);

    if ( QDialog::Accepted != d.exec( ) ) {
        return;
    }

    // TODO: lock documents or use shared-data objects
    // same as in ADVExportContext::sl_saveSelectedAnnotations()
    qStableSort( annotations.begin( ), annotations.end( ), __Annotation::annotationLessThan );

    // run task
    Task * t = NULL;
    if ( ExportAnnotationsDialog::CSV_FORMAT_ID == d.fileFormat( ) ) {
        t = new ExportAnnotations2CSVTask( annotations, QByteArray( ), QString( ), NULL, false,
        false, d.filePath( ) );
    } else {
        t = saveAnnotationsTask( d.filePath( ), d.fileFormat( ), annotations );
    }
    SAFE_POINT( NULL != t, "Invalid task detected!", );

    AppContext::getTaskScheduler( )->registerTopLevelTask( t );
}

void ExportObjectUtils::exportObject2Document( GObject *object, const QString &url,
    bool tracePath )
{
    if ( NULL == object || object->isUnloaded( ) ) {
        return;
    }
    ExportDocumentDialogController dialog( object, QApplication::activeWindow( ), url );
    export2Document( dialog, tracePath );
}

void ExportObjectUtils::export2Document( ExportDocumentDialogController &dialog, bool tracePath ) {
    int result = dialog.exec( );
    if ( result != QDialog::Accepted ) {
        return;
    }

    if ( tracePath ) {
        LastUsedDirHelper h;
        h.url = dialog.getDocumentURL( );
    }
    QString dstUrl = dialog.getDocumentURL( );
    if ( dstUrl.isEmpty( ) ) {
        return;
    }

    Project *project = AppContext::getProject( );
    if ( NULL != project && project->findDocumentByURL( dstUrl ) ) {
        QMessageBox::critical( QApplication::activeWindow( ), QObject::tr( "Error" ),
            QObject::tr( "Document with the same URL is added to the project.\n"
            "Remove it from the project first." ) );
        return;
    }
    bool addToProject = dialog.getAddToProjectFlag( );

    IOAdapterRegistry *ioar = AppContext::getIOAdapterRegistry( );
    SAFE_POINT( NULL != ioar, "Invalid I/O environment!", );
    IOAdapterFactory *iof = ioar->getIOAdapterFactoryById( IOAdapterUtils::url2io( dstUrl ) );
    CHECK_EXT( NULL != iof,
        coreLog.error( QObject::tr( "Unable to create I/O factory for " ) + dstUrl ), );
    DocumentFormatRegistry *dfr =  AppContext::getDocumentFormatRegistry( );
    DocumentFormatId formatId = dialog.getDocumentFormatId( );
    DocumentFormat *df = dfr->getFormatById( formatId );
    CHECK_EXT( NULL != df,
        coreLog.error( QObject::tr( "Unknown document format I/O factory: " ) + formatId ), );

    U2OpStatusImpl os;
    Document *srcDoc = dialog.getSourceDoc( );
    Document *dstDoc = NULL;
    if ( NULL == srcDoc ) {
        dstDoc = df->createNewLoadedDocument( iof, dstUrl, os );
        dstDoc->addObject( dialog.getSourceObject( ) );
    } else {
        dstDoc = srcDoc->getSimpleCopy( df, iof, dstUrl );
    }

    SaveDocFlags flags = SaveDocFlags( SaveDoc_Roll ) | SaveDoc_DestroyButDontUnload;
    if ( addToProject ) {
        flags |= SaveDoc_OpenAfter;
    }
    SaveDocumentTask *t = new SaveDocumentTask( dstDoc, iof, dstUrl, flags );
    AppContext::getTaskScheduler( )->registerTopLevelTask( t );
}

Task * ExportObjectUtils::saveAnnotationsTask(const QString &filepath, const DocumentFormatId &format,
    const QList<__Annotation> &annList )
{
    SaveDocFlags fl( SaveDoc_Roll );
    fl |= SaveDoc_DestroyAfter;
    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry( )->getIOAdapterFactoryById(
        IOAdapterUtils::url2io( filepath ) );
    CHECK_EXT( NULL != iof,
        coreLog.error( QObject::tr( "Unable to create I/O factory for " ) + filepath ), NULL );
    DocumentFormat *df = AppContext::getDocumentFormatRegistry( )->getFormatById( format );
    CHECK_EXT( NULL != df,
        coreLog.error( QObject::tr( "Unknown document format I/O factory: " ) + format ), NULL );
    U2OpStatus2Log os;
    Document *doc = df->createNewLoadedDocument( iof, filepath, os );
    CHECK_OP( os, NULL );

    // object and annotations will be deleted when savedoc task will delete doc
    const U2DbiRef dbiRef = AppContext::getDbiRegistry( )->getSessionTmpDbiRef( os );
    CHECK_OP( os, NULL );
    FeaturesTableObject *att = new FeaturesTableObject( "exported_annotations", dbiRef );
    bool setAttName = false;
    foreach ( const __Annotation &a, annList ) {
        const FeaturesTableObject *parentObject = a.getGObject( );
        if ( !setAttName && NULL != parentObject ) {
            const QString newName = parentObject->getGObjectName( );
            SAFE_POINT( !newName.isEmpty( ), "Empty annotation name detected!", NULL );
            att->setGObjectName( newName );
            setAttName = true;
        }
        const QString groupName = a.getGroup( ).getName( );
        att->addAnnotation( a.getData( ), groupName );
    }
    att->setModified( false );
    doc->addObject( att );
    return new SaveDocumentTask( doc, fl, DocumentUtils::getNewDocFileNameExcludesHint( ) );
}

} // namespace U2
