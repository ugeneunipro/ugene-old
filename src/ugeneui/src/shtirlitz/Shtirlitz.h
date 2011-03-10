#ifndef __SHTIRLITZ_H__
#define __SHTIRLITZ_H__

#include <QtCore/QUuid>
#include <U2Core/Task.h>

namespace U2{

class Shtirlitz;

//The main responsibility of Shtirlitz is to collect and send reports about
//the system where UGENE is installed and about UGENE usage.
class Shtirlitz {
public:
    //Sends all of the reports, if needed. 
    static void wakeup();
    //Sends custom reports, created by somebody other than Shtirlitz.
    static void sendCustomReport( const QString & customReport );
    //Saves to settings gathered during the current launch info
    static void saveGatheredInfo();
private:
    //Creates and sends counters info (statistics about UGENE main tasks usage)
    static void sendCountersReport();
    //Creates and sends system info: OS, hardware platform, etc.
    static void sendSystemReport();

    static QString formCountersReport();
    static QString formSystemReport(); 

    static void getOsNameAndVersion( QString & name, QString & version );

    static void getFirstLaunchInfo(bool& thisVersion, bool& allVersions);
    static bool enabled();

    //ugly stub for convenience - calls ShtirlitzPlugin::tr
    static QString tr( const char * str );

private:
    //loads uuid from settings if necessary
    //creates the new one if nothing was found
    static QUuid getUniqueUgeneId();
};

//Task which performs sending of data
class ShtirlitzTask : public Task {
    Q_OBJECT
public:
    ShtirlitzTask( const QString & _report );
    void run();
private:
    QString report;
};

} // U2


#endif
