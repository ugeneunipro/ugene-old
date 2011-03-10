#ifndef __OPENCL_SUPPORT_SETTINGS_CONTROLLER__
#define __OPENCL_SUPPORT_SETTINGS_CONTROLLER__

#include <QtCore/QVector>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>

#include <U2Gui/AppSettingsGUI.h>

namespace U2 {

#define OpenCLSupportSettingsPageId QString("oclss")

class OpenCLSupportSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    OpenCLSupportSettingsPageController( const QString & _displayMsg, QObject * p = 0 );

    virtual AppSettingsGUIPageState * getSavedState();

    virtual void saveState( AppSettingsGUIPageState * s );

    virtual AppSettingsGUIPageWidget * createWidget( AppSettingsGUIPageState* state );

private:
    QString displayMsg;
};

class OpenCLSupportSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    OpenCLSupportSettingsPageState( int num_gpus );
    QVector<bool> enabledGpus;
};

class OpenCLSupportSettingsPageWidget : public AppSettingsGUIPageWidget {
    Q_OBJECT
public:
    OpenCLSupportSettingsPageWidget( const QString & _msg, OpenCLSupportSettingsPageController * ctrl );

    virtual void setState( AppSettingsGUIPageState * state );

    virtual AppSettingsGUIPageState* getState( QString & err ) const;

    QString onlyMsg;

    QList<QCheckBox *> gpuEnableChecks;
};

} //namespace

#endif //__OPENCL_SUPPORT_SETTINGS_CONTROLLER__
