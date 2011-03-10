#ifndef _U2_CMDLINE_OPTIONS_H_
#define _U2_CMDLINE_OPTIONS_H_

#include <QtCore/QObject>
#include <QtCore/QString>
#include <U2Core/global.h>

namespace U2 {

// QObject needed for translations
class U2CORE_EXPORT CMDLineCoreOptions : public QObject {
    Q_OBJECT
public:
    static const QString INI_FILE;
    static const QString SUITE_URLS;
    static const QString TEST_REPORT;
    static const QString STAY_ALIVE;
    static const QString HELP;
    static const QString TRANSLATION;
    static const QString TEST_THREADS;
    static const QString TEAMCITY_OUTPUT;
    
public:
    // initialize help for core cmdline options
    static void initHelp();
    
}; // CMDLineCoreOptions

} // U2

#endif // _U2_CMDLINE_OPTIONS_H_
