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

#include "uHMM3SearchDialogImpl.h"

#include <U2Core/AppContext.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>
#include <U2Core/GObjectTypes.h>
#include <U2Remote/DistributedComputingUtil.h>

#include <gobject/uHMMObject.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>


#include <cmath>
#include <cassert>

namespace U2 {

const QString UHMM3SearchDialogImpl::DOM_E_PLUS_PREFIX          = "1E+";
const QString UHMM3SearchDialogImpl::DOM_E_MINUS_PREFIX         = "1E";
const QString UHMM3SearchDialogImpl::HMM_FILES_DIR_ID           = "uhmmer3_search_dlg_impl_hmm_dir";
const QString UHMM3SearchDialogImpl::ANNOTATIONS_DEFAULT_NAME   = "hmm_signal";

UHMM3SearchDialogImpl::UHMM3SearchDialogImpl( const U2SequenceObject * seqObj, QWidget * p ) : QDialog( p ) {
    assert( NULL != seqObj );
    
    setupUi( this );
    useScoreTresholdGroup.addButton( useExplicitScoreTresholdButton );
    useScoreTresholdGroup.addButton( useGATresholdsButton );
    useScoreTresholdGroup.addButton( useNCTresholdsButton );
    useScoreTresholdGroup.addButton( useTCTresholdsButton );
    useExplicitScoreTresholdButton->setChecked( true );
    
    model.sequence = seqObj->getWholeSequence();
    setModelValues(); // default settings here
    
    // Annotations widget
    CreateAnnotationModel annModel;
    annModel.hideLocation = true;
    annModel.sequenceObjectRef = seqObj;
    annModel.data->name = ANNOTATIONS_DEFAULT_NAME;
    annModel.sequenceLen = seqObj->getSequenceLength();
    annotationsWidgetController = new CreateAnnotationWidgetController( annModel, this );
    QWidget * firstTab = tabWidget->widget( 0 );
    assert( NULL != firstTab );
    QVBoxLayout * curLayout = qobject_cast< QVBoxLayout* >( firstTab->layout() );
    assert( NULL != curLayout );
    QWidget * aw = annotationsWidgetController->getWidget();
    curLayout->insertWidget( 1, aw );
    aw->setMinimumSize( aw->layout()->minimumSize() );

    connect( cancelButton, SIGNAL( clicked() ), SLOT( sl_cancelButtonClicked() ) );
    connect( searchButton, SIGNAL( clicked() ), SLOT( sl_okButtonClicked() ) );
    connect( useEvalTresholdsButton, SIGNAL( toggled( bool ) ), SLOT( sl_useEvalTresholdsButtonChanged( bool ) ) );
    connect( useScoreTresholdsButton, SIGNAL( toggled( bool ) ), SLOT( sl_useScoreTresholdsButtonChanged( bool ) ) );
    connect( useExplicitScoreTresholdButton, SIGNAL( toggled( bool ) ), SLOT( sl_useExplicitScoreTresholdButton( bool ) ) );
    connect( maxCheckBox, SIGNAL( stateChanged( int ) ), SLOT( sl_maxCheckBoxChanged( int ) ) );
    connect( domESpinBox, SIGNAL( valueChanged( int ) ), SLOT( sl_domESpinBoxChanged( int ) ) );
    connect( queryHmmFileToolButton, SIGNAL( clicked() ), SLOT( sl_queryHmmFileToolButtonClicked() ) );
    connect( domZCheckBox, SIGNAL( stateChanged( int ) ), SLOT( sl_domZCheckBoxChanged( int ) ) );
}

void UHMM3SearchDialogImpl::setModelValues() {
    const UHMM3SearchSettings & settings = model.searchSettings.inner;
    domESpinBox->setValue( 1 ); assert( 10.0 == settings.domE );
    scoreTresholdDoubleSpin->setValue( 0 ); // because default is OPTION_NOT_SET
    domZDoubleSpinBox->setValue( 0 ); // because default is OPTION_NOT_SET
    nobiasCheckBox->setChecked( (bool)settings.noBiasFilter );
    nonull2CheckBox->setChecked( (bool)settings.noNull2 );
    maxCheckBox->setChecked( (bool)settings.doMax );
    f1DoubleSpinBox->setValue( settings.f1 );
    f2DoubleSpinBox->setValue( settings.f2 );
    f3DoubleSpinBox->setValue( settings.f3 );
    seedSpinBox->setValue( settings.seed );
}

void UHMM3SearchDialogImpl::sl_cancelButtonClicked() {
    reject();
}

void UHMM3SearchDialogImpl::getModelValues() {
    UHMM3SearchSettings & settings = model.searchSettings.inner;
    
    if( useEvalTresholdsButton->isChecked() ) {
        settings.domE =  pow( 10.0, domESpinBox->value() );
        settings.domT = OPTION_NOT_SET;
    } else if( useScoreTresholdsButton->isChecked() ) {
        if( useExplicitScoreTresholdButton->isChecked() ) {
            settings.domT = scoreTresholdDoubleSpin->value();
        } else if( useGATresholdsButton->isChecked() ) {
            settings.useBitCutoffs = p7H_GA;
        } else if( useNCTresholdsButton->isChecked() ) {
            settings.useBitCutoffs = p7H_NC;
        } else if( useTCTresholdsButton->isChecked() ) {
            settings.useBitCutoffs = p7H_TC;
        } else {
            assert( false );
        }
    } else {
        assert( false );
    }
    
    if( domZCheckBox->isChecked() ) {
        settings.domZ = domZDoubleSpinBox->value();
    } else {
        settings.domZ = OPTION_NOT_SET;
    }
    
    settings.noBiasFilter = nobiasCheckBox->isChecked();
    settings.noNull2 = nonull2CheckBox->isChecked();
    settings.doMax = maxCheckBox->isChecked();
    
    settings.f1 = f1DoubleSpinBox->value();
    settings.f2 = f2DoubleSpinBox->value();
    settings.f3 = f3DoubleSpinBox->value();
    
    settings.seed = seedSpinBox->value();
    
    model.hmmfile = queryHmmFileEdit->text();
}

QString UHMM3SearchDialogImpl::checkModel() {
    assert( checkUHMM3SearchSettings( &model.searchSettings.inner ) );
    QString ret;
    
    if( model.hmmfile.isEmpty() ) {
        ret = tr( "HMM profile file path is empty" );
        return ret;
    }
    ret = annotationsWidgetController->validate();
    if( !ret.isEmpty() ) {
        return ret;
    }
    
    return ret;
}

void UHMM3SearchDialogImpl::sl_okButtonClicked() {
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
    UHMM3SWSearchToAnnotationsTask * searchTask = new UHMM3SWSearchToAnnotationsTask( model.hmmfile, model.sequence,
        annModel.getAnnotationObject(), annModel.groupName, annModel.data->name, model.searchSettings );
    AppContext::getTaskScheduler()->registerTopLevelTask( searchTask );
    
    QDialog::accept();
}


void UHMM3SearchDialogImpl::sl_useEvalTresholdsButtonChanged( bool checked ) {
    domESpinBox->setEnabled( checked );
}

void UHMM3SearchDialogImpl::sl_useScoreTresholdsButtonChanged( bool checked ) {
    useExplicitScoreTresholdButton->setEnabled( checked );
    useGATresholdsButton->setEnabled( checked );
    useNCTresholdsButton->setEnabled( checked );
    useTCTresholdsButton->setEnabled( checked );
    if( !checked ) {
        scoreTresholdDoubleSpin->setEnabled( false );
    } else {
        scoreTresholdDoubleSpin->setEnabled( useExplicitScoreTresholdButton->isChecked() );
    }
}

void UHMM3SearchDialogImpl::sl_useExplicitScoreTresholdButton( bool checked ) {
    scoreTresholdDoubleSpin->setEnabled( checked );
}

void UHMM3SearchDialogImpl::sl_maxCheckBoxChanged( int state ) {
    assert( Qt::PartiallyChecked != state );
    bool unchecked = Qt::Unchecked == state;
    f1Label->setEnabled( unchecked );
    f2Label->setEnabled( unchecked );
    f3Label->setEnabled( unchecked );
    f1DoubleSpinBox->setEnabled( unchecked );
    f2DoubleSpinBox->setEnabled( unchecked );
    f3DoubleSpinBox->setEnabled( unchecked );
}

void UHMM3SearchDialogImpl::sl_domESpinBoxChanged( int newVal ) {
    const QString & prefix = 0 <= newVal ? DOM_E_PLUS_PREFIX : DOM_E_MINUS_PREFIX;
    domESpinBox->setPrefix( prefix );
}

void UHMM3SearchDialogImpl::sl_queryHmmFileToolButtonClicked() {
    LastUsedDirHelper helper( HMM_FILES_DIR_ID );
    helper.url = QFileDialog::getOpenFileName( this, tr( "Select query HMM profile" ), 
        helper, DialogUtils::prepareDocumentsFileFilterByObjType( UHMMObject::UHMM_OT, true ) );
    if( !helper.url.isEmpty() ) {
        queryHmmFileEdit->setText( helper.url );
    }
}

void UHMM3SearchDialogImpl::sl_domZCheckBoxChanged( int state ) {
    assert( Qt::PartiallyChecked != state );
    bool checked = Qt::Checked == state;
    domZDoubleSpinBox->setEnabled( checked );
}

} // U2
