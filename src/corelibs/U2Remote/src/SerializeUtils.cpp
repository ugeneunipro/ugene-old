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

#include <U2Core/AppContext.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignment.h>

#include "RemoteMachine.h"
#include "SerializeUtils.h"

#include <QtCore/QFile>

namespace U2 {

// remote machine settings

#define HEADER_COMMENT "# UGENE remote machine settings"

QString SerializeUtils::serializeRemoteMachineSettings( const RemoteMachineSettingsPtr& machineSettings ) {
    assert( NULL != machineSettings );
    return HEADER_COMMENT "\n" + machineSettings->serialize();
}

RemoteMachineSettingsPtr SerializeUtils::deserializeRemoteMachineSettingsFromFile( const QString & machinePath) {
    QFile file( machinePath );
    if( !file.open( QIODevice::ReadOnly ) ) {
        return RemoteMachineSettingsPtr();
    }
    
    QString data;
    while (!file.atEnd()) {
        QString line = file.readLine();
        if (!line.startsWith("#")) {
            data.append(line);
        }
    }
    
    return deserializeRemoteMachineSettings( data );
}

/*bool SerializeUtils::deserializeRemoteMachineSettingsFromFile( const QString & machinePath, RemoteMachineSettings ** settings ) {
    QFile file( machinePath );
    if( !file.open( QIODevice::ReadOnly ) ) {
        return NULL;
    }

    QString data;
    while (!file.atEnd()) {
        QString line = file.readLine();
        if (!line.startsWith("#")) {
            data.append(line);
        }
    }

    if( !deserializeRemoteMachineSettings( data, settings ) || NULL == settings ) { return false; }
    return true;
}
*/

static QString getDefaultProtocolId( ) {
    QString res;
    QList< ProtocolInfo* > infos = AppContext::getProtocolInfoRegistry()->getProtocolInfos();
    
    if (infos.count() > 0) {
        res = infos.first()->getId();
    }

    return res;
}

RemoteMachineSettingsPtr SerializeUtils::deserializeRemoteMachineSettings( const QString & data, QString * retProtoId ) {
    
    QString protoId = getDefaultProtocolId();
    ProtocolInfo * protoInfo = AppContext::getProtocolInfoRegistry()->getProtocolInfo( protoId );
    if( NULL == protoInfo ) {
        return RemoteMachineSettingsPtr();
    }
        
    RemoteMachineSettingsPtr machineSettings = protoInfo->getRemoteMachineFactory()->createSettings( data );
    if( NULL != retProtoId ) {
        *retProtoId = protoId;
    }

    return machineSettings;
}

/*
bool SerializeUtils::deserializeRemoteMachineSettings( const QString & data, RemoteMachine ** machine ) {
    if( NULL == machine ) {
        return false;
    }
    *machine = NULL;
    
    RemoteMachineSettings * settings = NULL;
    QString protoId;
    if( !deserializeRemoteMachineSettings( data, &settings, &protoId ) ) {
        assert( NULL == settings );
        return false;
    }
    assert( NULL != settings );
    *machine = AppContext::getProtocolInfoRegistry()->getProtocolInfo( protoId )->getRemoteMachineFactory()->createInstance( settings );
    delete settings;
    
    return NULL == *machine ? false : true;
}
*/

} // U2
