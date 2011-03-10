#ifndef __CUDA_SUPPORT_SETTINGS_CONTROLLER__
#define __CUDA_SUPPORT_SETTINGS_CONTROLLER__

#include <QtCore/QVector>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>

#include <U2Gui/AppSettingsGUI.h>

namespace U2 {

#define CudaSupportSettingsPageId QString("css")

class CudaSupportSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    CudaSupportSettingsPageController( const QString & _displayMsg, QObject * p = 0 );

    virtual AppSettingsGUIPageState * getSavedState();

    virtual void saveState( AppSettingsGUIPageState * s );

    virtual AppSettingsGUIPageWidget * createWidget( AppSettingsGUIPageState* state );

private:
    QString displayMsg;
};

class CudaSupportSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    CudaSupportSettingsPageState( int num_gpus );
    QVector<bool> enabledGpus;
};

class CudaSupportSettingsPageWidget : public AppSettingsGUIPageWidget {
    Q_OBJECT
public:
    CudaSupportSettingsPageWidget( const QString & _msg, CudaSupportSettingsPageController * ctrl );

    virtual void setState( AppSettingsGUIPageState * state );

    virtual AppSettingsGUIPageState* getState( QString & err ) const;

    QString onlyMsg;

    QList<QCheckBox *> gpuEnableChecks;
};

} //namespace

#endif //__CUDA_SUPPORT_SETTINGS_CONTROLLER__
