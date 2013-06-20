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
        items.clear();
        return;
    }
}

RemoteMachineMonitor::~RemoteMachineMonitor() {
    if (!initialized) {
        return;
    }
    saveSettings();
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
    foreach( const RemoteMachineSettingsPtr& item, items ) {
        QVariantList itemRes;
        // call leads to FAIL("Obsolete! Not implemented!", "");
//         QString data = SerializeUtils::serializeRemoteMachineSettings(item);
//         itemRes << data;
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
        if( itemArgs.size() == 0 ) {
            return false;
        }
        RemoteMachineSettingsPtr machineSettings = SerializeUtils::deserializeRemoteMachineSettings(itemArgs[0].toString());
        if( machineSettings == NULL ) {
            return false;
        }
        
        items << machineSettings;
    }
    return true;
}

bool RemoteMachineMonitor::addMachineConfiguration( const RemoteMachineSettingsPtr& machineSettings ) {
    ensureInitialized();

    if( NULL == machineSettings || items.contains(machineSettings) ) {
        return false;
    }
    items.append( machineSettings );
    return true;
}

void RemoteMachineMonitor::removeMachineConfiguration( const RemoteMachineSettingsPtr& machineSettings ) {
    ensureInitialized();

    if( NULL == machineSettings ) {
        return;
    }
    
    items.removeOne(machineSettings);
}

QList<RemoteMachineSettingsPtr> RemoteMachineMonitor::getRemoteMachineMonitorItems() {
    ensureInitialized();
    return items;
}


RemoteMachineSettingsPtr RemoteMachineMonitor::findMachineSettings( const QString& id ) const
{
    foreach ( const RemoteMachineSettingsPtr& item, items) {
        if (item->serialize() == id ) {
            return item;
        }
    }
    return RemoteMachineSettingsPtr();
}




} // U2
