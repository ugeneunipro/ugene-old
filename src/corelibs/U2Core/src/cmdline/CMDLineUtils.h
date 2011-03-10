#ifndef _U2_CMDLINE_UTILS_H_
#define _U2_CMDLINE_UTILS_H_

#include <QtCore/QString>
#include <QtCore/QList>

#include <U2Core/CMDLineRegistry.h>

namespace U2 {

class U2CORE_EXPORT CMDLineRegistryUtils {
public:
    // -1 if not found
    static int getParameterIndex( const QString & paramName, int startWith = 0 );
    // in case: '-t a b c' return {a, b, c}
    static QStringList getParameterValues( const QString & paramName, int startWith = 0 );
    // in case '-t "a b" c' return {a, b, c} not {"a b", c}
    static QStringList getParameterValuesByWords( const QString & paramName, int startWith = 0 );
    // return list of all values that doesn't have keys ("", paramValue)
    // search starts at startWithIdx, stops at first (paramKey, paramValue) pair
    // by default, search starts at 1 because at params[0] is usually ("", programName) pair
    static QStringList getPureValues( int startWithIdx = 1 );

private:
    static void setCMDLineParams( QList<StringPair> & to );

}; // CMDLineRegistryUtils

class U2CORE_EXPORT CMDLineUtils {
public:
    static void init();
    
}; // CMDLineUtils

} // U2

#endif // _U2_CMDLINE_UTILS_H_
