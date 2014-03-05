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

#include <QtCore/QList>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QFileDialog>
#else
#include <QtWidgets/QFileDialog>
#endif
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/global.h>
#include <U2Core/U2SafePoints.h>
#include <U2Gui/HelpButton.h>
#include <U2Core/FormatUtils.h>

#include <U2Gui/LastUsedDirHelper.h>

#include "ui_ExportAnnotationsDialog.h"
#include "ExportAnnotationsDialog.h"

namespace U2 {

const QString ExportAnnotationsDialog::CSV_FORMAT_ID( "csv" );

ExportAnnotationsDialog::ExportAnnotationsDialog( const QString &filename, QWidget *parent )
    : QDialog( parent ), ui( new Ui::ExportAnnotationsDialog( ) )
{
    ui->setupUi( this );
    new HelpButton(this, ui->buttonBox, "4227370");

    ui->fileNameEdit->setText( filename );
    connect( ui->chooseFileButton, SIGNAL( clicked( ) ), SLOT( sl_onChooseFileButtonClicked( ) ) );

    DocumentFormatConstraints constr;
    QList<DocumentFormatId> supportedFormats;
    constr.supportedObjectTypes.insert( GObjectTypes::ANNOTATION_TABLE );
    constr.addFlagToSupport( DocumentFormatFlag_SupportWriting );
    DocumentFormatRegistry *formatRegistry = AppContext::getDocumentFormatRegistry( );
    SAFE_POINT( NULL != formatRegistry, "Invalid document format registry!", );
    supportedFormats = formatRegistry->selectFormats( constr );
    supportedFormats.append( CSV_FORMAT_ID );
    supportedFormatsExts.append( CSV_FORMAT_ID );

    foreach ( const DocumentFormatId &fid, supportedFormats ) {
        ui->formatsBox->addItem( fid );
        if ( CSV_FORMAT_ID != fid ) {
            DocumentFormat *df = AppContext::getDocumentFormatRegistry( )->getFormatById( fid );
            SAFE_POINT( NULL != df, "Invalid document format detected!", );
            supportedFormatsExts.append( df->getSupportedDocumentFileExtensions( ).first( ) );
        }
    }
    ui->formatsBox->setCurrentIndex( ui->formatsBox->findText( supportedFormats.first( ) ) );
    connect( ui->formatsBox, SIGNAL( currentIndexChanged( const QString & ) ),
        SLOT( sl_onFormatChanged( const QString & ) ) );
    sl_onFormatChanged( ui->formatsBox->currentText( ) );
}

ExportAnnotationsDialog::~ExportAnnotationsDialog( ) {
    delete ui;
}

void ExportAnnotationsDialog::sl_onFormatChanged( const QString &newFormat ) {
    const bool isNewFormatCsv = ( newFormat == CSV_FORMAT_ID );
    ui->exportSequenceCheck->setEnabled( isNewFormatCsv );
    ui->exportSequenceNameCheck->setEnabled( isNewFormatCsv );
    QString ext( "." );
    if ( isNewFormatCsv ) {
        ext.append( CSV_FORMAT_ID );
    } else {
        DocumentFormatRegistry *formatRegistry = AppContext::getDocumentFormatRegistry( );
        SAFE_POINT( NULL != formatRegistry, "Invalid document format registry!", );
        DocumentFormat *df = formatRegistry->getFormatById( newFormat );
        SAFE_POINT( NULL != df, "Invalid document format detected!", );
        ext.append( df->getSupportedDocumentFileExtensions( ).first( ) );
    }
    SAFE_POINT( 1 != ext.size( ), "Invalid file extension detected!", );
    QFileInfo fi( ui->fileNameEdit->text( ) );
    ui->fileNameEdit->setText( QDir::cleanPath( fi.absoluteDir( ).absolutePath( ) + "/"
        + fi.completeBaseName( ) + ext ) );
}

QString ExportAnnotationsDialog::filePath( ) const {
    return ui->fileNameEdit->text( );
}

bool ExportAnnotationsDialog::exportSequence( ) const {
    return ui->exportSequenceCheck->isChecked( );
}

bool ExportAnnotationsDialog::exportSequenceNames( ) const {
    return ui->exportSequenceNameCheck->isChecked( );
}

void ExportAnnotationsDialog::sl_onChooseFileButtonClicked( ) {
    QString curFormatExt;

    const QString curFormat = ui->formatsBox->currentText( );
    const bool isCurrentFormatCsv = ( curFormat == CSV_FORMAT_ID );
    if ( isCurrentFormatCsv ) {
        curFormatExt = CSV_FORMAT_ID;
    } else {
        DocumentFormatRegistry *documentReg = AppContext::getDocumentFormatRegistry( );
        SAFE_POINT( NULL != documentReg, "Invalid document format registry!", );
        DocumentFormat *df = documentReg->getFormatById( curFormat );
        SAFE_POINT( NULL != df, "Invalid document format detected!", );
        curFormatExt = df->getSupportedDocumentFileExtensions( ).first( );
    }

    QStringList formats( supportedFormatsExts );
    formats.removeAll( curFormatExt );
    formats.prepend( curFormatExt );
    QString fileFormats;
    foreach ( const QString &formatName, formats ) {
        fileFormats += formatName.toUpper( ) + " format (*." + formatName + ");;";
    }
    fileFormats.append( "All files (*)" );

    LastUsedDirHelper helper( "ExportAnnotationsDialogHelperDomain" );
    helper.url = QFileDialog::getSaveFileName( this, tr( "Select file to save annotations" ),
        helper.dir, fileFormats, NULL, QFileDialog::DontConfirmOverwrite );
    if ( !helper.url.isEmpty( ) ) {
        ui->fileNameEdit->setText( helper.url );
        sl_onFormatChanged( ui->formatsBox->currentText( ) );
    }
}

void ExportAnnotationsDialog::setExportSequenceVisible( bool value ) {
    ui->exportSequenceCheck->setVisible( value );
    ui->exportSequenceNameCheck->setVisible( value );
}

QString ExportAnnotationsDialog::fileFormat( ) const {
    return ui->formatsBox->currentText( );
}

} // namespace U2
