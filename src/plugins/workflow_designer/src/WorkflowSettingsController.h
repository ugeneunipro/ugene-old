#ifndef _U2_W_SETTINGS_GUI_CONTROLLER_H_
#define _U2_W_SETTINGS_GUI_CONTROLLER_H_

#include <ui/ui_WorkflowSettingsWidget.h>

#include <U2Gui/AppSettingsGUI.h>

namespace U2 {

#define WorkflowSettingsPageId QString("wds")

class WorkflowSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    WorkflowSettingsPageController(QObject* p = NULL);

    virtual AppSettingsGUIPageState* getSavedState();

    virtual void saveState(AppSettingsGUIPageState* s);

    virtual AppSettingsGUIPageWidget* createWidget(AppSettingsGUIPageState* data);
};


class WorkflowSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    bool showGrid;
    bool snap2grid;
    bool lockRun;
    //bool failFast;
    QString style;
    QFont font;
    QString path;
    QColor color;
    bool showEmptyPorts;
    bool runSchemaInSeparateProcess;
};


class WorkflowSettingsPageWidget: public AppSettingsGUIPageWidget, public Ui_WorkflowSettingsWidget {
    Q_OBJECT
public:
    WorkflowSettingsPageWidget(WorkflowSettingsPageController* ctrl);

    virtual void setState(AppSettingsGUIPageState* state);

    virtual AppSettingsGUIPageState* getState(QString& err) const;

    virtual bool eventFilter( QObject * watched, QEvent * event ); 

private slots:
    void sl_getDirectory();
    void sl_getColor();
};

}//namespace

#endif
