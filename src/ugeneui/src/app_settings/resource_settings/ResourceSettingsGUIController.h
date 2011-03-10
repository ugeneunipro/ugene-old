#ifndef _U2_RESOURCE_SETTINGS_GUI_CONTROLLER_H_
#define _U2_RESOURCE_SETTINGS_GUI_CONTROLLER_H_

#include <ui/ui_ResourceSettingsWidget.h>

#include <U2Core/NetworkConfiguration.h>
#include <U2Gui/AppSettingsGUI.h>

#include <QtCore/QUrl>

namespace U2 {


class ResourceSettingsGUIPageController: public AppSettingsGUIPageController {
    Q_OBJECT
public:
    ResourceSettingsGUIPageController(QObject* p = NULL);

    virtual AppSettingsGUIPageState* getSavedState();

    virtual void saveState(AppSettingsGUIPageState* s);

    virtual AppSettingsGUIPageWidget* createWidget(AppSettingsGUIPageState* data);
};


class ResourceSettingsGUIPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    ResourceSettingsGUIPageState() : nCpus(0), nThreads(0), maxMem(0){}
    int nCpus;
    int nThreads;
    int maxMem;
};


class ResourceSettingsGUIPageWidget: public AppSettingsGUIPageWidget, public Ui_ResourceSettingsWidget {
    Q_OBJECT
public:
    ResourceSettingsGUIPageWidget(ResourceSettingsGUIPageController* ctrl);

    virtual void setState(AppSettingsGUIPageState* state);

    virtual AppSettingsGUIPageState* getState(QString& err) const;

private slots:
    void sl_threadsCountChanged(int n);
    void sl_cpuCountChanged(int n);
};

}//namespace

#endif
