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

QString SerializeUtils::serializeRemoteMachineSettings( RemoteMachineSettings * machine ) {
    assert( NULL != machine );
    return HEADER_COMMENT "\n" + machine->serialize();
}

bool SerializeUtils::deserializeRemoteMachineSettingsFromFile( const QString & machinePath, RemoteMachine ** machine ) {
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
    
    if( !deserializeRemoteMachineSettings( data, machine ) || NULL == machine ) {
        return false;
    }
    return true;
}

bool SerializeUtils::deserializeRemoteMachineSettingsFromFile( const QString & machinePath, RemoteMachineSettings ** settings ) {
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

static QString getDefaultProtocolId( ) {
    QString res;
    QList< ProtocolInfo* > infos = AppContext::getProtocolInfoRegistry()->getProtocolInfos();
    
    if (infos.count() > 0) {
        res = infos.first()->getId();
    }

    return res;
}

bool SerializeUtils::deserializeRemoteMachineSettings( const QString & data, RemoteMachineSettings ** ret, QString * retProtoId ) {
    if( NULL == ret ) {
        return false;
    }
    *ret = NULL;
    
    
    QString protoId = getDefaultProtocolId();
    ProtocolInfo * protoInfo = AppContext::getProtocolInfoRegistry()->getProtocolInfo( protoId );
    if( NULL == protoInfo ) {
        return false;
    }
        
    *ret = protoInfo->getRemoteMachineFactory()->createSettings( data );
    if( NULL != retProtoId ) {
        *retProtoId = protoId;
    }

    return NULL == *ret ? false : true;
}

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

} // U2
