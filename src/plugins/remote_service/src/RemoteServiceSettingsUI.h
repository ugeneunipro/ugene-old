#ifndef _REMOTE_SERVICE_SETTINGS_UI_H_
#define _REMOTE_SERVICE_SETTINGS_UI_H_

#include <U2Remote/ProtocolUI.h>
#include "ui/ui_RemoteServiceSupportUI.h"

namespace U2 {

class RemoteServiceMachineSettings;

class RemoteServiceSettingsUI : public ProtocolUI, public Ui::RemoteServiceSupportUI
{
    Q_OBJECT
public:
    RemoteServiceSettingsUI();
    virtual ~RemoteServiceSettingsUI();

    virtual RemoteMachineSettings * createMachine() const;
    virtual void initializeWidget(const RemoteMachineSettings *settings);
    virtual void clearWidget();
    virtual QDialog* createUserTasksDialog(const RemoteMachineSettings* settings, QWidget* parent);
    virtual QString validate() const;

};

} // namespace U2

#endif // _REMOTE_SERVICE_SETTINGS_UI_H_
