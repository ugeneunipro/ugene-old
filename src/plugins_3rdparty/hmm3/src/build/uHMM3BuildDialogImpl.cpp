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

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>

#include <gobject/uHMMObject.h>
#include "uHMM3BuildDialogImpl.h"
#include <U2Gui/HelpButton.h>
#include <QtGui/QPushButton>


namespace U2 {

const QString UHMM3BuildDialogImpl::MA_FILES_DIR_ID     = "uhmmer3_build_ma_files_dir";
const QString UHMM3BuildDialogImpl::HMM_FILES_DIR_ID    = "uhmmer3_build_hmm_files_dir";

void UHMM3BuildDialogImpl::setSignalsAndSlots() {

    QPushButton* okButton = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

    connect( maOpenFileButton, SIGNAL( clicked() ), SLOT( sl_maOpenFileButtonClicked() ) );
    connect( outHmmfileToolButton, SIGNAL( clicked() ), SLOT( sl_outHmmFileButtonClicked() ) );
    connect( okButton, SIGNAL( clicked() ), SLOT( sl_buildButtonClicked() ) );
    connect( cancelButton, SIGNAL( clicked() ), SLOT( sl_cancelButtonClicked() ) );
    connect( mcFastRadioButton, SIGNAL( toggled( bool ) ), SLOT( sl_fastMCRadioButtonChanged( bool ) ) );
    connect( wblosumRSWRadioButton, SIGNAL( toggled( bool ) ), SLOT( sl_wblosumRSWRadioButtonChanged( bool ) ) );
    connect( eentESWRadioButton, SIGNAL( toggled( bool ) ), SLOT( sl_eentESWRadioButtonChanged( bool ) ) );
    connect( eclustESWRadioButton, SIGNAL( toggled( bool ) ), SLOT( sl_eclustESWRadioButtonChanged( bool ) ) );
    connect( esetESWRadioButton, SIGNAL( toggled( bool ) ), SLOT( sl_esetESWRadioButtonChanged( bool ) ) );
    
    //temporary disabling of strange label/spinbox
    fragThreshDoubleSpinBox->setVisible(false);
    fragthreshLabel->setVisible(false);
}

void UHMM3BuildDialogImpl::initialize() {
    setupUi( this );
    new HelpButton(this, buttonBox, "4227693");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Build"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    setModelValues(); // build settings are default here
    setSignalsAndSlots();
}

UHMM3BuildDialogImpl::UHMM3BuildDialogImpl( const MAlignment & ma, QWidget * p ) : QDialog( p ) {
    initialize();
    model.alignment = ma;
    model.alignmentUsing = !model.alignment.isEmpty();
    
    if( model.alignmentUsing ) {
        maLoadFromFileEdit->hide();
        maLoadFromFileLabel->hide();
        maOpenFileButton->hide();
    }

}

void UHMM3BuildDialogImpl::setModelValues() {
    const UHMM3BuildSettings & settings = model.buildSettings.inner;
    symfracDoubleSpinBox->setValue( settings.symfrac );
    widRSWDoubleSpinBox->setValue( settings.wid );
    eidESWDoubleSpinBox->setValue( settings.eid );
    esetESWDoubleSpinBox->setValue( settings.eset );
    emlSpinBox->setValue( settings.eml );
    emnSpinBox->setValue( settings.emn );
    evlSpinBox->setValue( settings.evl );
    evnSpinBox->setValue( settings.evn );
    eflSpinBox->setValue( settings.efl );
    efnSpinBox->setValue( settings.efn );
    eftDoubleSpinBox->setValue( settings.eft );
    seedSpinBox->setValue( settings.seed );
    esigmaDoubleSpinBox->setValue( settings.esigma );
    fragThreshDoubleSpinBox->setValue( settings.fragtresh );
}

void UHMM3BuildDialogImpl::sl_maOpenFileButtonClicked() {
    LastUsedDirHelper helper( MA_FILES_DIR_ID );
    helper.url = QFileDialog::getOpenFileName( this, tr( "Select multiple alignment file" ),
        helper, DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true));
    if( !helper.url.isEmpty() ) {
        maLoadFromFileEdit->setText( helper.url );
    }
}

void UHMM3BuildDialogImpl::sl_outHmmFileButtonClicked() {
    LastUsedDirHelper helper( HMM_FILES_DIR_ID );
    helper.url = QFileDialog::getSaveFileName( this, tr( "Select hmm file to create" ), 
        helper, DialogUtils::prepareDocumentsFileFilterByObjType( UHMMObject::UHMM_OT, true) );
    if( !helper.url.isEmpty() ) {
        outHmmfileEdit->setText( helper.url );
    }
}

void UHMM3BuildDialogImpl::getModelValues() {
    UHMM3BuildSettings & bldSettings = model.buildSettings.inner;
    bldSettings.symfrac     = symfracDoubleSpinBox->value();
    bldSettings.wid         = widRSWDoubleSpinBox->value();
    bldSettings.eid         = eidESWDoubleSpinBox->value();
    bldSettings.eset        = esetESWDoubleSpinBox->value();
    bldSettings.eml         = emlSpinBox->value();
    bldSettings.emn         = emnSpinBox->value();
    bldSettings.evl         = evlSpinBox->value();
    bldSettings.evn         = evnSpinBox->value();
    bldSettings.efl         = eflSpinBox->value();
    bldSettings.efn         = efnSpinBox->value();
    bldSettings.eft         = eftDoubleSpinBox->value();
    bldSettings.seed        = seedSpinBox->value();
    bldSettings.esigma      = esigmaDoubleSpinBox->value();
    bldSettings.fragtresh   = fragThreshDoubleSpinBox->value();
    if( 0 != ereESWDoubleSpinBox->value() ) {
        bldSettings.ere = ereESWDoubleSpinBox->value();
    }
    
    if( mcFastRadioButton->isChecked() ) {
        bldSettings.archStrategy = p7_ARCH_FAST;
    } else {
        bldSettings.archStrategy = p7_ARCH_HAND;
    }
    
    if( wgscRSWRadioButton->isChecked() ) {
        bldSettings.wgtStrategy = p7_WGT_GSC;
    } else if( wblosumRSWRadioButton->isChecked() ) {
        bldSettings.wgtStrategy = p7_WGT_BLOSUM;
    } else if( wpbRSWRadioButton->isChecked() ) {
        bldSettings.wgtStrategy = p7_WGT_PB;
    } else if( wnoneRSWRadioButton->isChecked() ) {
        bldSettings.wgtStrategy = p7_WGT_NONE;
    } else if( wgivenRSWRadioButton->isChecked() ) {
        bldSettings.wgtStrategy = p7_WGT_GIVEN;
    } else {
        assert( false );
    }
    
    if( eentESWRadioButton->isChecked() ) {
        bldSettings.effnStrategy = p7_EFFN_ENTROPY;
    } else if( eclustESWRadioButton->isChecked() ) {
        bldSettings.effnStrategy = p7_EFFN_CLUST;
    } else if( enoneESWRadioButton->isChecked() ) {
        bldSettings.effnStrategy = p7_EFFN_NONE;
    } else if( esetESWRadioButton->isChecked() ) {
        bldSettings.effnStrategy = p7_EFFN_SET;
    } else {
        assert( false );
    }
    
    model.buildSettings.outFile = outHmmfileEdit->text();
    model.inputFile = maLoadFromFileEdit->text();
}

QString UHMM3BuildDialogImpl::checkModel() {
    assert( checkUHMM3BuildSettings( &model.buildSettings.inner ) );
    if( !model.alignmentUsing && model.inputFile.isEmpty() ) {
        return tr( "input file is empty" );
    }
    if( model.buildSettings.outFile.isEmpty() ) {
        return tr( "output hmm file is empty" );
    }
    return QString();
}

void UHMM3BuildDialogImpl::sl_buildButtonClicked() {
    getModelValues();
    QString err = checkModel();
    if( !err.isEmpty() ) {
        QMessageBox::critical( this, tr( "Error: bad arguments!" ), err );
        return;
    }
    
    Task * buildTask = NULL;
    if( model.alignmentUsing ) {
        buildTask = new UHMM3BuildToFileTask( model.buildSettings, model.alignment );
    } else {
        buildTask = new UHMM3BuildToFileTask( model.buildSettings, model.inputFile );
    }
    assert( NULL != buildTask );
    
    AppContext::getTaskScheduler()->registerTopLevelTask( buildTask );
    QDialog::accept();
}

void UHMM3BuildDialogImpl::sl_cancelButtonClicked() {
    reject();
}

void UHMM3BuildDialogImpl::sl_fastMCRadioButtonChanged( bool checked ) {
    mcFastSymfracLabel->setEnabled( checked );
    symfracDoubleSpinBox->setEnabled( checked );
}

void UHMM3BuildDialogImpl::sl_wblosumRSWRadioButtonChanged( bool checked ) {
    widRSWLabel->setEnabled( checked );
    widRSWDoubleSpinBox->setEnabled( checked );
}

void UHMM3BuildDialogImpl::sl_eentESWRadioButtonChanged( bool checked ) {
    ereESWDoubleSpinBox->setEnabled( checked );
    esigmaDoubleSpinBox->setEnabled( checked );
    esigmaLabel->setEnabled( checked );
    ereLabel->setEnabled( checked );
}

void UHMM3BuildDialogImpl::sl_eclustESWRadioButtonChanged( bool checked ) {
    eidESWLabel->setEnabled( checked );
    eidESWDoubleSpinBox->setEnabled( checked );
}

void UHMM3BuildDialogImpl::sl_esetESWRadioButtonChanged( bool checked ) {
    esetESWDoubleSpinBox->setEnabled( checked );
}

} // GB2
