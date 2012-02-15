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

#ifndef _U2_DISTRIBUTED_COMPUTING_UTIL_H
#define _U2_DISTRIBUTED_COMPUTING_UTIL_H

#include <QtCore/QObject>

#include "RemoteMachine.h"

namespace U2 {

    class PingTask;

    /* RemoteMachine functions are blocking, so this task can be useful, 
    * because it waits for remote machine responses in separate thread 
    */
    class U2REMOTE_EXPORT RetrieveRemoteMachineInfoTask : public Task {
        Q_OBJECT
    public:
        RetrieveRemoteMachineInfoTask( RemoteMachineSettingsPtr s);
        ~RetrieveRemoteMachineInfoTask();

        virtual void prepare();
        virtual void run();
        virtual ReportResult report();

        QStringList getServicesList() const;
        //QUuid getUuid() const;
        QString getHostName() const;
        bool isPingOk() const;
        RemoteMachineSettingsPtr getMachineSettings() const {return settings;}

    private:
        QStringList                 services;
        QString                     hostname;
        PingTask*                   pingTask;
        bool                        pingOK;
        RemoteMachine *             machine;
        RemoteMachineSettingsPtr    settings;
    }; // RetrieveRemoteMachineInfoTask


    /* Retrieves public RemoteMachines settings from hard-coded ugene websites
    */
    class RetrievePublicMachinesTask : public Task {
        Q_OBJECT
    public:
        static const QString PUBLIC_MACHINES_KEEPER_SERVER;
        static const QString PUBLIC_MACHINES_KEEPER_PAGE;
        static const QString PUBLIC_MACHINES_STR_SEPARATOR;

    public:
        RetrievePublicMachinesTask();
        ~RetrievePublicMachinesTask();

        virtual void run();

        QList< RemoteMachineSettingsPtr > getPublicMachines() const;
        QList< RemoteMachineSettingsPtr > takePublicMachines();

    private:
        void processEncodedMachines( const QString & encodedMachines );

    private:
        QList< RemoteMachineSettingsPtr > publicMachines;

    }; // RetrievePublicMachinesTask

    /* Save remote machine settings to file 
    */
    class SaveRemoteMachineSettings : public Task {
        Q_OBJECT
    public:
        SaveRemoteMachineSettings(const RemoteMachineSettingsPtr& machineSettings, const QString& filename);
        virtual void run();

    private:
        QByteArray data;
        QString filename;

    }; // SaveRemoteMachineSettings

} // U2

#endif // _U2_DISTRIBUTED_COMPUTING_UTIL_H
