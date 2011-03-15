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
#include <U2Core/Task.h>

#include <QtGui/QMessageBox>
#include <QtGui/QMenu>

#include <U2Gui/GUIUtils.h>

#include <U2Core/ExportToNewFileFromIndexTask.h>

#include "UIndexViewerFactory.h"
#include "UIndexViewer.h"
#include "UIndexExportToNewFileDialogImpl.h"

namespace {

using namespace U2;

const QString GZIP_END = ".gz";

void updateModel( UIndexExportToNewFileDialogModel& model ) {
    if( model.gzipExported && !model.exportFilename.endsWith( GZIP_END ) ) {
        model.exportFilename.append( GZIP_END );
    }
}

} // anonymous namespace

namespace U2 {

const QString UIndexViewer::EXPORT_MENU_NAME       = "export_menu_name";

UIndexViewer::UIndexViewer( const QString& viewName, UIndexObject* o ) : GObjectView( UIndexViewerFactory::ID, viewName ), indexObject( o ), ui( NULL ) {
    objects.append( indexObject );
    requiredObjects.append( indexObject );
}

void UIndexViewer::buildStaticToolbar(QToolBar* tb) {
    GObjectView::buildStaticToolbar( tb );
}

void UIndexViewer::buildStaticMenu( QMenu* m ) {
    if( NULL == m ) {
        return;
    }
    addExportMenu( m );
    GObjectView::buildStaticMenu( m );
    GUIUtils::disableEmptySubmenus( m );
}

void UIndexViewer::addExportMenu( QMenu* m ) {
    assert( NULL != m );
    QMenu* exMenu = m->addMenu( tr( "Export " ) );
    exMenu->menuAction()->setObjectName( EXPORT_MENU_NAME );
    QAction* exToNewDoc = exMenu->addAction(tr("Save selection to a new file"));
    connect( exToNewDoc, SIGNAL( triggered() ), SLOT( sl_exportToNewDoc() ) );
}

Task* UIndexViewer::updateViewTask(const QVariantMap& stateData) {
    Q_UNUSED(stateData);
    return NULL;
}

QWidget* UIndexViewer::createWidget() {
    assert( NULL == ui );
    ui = new UIndexViewWidgetImpl( NULL, indexObject->getIndex() );
    connect( ui, SIGNAL( customContextMenuRequested (const QPoint& ) ),
                 SLOT  ( sl_onContextMenuRequested  (const QPoint&) ) );
    ui->setWindowIcon(GObjectTypes::getTypeInfo(GObjectTypes::UINDEX).icon);
    return ui;
}

void UIndexViewer::sl_onContextMenuRequested( const QPoint& pos ) {
    Q_UNUSED( pos );
    QMenu menu;
    
    addExportMenu( &menu );
    emit si_buildPopupMenu( this, &menu );
    GUIUtils::disableEmptySubmenus( &menu );
    menu.exec( QCursor::pos() );
}

void UIndexViewer::sl_exportToNewDoc() {
    QList< int > docNums = ui->getSelectedDocNums();
    if( docNums.isEmpty() ) {
        QMessageBox::critical( ui, tr( "Error" ), tr( "No items selected" ) );
        return;
    }
    UIndex index = ui->getIndex();
    //TODO: pass file ext?
    //TODO: support per-object export?
    UIndexExportToNewFileDialogImpl dlg;
    int rc = dlg.exec();
    if (rc == QDialog::Rejected) {
        return;
    }
    UIndexExportToNewFileDialogModel model = dlg.getModel();
    updateModel( model );
    ExportToNewFileFromIndexTask* exportTask = new ExportToNewFileFromIndexTask( index, docNums, model.exportFilename );
    AppContext::getTaskScheduler()->registerTopLevelTask( exportTask );
}


} // U2
