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
#include <U2Core/Settings.h>

#include "SerializeUtils.h"
#include "RemoteMachineMonitor.h"

namespace U2 {

const QString RemoteMachineMonitor::REMOTE_MACHINE_MONITOR_SETTINGS_TAG = "rservice/machines";


RemoteMachineMonitor::RemoteMachineMonitor() : initialized( false ) {
}

void RemoteMachineMonitor::ensureInitialized() {
    if (!initialized) {
        initialize();
    }
}

void RemoteMachineMonitor::initialize() {
    assert(!initialized);
    initialized = true;

    Settings * settings = AppContext::getSettings();
    assert( NULL != settings );
    if( !deserializeMachines( settings->getValue( REMOTE_MACHINE_MONITOR_SETTINGS_TAG ) ) ) {
        qDeleteAll( getMachinesList() );
        items.clear();
        return;
    }
}

RemoteMachineMonitor::~RemoteMachineMonitor() {
    if (!initialized) {
        return;
    }
    saveSettings();
    qDeleteAll( getMachinesList() );
}

void RemoteMachineMonitor::saveSettings() {
    if (!initialized) {
        return;
    }
    Settings * settings = AppContext::getSettings();
    assert( NULL != settings );
    QVariant serializedMachines = serializeMachines();
    assert( serializedMachines.isValid() );
    settings->setValue( REMOTE_MACHINE_MONITOR_SETTINGS_TAG, serializedMachines );
}

QVariant RemoteMachineMonitor::serializeMachines() const {
    QVariantList res;
    foreach( const RemoteMachineMonitorItem & item, items ) {
        QVariantList itemRes;
        itemRes << QVariant( SerializeUtils::serializeRemoteMachineSettings( item.machine ) );
        itemRes << QVariant( item.selected );
        res << QVariant( itemRes );
    }
    return res;
}

bool RemoteMachineMonitor::deserializeMachines( const QVariant & data ) {
    if( !data.canConvert( QVariant::List ) ) {
        return false;
    }
    QVariantList args = data.toList();
    
    foreach( QVariant arg, args ) {
        if( !arg.canConvert( QVariant::List ) ) {
            return false;
        }
        QVariantList itemArgs = arg.toList();
        if( 2 != itemArgs.size() ) {
            return false;
        }
        RemoteMachineMonitorItem item;
        if( !SerializeUtils::deserializeRemoteMachineSettings( itemArgs[0].toString(), &item.machine ) ) {
            assert( NULL == item.machine );
            return false;
        }
        if( !SerializeUtils::deserializeValue<bool>( itemArgs[1], &item.selected ) ) {
            return false;
        }
        items << item;
    }
    return true;
}

bool RemoteMachineMonitor::addMachine( RemoteMachineSettings * machine, bool selected ) {
    ensureInitialized();

    if( NULL == machine || hasMachineInMonitor( machine ) ) {
        return false;
    }
    items.append( RemoteMachineMonitorItem( machine, selected ) );
    return true;
}

void RemoteMachineMonitor::setSelected( RemoteMachineSettings * machine, bool selected ) {
    if( NULL == machine ) {
        return;
    }
    int sz = items.size();
    for( int i = 0; i < sz; ++i ) {
        RemoteMachineMonitorItem & item = items[i];
        if( item.machine == machine ) {
            item.selected = selected;
            break;
        }
    }
}

bool RemoteMachineMonitor::hasMachineInMonitor( RemoteMachineSettings * machine ) const {
    assert( NULL != machine );
    foreach( const RemoteMachineMonitorItem & item, items ) {
        if( item.machine == machine ) {
            return true;
        }
    }
    return false;
}

void RemoteMachineMonitor::removeMachine( RemoteMachineSettings * machine ) {
    ensureInitialized();

    if( NULL == machine || !hasMachineInMonitor( machine ) ) {
        return;
    }
    
    int at = -1;
    int sz = items.size();
    for( int i = 0; i < sz; ++i ) {
        if( items.at( i ).machine == machine ) {
            at = i;
            break;
        }
    }
    assert( -1 != at );
    delete items.at( at ).machine;
    items.removeAt( at );
}

QList< RemoteMachineSettings * > RemoteMachineMonitor::getMachinesList() {
    ensureInitialized();
    QList< RemoteMachineSettings* > ret;
    foreach( const RemoteMachineMonitorItem & item, items ) {
        ret << item.machine;
    }
    return ret;
}

QList< RemoteMachineMonitorItem > RemoteMachineMonitor::getRemoteMachineMonitorItems() {
    ensureInitialized();
    return items;
}

QList< RemoteMachineSettings* > RemoteMachineMonitor::getSelectedMachines() {
    ensureInitialized();

    QList< RemoteMachineSettings* > res;
    foreach( const RemoteMachineMonitorItem & item, items ) {
        if( item.selected ) {
            res << item.machine;
        }
    }
    return res;
}

RemoteMachineSettings* RemoteMachineMonitor::findMachine( const QString& id ) const
{
    foreach ( const RemoteMachineMonitorItem& item, items) {
        if (item.machine->serialize() == id ) {
            return item.machine;
        }
    }
    return NULL;
}




} // U2
