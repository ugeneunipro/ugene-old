#ifndef _U2_CMD_LINE_REGISTRY_
#define _U2_CMD_LINE_REGISTRY_

#include <U2Core/global.h>

#include <QtCore/QStringList>
#include <QtCore/QMap>

namespace U2 {

class CMDLineHelpProvider;

typedef QPair<QString, QString> StringPair;

/*  Reads command arguments into UGENE's command line parameters 
    
    Convention:
    --paramName=paramValue -> (paramName, paramValue)
    -paramName paramValue -> (paramName, paramValue)
    paramValue1 paramValue2  -> ("", paramValue1), ("", paramValue2)
*/
class U2CORE_EXPORT CMDLineRegistry : public QObject {
    Q_OBJECT
public:
    CMDLineRegistry(const QStringList& arguments);
    virtual ~CMDLineRegistry();
    
    const QList<StringPair> & getParameters() const;
    // as they were in cmdline. Empty keys also here
    QStringList getOrderedParameterNames() const;
    
    // finding starts at startsWithIdx
    bool hasParameter( const QString & paramName, int startsWithIdx = 0 ) const;
    // returns value of first appearance from startWithIdx of paramName key
    QString getParameterValue( const QString & paramName, int startWithIdx = 0 ) const;
    
    void registerCMDLineHelpProvider(CMDLineHelpProvider* provider);
    void unregisterCMDLineHelpProvider(CMDLineHelpProvider* provider);

    const QList<CMDLineHelpProvider* >& listCMDLineHelpProviders() const { return helpProviders; }
        
private:
    QList<StringPair>                   params; // pairs (paramName, paramValue) ordered as in the cmdline
    QList<CMDLineHelpProvider* >        helpProviders; // sorted by section name
    
}; // CMDLineRegistry

} //namespace U2

#endif // _U2_CMD_LINE_REGISTRY_
