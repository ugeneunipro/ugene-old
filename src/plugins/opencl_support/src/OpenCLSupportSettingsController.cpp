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

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QCheckBox>

#include <U2Core/AppContext.h>
#include <U2Algorithm/OpenCLGpuRegistry.h>
#include <U2Core/AppResources.h>

#include "OpenCLSupportSettingsController.h"

namespace U2 {

OpenCLSupportSettingsPageController::OpenCLSupportSettingsPageController( const QString & _displayMsg, QObject * p /* = 0 */ ) :
AppSettingsGUIPageController( tr("OpenCL"), OpenCLSupportSettingsPageId, p ), displayMsg(_displayMsg) {}

AppSettingsGUIPageState * OpenCLSupportSettingsPageController::getSavedState() {
    QList<OpenCLGpuModel *> registeredGpus = AppContext::getOpenCLGpuRegistry()->getRegisteredGpus();
    OpenCLSupportSettingsPageState * s = new OpenCLSupportSettingsPageState( registeredGpus.size() );
    for( int i = 0, end = s->enabledGpus.size(); i < end; ++i ) {
        s->enabledGpus[i] = registeredGpus.at(i)->isEnabled();
    }

    return s;    
}

void OpenCLSupportSettingsPageController::saveState( AppSettingsGUIPageState * _s ) {
    QList<OpenCLGpuModel *> registeredGpus = AppContext::getOpenCLGpuRegistry()->getRegisteredGpus();
    OpenCLSupportSettingsPageState * s = qobject_cast<OpenCLSupportSettingsPageState *>(_s);

    //saving state of enabled/disabled GPUs into registry
    for( int i = 0, end = s->enabledGpus.size(); i < end; ++i ) {
        registeredGpus[i]->setEnabled( s->enabledGpus[i] );
    }

    //increasing/decreasing maxuse of according resource
    int totalEnabled = s->enabledGpus.count(true);
    AppResource * gpuResource = AppResourcePool::instance()->getResource( RESOURCE_OPENCL_GPU );
    if( gpuResource ) {
        gpuResource->maxUse = totalEnabled;
    } //else - resource was not registered, nothing to do.
}

AppSettingsGUIPageWidget * OpenCLSupportSettingsPageController::createWidget( AppSettingsGUIPageState* state ) {
    OpenCLSupportSettingsPageWidget * w = new OpenCLSupportSettingsPageWidget(displayMsg, this);
    w->setState(state);
    return w;
}

OpenCLSupportSettingsPageState::OpenCLSupportSettingsPageState( int num_gpus ) {
    assert( num_gpus >= 0 );
    enabledGpus.resize( num_gpus );
}

const static char * gpusDiscoveredText = 
    "The following OpenCL-enabled GPUs are detected.<br>\
    Check the GPUs to use for accelerating algorithms computations.";

const static char * noGpusDiscoveredText = "No OpenCL-enabled GPU detected.";

OpenCLSupportSettingsPageWidget::OpenCLSupportSettingsPageWidget( const QString & _msg, OpenCLSupportSettingsPageController * /*ctrl*/ ) :
onlyMsg(_msg){

    if( !onlyMsg.isEmpty() ) {
        //just display the centered warning message
        QHBoxLayout* hLayout = new QHBoxLayout(this);
        QLabel * msgLabel = new QLabel( onlyMsg, this );
        msgLabel->setAlignment( Qt::AlignLeft );

        hLayout->setAlignment( Qt::AlignTop | Qt::AlignLeft);
        hLayout->addWidget(msgLabel);
        hLayout->addStretch();
        setLayout(hLayout);
    } else {
        //everything is OK - adding info about all available GPUs
        QVBoxLayout* vLayout = new QVBoxLayout(this);
        QList<OpenCLGpuModel *> gpus = AppContext::getOpenCLGpuRegistry()->getRegisteredGpus();
        const QString & actualText = gpus.empty() ? tr(noGpusDiscoveredText) : tr(gpusDiscoveredText);
        QLabel * gpusDiscoveredLabel = new QLabel( actualText, this );
        gpusDiscoveredLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        vLayout->addWidget( gpusDiscoveredLabel );

        foreach( OpenCLGpuModel * m, gpus ) {
            vLayout->setAlignment( Qt::AlignLeft | Qt::AlignTop );
            QHBoxLayout * hLayout = new QHBoxLayout(this);

            QString gpuText = m->getName() + " " + QString::number(m->getGlobalMemorySizeBytes() / (1024*1024)) + " Mb"; 
            QCheckBox * check = new QCheckBox( gpuText, this );

            check->setChecked(true);

            gpuEnableChecks.push_back(check);
            hLayout->addWidget(check);
            vLayout->addLayout( hLayout );
        }
        setLayout(vLayout);
    }
    
}

void OpenCLSupportSettingsPageWidget::setState( AppSettingsGUIPageState * _state ) {
    OpenCLSupportSettingsPageState * state = qobject_cast<OpenCLSupportSettingsPageState*>(_state);
    assert( state->enabledGpus.size() == gpuEnableChecks.size() );

    for( int i = 0, end = state->enabledGpus.size(); i < end; ++i ) {
        gpuEnableChecks.at(i)->setChecked( state->enabledGpus.at(i) );
    }
}

AppSettingsGUIPageState* OpenCLSupportSettingsPageWidget::getState( QString & /*err*/ ) const {
    OpenCLSupportSettingsPageState * state = new OpenCLSupportSettingsPageState( gpuEnableChecks.size() );

    assert( state->enabledGpus.size() == gpuEnableChecks.size() );
    for( int i = 0, end = state->enabledGpus.size(); i < end; ++i ) {
        state->enabledGpus[i] = gpuEnableChecks.at(i)->isChecked();
    }
    return state;
}

} //namespace
