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


#ifndef _U2_REMOTE_MACHINE_MONITOR_H_
#define _U2_REMOTE_MACHINE_MONITOR_H_

#include <QtCore/QList>
#include <QtCore/QVariant>

#include <U2Core/global.h>
#include "RemoteMachine.h"

namespace U2 {

/*
 * Stores info about all remote machines registered in system
 * Can be accessed by AppContext::getRemoteMachineMonitor()
 */

class U2REMOTE_EXPORT RemoteMachineMonitor {
public:
    static const QString REMOTE_MACHINE_MONITOR_SETTINGS_TAG;
    
public:
    RemoteMachineMonitor();
    ~RemoteMachineMonitor();
    /* RemoteMachineMonitor shares ownership of the machine settings with machine instances */
    bool addMachineConfiguration( const RemoteMachineSettingsPtr& machineSettings);
    void removeMachineConfiguration( const RemoteMachineSettingsPtr& machineSettings);
    RemoteMachineSettingsPtr findMachineSettings(const QString& serializedSettings) const;
    /* Function not const because we can call initialize() here */
    QList<RemoteMachineSettingsPtr> getRemoteMachineMonitorItems(); 
    void saveSettings();
    
private:
    RemoteMachineMonitor( const RemoteMachineMonitor & );
    RemoteMachineMonitor & operator=( const RemoteMachineMonitor & );
    
    QVariant serializeMachines() const;
    bool deserializeMachines( const QVariant & data );
    
    void ensureInitialized();
    void initialize();
    
private:
    QList<RemoteMachineSettingsPtr> items;
    bool initialized;
    
}; // RemoteMachineMonitor

} // U2

#endif // _U2_REMOTE_MACHINE_MONITOR_H_
