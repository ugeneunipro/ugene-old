/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include <cassert>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Core/GObjectTypes.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>

#include <phmmer/uhmm3PhmmerTask.h>
#include "uHMM3PhmmerDialogImpl.h"

namespace U2 {

const QString UHMM3PhmmerDialogImpl::QUERY_FILES_DIR            = "uhmm3_phmmer_query_files_dir";
const QString UHMM3PhmmerDialogImpl::DOM_E_PLUS_PREFIX          = "1E+";
const QString UHMM3PhmmerDialogImpl::DOM_E_MINUS_PREFIX         = "1E";
const QString UHMM3PhmmerDialogImpl::ANNOTATIONS_DEFAULT_NAME   = "signal";

UHMM3PhmmerDialogImpl::UHMM3PhmmerDialogImpl( const U2SequenceObject * seqObj, QWidget * p ) : QDialog( p ) {
    assert( NULL != seqObj );
    setupUi( this );
    
    model.dbSequence = seqObj->getWholeSequence();
    setModelValues(); // default model here
    
    // Annotations widget
    CreateAnnotationModel annModel;
    annModel.hideLocation = true;
    annModel.sequenceObjectRef = seqObj;
    annModel.data->name = ANNOTATIONS_DEFAULT_NAME;
    annModel.sequenceLen = seqObj->getSequenceLength();
    annotationsWidgetController = new CreateAnnotationWidgetController( annModel, this );
    QWidget * firstTab = mainTabWidget->widget( 0 );
    assert( NULL != firstTab );
    QVBoxLayout * curLayout = qobject_cast< QVBoxLayout* >( firstTab->layout() );
    assert( NULL != curLayout );
    curLayout->insertWidget( ANNOTATIONS_WIDGET_LOCATION, annotationsWidgetController->getWidget() );
    firstTab->setMinimumHeight( firstTab->size().height() );
    
    connect( queryToolButton, SIGNAL( clicked() ), SLOT( sl_queryToolButtonClicked() ) );
    connect( okPushButton, SIGNAL( clicked() ), SLOT( sl_okButtonClicked() ) );
    connect( cancelPushButton, SIGNAL( clicked() ), SLOT( sl_cancelButtonClicked() ) );
    connect( useEvalTresholdsButton, SIGNAL( toggled( bool ) ), SLOT( sl_useEvalTresholdsButtonChanged( bool ) ) );
    connect( useScoreTresholdsButton, SIGNAL( toggled( bool ) ), SLOT( sl_useScoreTresholdsButtonChanged( bool ) ) );
    connect( domZCheckBox, SIGNAL( stateChanged( int ) ), SLOT( sl_domZCheckBoxChanged( int ) ) );
    connect( maxCheckBox, SIGNAL( stateChanged( int ) ), SLOT( sl_maxCheckBoxChanged( int ) ) );
    connect( domESpinBox, SIGNAL( valueChanged( int ) ), SLOT( sl_domESpinBoxChanged( int ) ) );
}

void UHMM3PhmmerDialogImpl::setModelValues() {
    const UHMM3PhmmerSettings & settings = model.phmmerSettings;
    domESpinBox->setValue( 1 ); assert( 10.0 == settings.domE );
    scoreTresholdDoubleSpin->setValue( settings.domT );
    f1DoubleSpinBox->setValue( settings.f1 );
    f2DoubleSpinBox->setValue( settings.f2 );
    f3DoubleSpinBox->setValue( settings.f3 );
    seedSpinBox->setValue( settings.seed );
    emlSpinBox->setValue( settings.eml );
    emnSpinBox->setValue( settings.emn );
    evlSpinBox->setValue( settings.evl );
    evnSpinBox->setValue( settings.evn );
    eflSpinBox->setValue( settings.efl );
    efnSpinBox->setValue( settings.efn );
    eftDoubleSpinBox->setValue( settings.eft );
    popenDoubleSpinBox->setValue( settings.popen );
    pextendDoubleSpinBox->setValue( settings.pextend );
}

void UHMM3PhmmerDialogImpl::sl_queryToolButtonClicked() {
    LastUsedDirHelper helper( QUERY_FILES_DIR );
    helper.url = QFileDialog::getOpenFileName( this, tr( "Select query sequence file" ),
        helper, DialogUtils::prepareDocumentsFileFilterByObjType( GObjectTypes::SEQUENCE, true ) );
    if( !helper.url.isEmpty() ) {
        queryLineEdit->setText( helper.url );
    }
}

void UHMM3PhmmerDialogImpl::sl_cancelButtonClicked() {
    reject();
}

void UHMM3PhmmerDialogImpl::getModelValues() {
    UHMM3PhmmerSettings & settings = model.phmmerSettings;
    
    model.queryfile = queryLineEdit->text();
    if( useEvalTresholdsButton->isChecked() ) {
        settings.domE = pow( 10.0, domESpinBox->value() );
        settings.domT = OPTION_NOT_SET;
    } else if( useScoreTresholdsButton->isChecked() ) {
        settings.domT = scoreTresholdDoubleSpin->value();
    } else {
        assert( false );
    }
    
    settings.popen = popenDoubleSpinBox->value();
    settings.pextend = pextendDoubleSpinBox->value();
    
    settings.noBiasFilter = nobiasCheckBox->isChecked();
    settings.noNull2 = nonull2CheckBox->isChecked();
    settings.doMax = maxCheckBox->isChecked();
    settings.f1 = f1DoubleSpinBox->value();
    settings.f2 = f2DoubleSpinBox->value();
    settings.f3 = f3DoubleSpinBox->value();
    
    settings.eml         = emlSpinBox->value();
    settings.emn         = emnSpinBox->value();
    settings.evl         = evlSpinBox->value();
    settings.evn         = evnSpinBox->value();
    settings.efl         = eflSpinBox->value();
    settings.efn         = efnSpinBox->value();
    settings.eft         = eftDoubleSpinBox->value();
    settings.seed        = seedSpinBox->value();
}

QString UHMM3PhmmerDialogImpl::checkModel() {
    assert( model.phmmerSettings.isValid() );
    QString ret;
    if( model.queryfile.isEmpty() ) {
        ret = tr( "Query sequence file path is empty" );
        return ret;
    }
    ret = annotationsWidgetController->validate();
    if( !ret.isEmpty() ) {
        QMessageBox::critical( this, tr( "Error: bad arguments!" ), ret );
        return ret;
    }
    
    return ret;
}

void UHMM3PhmmerDialogImpl::sl_okButtonClicked() {
    getModelValues();
    QString err = checkModel();
    if( !err.isEmpty() ) {
        QMessageBox::critical( this, tr( "Error: bad arguments!" ), err );
        return;
    }
    bool objectPrepared = annotationsWidgetController->prepareAnnotationObject();
    if (!objectPrepared){
        QMessageBox::warning(this, tr("Error"), tr("Cannot create an annotation object. Please check settings"));
        return;
    }
    const CreateAnnotationModel & annModel = annotationsWidgetController->getModel();
    UHMM3PhmmerToAnnotationsTask * phmmerTask = new UHMM3PhmmerToAnnotationsTask( model.queryfile, model.dbSequence,
        annModel.getAnnotationObject(), annModel.groupName, annModel.data->name, model.phmmerSettings );
    AppContext::getTaskScheduler()->registerTopLevelTask( phmmerTask );
    
    QDialog::accept();
}

void UHMM3PhmmerDialogImpl::sl_useEvalTresholdsButtonChanged( bool checked ) {
    domESpinBox->setEnabled( checked );
}

void UHMM3PhmmerDialogImpl::sl_useScoreTresholdsButtonChanged( bool checked ) {
    scoreTresholdDoubleSpin->setEnabled( checked );
}

void UHMM3PhmmerDialogImpl::sl_domZCheckBoxChanged( int state ) {
    assert( Qt::PartiallyChecked != state );
    bool checked = Qt::Checked == state;
    domZDoubleSpinBox->setEnabled( checked );
}

void UHMM3PhmmerDialogImpl::sl_maxCheckBoxChanged( int state ) {
    assert( Qt::PartiallyChecked != state );
    bool unchecked = Qt::Unchecked == state;
    f1Label->setEnabled( unchecked );
    f2Label->setEnabled( unchecked );
    f3Label->setEnabled( unchecked );
    f1DoubleSpinBox->setEnabled( unchecked );
    f2DoubleSpinBox->setEnabled( unchecked );
    f3DoubleSpinBox->setEnabled( unchecked );
}

void UHMM3PhmmerDialogImpl::sl_domESpinBoxChanged( int newVal ) {
    const QString & prefix = 0 <= newVal ? DOM_E_PLUS_PREFIX : DOM_E_MINUS_PREFIX;
    domESpinBox->setPrefix( prefix );
}

} // U2
