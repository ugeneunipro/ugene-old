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

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowUtils.h>

#include "globals.h"
#include "WorkflowElementFacade.h"

static const QString READ_ELEMENT_PREFIX =          "read-";
static const QString WRITE_ELEMENT_PREFIX =         "write-";
static const QString INPUT_PORT_PREFIX =            "in-";
static const QString OUTPUT_PORT_PREFIX =           "out-";

U2ErrorType getActorPrototype( const QString &elementType,
    U2::Workflow::ActorPrototype **outProto )
{
    *outProto = NULL;
    U2::Workflow::ActorPrototypeRegistry *prototypeRegistry
        = U2::Workflow::WorkflowEnv::getProtoRegistry( );
    CHECK( NULL != prototypeRegistry, U2_INVALID_CALL );
    U2::Workflow::ActorPrototype *prototype = prototypeRegistry->getProto( elementType );
    CHECK( NULL != prototype, U2_UNKNOWN_ELEMENT );
    *outProto = prototype;
    return U2_OK;
}

U2ErrorType getElementPorts( const QString &elementType,
    QList<U2::Workflow::PortDescriptor *> &ports )
{
    U2ErrorType result = U2_OK;
    ports.clear( );
    U2::Workflow::ActorPrototype *prototype = NULL;
    CHECK( U2_OK == ( result = getActorPrototype( elementType, &prototype ) ), result );
    ports = prototype->getPortDesciptors( );
    return result;
}

U2ErrorType addPrefixToSlotId( const QString &slotId, const QString &prefix, QString &outString ) {
    outString = QString( );
    outString = prefix + slotId;
    return U2_OK;
}

namespace U2 {

U2ErrorType WorkflowElementFacade::getElementNameByType( const QString &type, QString &name ) {
    U2ErrorType result = U2_OK;
    name = QString( );
    Workflow::ActorPrototype *prototype = NULL;
    CHECK( U2_OK == ( result = getActorPrototype( type, &prototype ) ), result );
    name = prototype->getDisplayName( );
    return result;
}

U2ErrorType WorkflowElementFacade::doesElementTypeExist( const QString &type, bool *exists ) {
    Workflow::ActorPrototype *prototype = NULL;
    U2ErrorType result = getActorPrototype( type, &prototype );
    *exists = ( NULL != prototype );
    return result;
}

U2ErrorType WorkflowElementFacade::doesElementHaveParameter( const QString &elementType,
    const QString &parameterName, bool *has )
{
    U2ErrorType result = U2_OK;
    *has = false;
    Workflow::ActorPrototype *prototype = NULL;
    CHECK( U2_OK == ( result = getActorPrototype( elementType, &prototype ) ), result );
    *has = ( NULL != prototype->getAttribute( parameterName ) );
    if ( !( *has ) ) {
        return U2_INVALID_NAME;
    }
    return result;
}

U2ErrorType WorkflowElementFacade::doesElementHavePort( const QString &elementType,
    const QString &portId, bool *has )
{
    U2ErrorType result = U2_OK;
    *has = false;
    QList<Workflow::PortDescriptor *> ports;
    CHECK( U2_OK == ( result = getElementPorts( elementType, ports ) ), result );
    foreach ( Workflow::PortDescriptor *port, ports ) {
        if ( portId == port->getId( ) ) {
            *has = true;
            break;
        }
    }
    if ( !( *has ) ) {
        return U2_INVALID_NAME;
    }
    return result;
}

U2ErrorType WorkflowElementFacade::doesElementHaveOutputSlot( const QString &elementType,
    const QString &slotId, bool *has )
{
    U2ErrorType result = U2_OK;
    *has = false;
    QList<Workflow::PortDescriptor *> ports;
    CHECK( U2_OK == ( result = getElementPorts( elementType, ports ) ), result );
    foreach ( Workflow::PortDescriptor *port, ports ) {
        if ( port->isOutput( ) ) { // considering all WD elements to have only one output port
            *has = port->getOwnTypeMap( ).contains( slotId );
            break;
        }
    }
    if ( !( *has ) ) {
        return U2_INVALID_NAME;
    }
    return result;
}

U2ErrorType WorkflowElementFacade::doesElementHaveInputSlot( const QString &elementType,
    const QString &portId, const QString &slotId, bool *has )
{
    U2ErrorType result = U2_OK;
    *has = false;
    QList<Workflow::PortDescriptor *> ports;
    CHECK( U2_OK == ( result = getElementPorts( elementType, ports ) ), result );
    foreach ( Workflow::PortDescriptor *port, ports ) {
        if ( portId == port->getId( ) && port->isInput( ) ) {
            *has = port->getOwnTypeMap( ).contains( slotId );
            break;
        }
    }
    if ( !( *has ) ) {
        return U2_INVALID_NAME;
    }
    return result;
}

U2ErrorType WorkflowElementFacade::doesElementSuitForSas( const QString &elementType, bool *suits )
{
    U2ErrorType result = U2_OK;
    *suits = false;
    QList<Workflow::PortDescriptor *> ports;
    CHECK( U2_OK == ( result = getElementPorts( elementType, ports ) ), result );
    if ( 2 == ports.length( ) && (ports.first( )->isInput( ) ^ ports.last( )->isInput( ) ) ) {
        foreach ( Workflow::PortDescriptor *port, ports ) {
            if ( !( *suits = ( 1 == port->getOwnTypeMap( ).size( ) ) ) ) {
                break;
            }
        }
    }
    return result;
}

U2ErrorType WorkflowElementFacade::getElementSlotIds( const QString &elementType, bool inputSlot,
    const QString &portId, QStringList &slotIds )
{
    U2ErrorType result = U2_OK;
    slotIds.clear( );
    QList<Workflow::PortDescriptor *> ports;
    CHECK( U2_OK == ( result = getElementPorts( elementType, ports ) ), result );
    foreach ( Workflow::PortDescriptor *port, ports ) {
        if ( !( inputSlot ^ port->isInput( ) ) && portId.isEmpty( ) || portId == port->getId( ) ) {
            QList<Descriptor> slotList = port->getOwnTypeMap( ).keys( );
            foreach ( Descriptor slotDescriptor, slotList ) {
                slotIds.append( slotDescriptor.getId( ) );
            }
            break;
        }
    }
    if ( slotIds.isEmpty( ) ) {
        return U2_INVALID_NAME;
    }
    return U2_OK;
}

U2ErrorType WorkflowElementFacade::getReadElementTypeForSlot( const QString &slotId,
    QString &readerType )
{
    return addPrefixToSlotId( slotId, READ_ELEMENT_PREFIX, readerType );
}

U2ErrorType WorkflowElementFacade::getWriteElementTypeForSlot( const QString &slotId,
    QString &writerType )
{
    return addPrefixToSlotId( slotId, WRITE_ELEMENT_PREFIX, writerType );
}

U2ErrorType WorkflowElementFacade::getInputPortIdForSlot( const QString &slotId, QString &portId ) {
    return addPrefixToSlotId( slotId, INPUT_PORT_PREFIX, portId );
}

U2ErrorType WorkflowElementFacade::getOutputPortIdForSlot( const QString &slotId, QString &portId ) {
    return addPrefixToSlotId( slotId, OUTPUT_PORT_PREFIX, portId );
}

} // namespace U2