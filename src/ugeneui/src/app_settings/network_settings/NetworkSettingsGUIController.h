#ifndef _U2_NETWORK_SETTINGS_WIDGET_IMPL_H_
#define _U2_NETWORK_SETTINGS_WIDGET_IMPL_H_

#include <ui/ui_NetworkSettingsWidget.h>

#include <U2Core/NetworkConfiguration.h>
#include <U2Gui/AppSettingsGUI.h>

#include <QtCore/QUrl>

namespace U2 {


class NetworkSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    NetworkSettingsPageController(QObject* p = NULL);

    virtual AppSettingsGUIPageState* getSavedState();

    virtual void saveState(AppSettingsGUIPageState* s);

    virtual AppSettingsGUIPageWidget* createWidget(AppSettingsGUIPageState* data);
};


class NetworkSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    NetworkSettingsPageState();
    NetworkConfiguration config;
};


class NetworkSettingsPageWidget: public AppSettingsGUIPageWidget, public Ui_NetworkSettingsWidget {
    Q_OBJECT
public:
    NetworkSettingsPageWidget();

    virtual void setState(AppSettingsGUIPageState* state);

    virtual AppSettingsGUIPageState* getState(QString& err) const;

private slots:
    void sl_HttpChecked( int state );
    void sl_ExceptionsChecked( int state );
};

}//namespace

#endif
