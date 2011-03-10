
#ifndef _U2_REMOTE_MACHINE_SCANNER_H_
#define _U2_REMOTE_MACHINE_SCANNER_H_

#include <QtCore/QList>

#include "RemoteMachine.h"

namespace U2 {

/*
 * Base class. Each transport protocol that can scan remote machines should implement this
 */
class U2REMOTE_EXPORT RemoteMachineScanner : public QObject {
public:
    RemoteMachineScanner( bool filter = true );
    virtual ~RemoteMachineScanner();
    
    virtual void startScan() = 0;
    /* creates new allocated list of machine settings. caller should delete them. returned machines - newly scanned since startScan() */
    virtual QList< RemoteMachineSettings * > takeScanned() = 0;
    
    virtual void setFilterLocalHost( bool filter );
    virtual bool getFilterLocalHost() const;
    
protected:
    bool filterLocalHost;
    
}; // RemoteMachineScanner

} // U2

#endif // _U2_REMOTE_MACHINE_SCANNER_H_
