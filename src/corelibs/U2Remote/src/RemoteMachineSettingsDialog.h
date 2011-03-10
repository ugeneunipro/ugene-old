#ifndef _U2_REMOTE_MACHINE_SETTINGS_DIALOG_H_
#define _U2_REMOTE_MACHINE_SETTINGS_DIALOG_H_

#include <QtGui/QDialog>

#include <U2Remote/RemoteMachine.h>
#include <U2Remote/ProtocolUI.h>

#include <ui/ui_RemoteMachineSettingsDialog.h>

namespace U2 {

class RemoteMachineSettingsDialog : public QDialog, public Ui::RemoteMachineSettingsDialog {
    Q_OBJECT
public:
    RemoteMachineSettingsDialog(QWidget* parent, RemoteMachineSettings* settings = NULL);
    ~RemoteMachineSettingsDialog();
    
    RemoteMachineSettings * getMachineSettings() const;

private slots:
    void sl_okPushButtonClicked();
    
private:
    void showErrorLabel(const QString& error);
    void createMachineSettings();
    RemoteMachineSettings *         machineSettings;
    QString                         protoId;
    ProtocolUI *                    currentUi;
    
}; // RemoteMachineSettingsDialog

} // U2

#endif // _U2_REMOTE_MACHINE_SETTINGS_DIALOG_H_
