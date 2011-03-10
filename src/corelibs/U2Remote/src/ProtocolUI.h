
#ifndef _U2_PROTOCOL_UI_H_
#define _U2_PROTOCOL_UI_H_

#include <cassert>

#include <QtCore/QList>
#include <QtGui/QWidget>
#include "RemoteMachine.h"

namespace U2 {

/*
 * Base class for GUI form with remote machine info
 */
class U2REMOTE_EXPORT ProtocolUI : public QWidget {
public:
    ProtocolUI();
    virtual ~ProtocolUI();
    
    // after user clicks 'ok', creates remote machine settings based on the info in the form. Caller should delete it
    virtual RemoteMachineSettings * createMachine() const = 0;
    virtual void initializeWidget( const RemoteMachineSettings *settings ) = 0;
    virtual void clearWidget() = 0;
    virtual QString validate() const = 0;
    virtual QDialog* createUserTasksDialog(const RemoteMachineSettings* settings, QWidget* parent) = 0;
    
private:
    ProtocolUI( const ProtocolUI & );
    ProtocolUI & operator=( const ProtocolUI & );
    
}; // ProtocolUI



} // U2

#endif // _U2_PROTOCOL_UI_H
