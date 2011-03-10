#ifndef _U2_DISTRIBUTED_COMPUTING_UTIL_H
#define _U2_DISTRIBUTED_COMPUTING_UTIL_H

#include <QtCore/QObject>

#include "RemoteMachine.h"

namespace U2 {

    class PingTask;

    /* RemoteMachine functions are blocking, so this task can be useful, 
    * because it waits for remote machine responses in separate thread 
    */
    class RetrieveRemoteMachineInfoTask : public Task {
        Q_OBJECT
    public:
        RetrieveRemoteMachineInfoTask( RemoteMachineSettings* s);
        ~RetrieveRemoteMachineInfoTask();

        virtual void prepare();
        virtual void run();
        virtual ReportResult report();

        QStringList getServicesList() const;
        //QUuid getUuid() const;
        QString getHostName() const;
        bool isPingOk() const;
        RemoteMachineSettings* getMachineSettings() const {return settings;}

    private:
        QStringList             services;
        QString                 hostname;
        PingTask*               pingTask;
        bool                    pingOK;
        RemoteMachine *         machine;
        RemoteMachineSettings*  settings;
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

        QList< RemoteMachineSettings* > getPublicMachines() const;
        QList< RemoteMachineSettings* > takePublicMachines();

    private:
        void processEncodedMachines( const QString & encodedMachines );

    private:
        QList< RemoteMachineSettings* > publicMachines;

    }; // RetrievePublicMachinesTask

    /* Save remote machine settings to file 
    */
    class SaveRemoteMachineSettings : public Task {
        Q_OBJECT
    public:
        SaveRemoteMachineSettings(RemoteMachineSettings * machineSettings, const QString& filename);
        virtual void run();

    private:
        QByteArray data;
        QString filename;

    }; // SaveRemoteMachineSettings

} // U2

#endif // _U2_DISTRIBUTED_COMPUTING_UTIL_H
