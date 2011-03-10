#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Gui/AppSettingsGUI.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/Settings.h>
#include <U2Core/ExternalToolRegistry.h>

#include <QtCore/QSettings>
#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtGui/QStyleFactory>
#include <QtGui/QMessageBox>

namespace U2 {

#define NUMBER_EXTERNAL_TOOL SETTINGS + "numberExternalTools"
#define PREFIX_EXTERNAL_TOOL_NAME SETTINGS + "exToolName"
#define PREFIX_EXTERNAL_TOOL_PATH SETTINGS + "exToolPath"
#define PREFIX_EXTERNAL_TOOL_IS_VALID SETTINGS + "exToolIsValid"
#define PREFIX_EXTERNAL_TOOL_VERSION SETTINGS + "exToolVersion"
#define TEMPORY_DIRECTORY SETTINGS + "temporyDirectory"

Watcher* const ExternalToolSupportSettings::watcher = new Watcher;

int ExternalToolSupportSettings::prevNumberExternalTools = 0;

int ExternalToolSupportSettings::getNumberExternalTools() {
    return AppContext::getSettings()->getValue(NUMBER_EXTERNAL_TOOL, 0).toInt();
}

void ExternalToolSupportSettings::setNumberExternalTools( int v ) {
    AppContext::getSettings()->setValue(NUMBER_EXTERNAL_TOOL, v);
    emit watcher->changed();
}

bool ExternalToolSupportSettings::getExternalTools() {
    int numberExternalTools=getNumberExternalTools();
    QString name;
    QString path;
    bool isValid;
    QString version;
    for(int i=0; i<numberExternalTools;i++){
        name=AppContext::getSettings()->getValue(PREFIX_EXTERNAL_TOOL_NAME + QString::number(i), QVariant("")).toString();
        path=AppContext::getSettings()->getValue(PREFIX_EXTERNAL_TOOL_PATH + QString::number(i), QVariant("")).toString();
        isValid=AppContext::getSettings()->getValue(PREFIX_EXTERNAL_TOOL_IS_VALID + QString::number(i), QVariant(false)).toBool();
        version=AppContext::getSettings()->getValue(PREFIX_EXTERNAL_TOOL_VERSION + QString::number(i), QVariant("unknown")).toString();
        if(AppContext::getExternalToolRegistry()->getByName(name) != NULL){
            AppContext::getExternalToolRegistry()->getByName(name)->setValid(isValid);
            AppContext::getExternalToolRegistry()->getByName(name)->setPath(path);
            AppContext::getExternalToolRegistry()->getByName(name)->setVersion(version);
        }
    }
    prevNumberExternalTools = numberExternalTools;
    return true;//bad code
}

void ExternalToolSupportSettings::setExternalTools() {
    QList<ExternalTool*> ExternalToolList = AppContext::getExternalToolRegistry()->getAllEntries();
    int numberExternalTools = ExternalToolList.length();
    setNumberExternalTools(numberExternalTools);
    QString name;
    QString path;
    bool isValid;
    QString version;
    int numberIterations=numberExternalTools >= prevNumberExternalTools ? numberExternalTools : prevNumberExternalTools;
    for(int i=0; i<numberIterations;i++){
        if(i<numberExternalTools){
            name = ExternalToolList.at(i)->getName();
            path = ExternalToolList.at(i)->getPath();
            isValid = ExternalToolList.at(i)->isValid();
            version = ExternalToolList.at(i)->getVersion();
            AppContext::getSettings()->setValue(PREFIX_EXTERNAL_TOOL_NAME + QString::number(i), name);
            AppContext::getSettings()->setValue(PREFIX_EXTERNAL_TOOL_PATH + QString::number(i), path);
            AppContext::getSettings()->setValue(PREFIX_EXTERNAL_TOOL_IS_VALID + QString::number(i), isValid);
            AppContext::getSettings()->setValue(PREFIX_EXTERNAL_TOOL_VERSION + QString::number(i), version);
        }else{
            AppContext::getSettings()->remove(PREFIX_EXTERNAL_TOOL_NAME + QString::number(i));
            AppContext::getSettings()->remove(PREFIX_EXTERNAL_TOOL_PATH + QString::number(i));
            AppContext::getSettings()->remove(PREFIX_EXTERNAL_TOOL_IS_VALID + QString::number(i));
            AppContext::getSettings()->remove(PREFIX_EXTERNAL_TOOL_VERSION + QString::number(i));
        }
    }
    prevNumberExternalTools = numberExternalTools;
}

void ExternalToolSupportSettings::checkTemporaryDir(){
    if(AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath().isEmpty()){
        QMessageBox msgBox;
        msgBox.setWindowTitle(QObject::tr("Path for temporary files"));
        msgBox.setText(QObject::tr("Path for temporary files not selected."));
        msgBox.setInformativeText(QObject::tr("Do you want to select it now?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret) {
           case QMessageBox::Yes:
               AppContext::getAppSettingsGUI()->showSettingsDialog(APP_SETTINGS_USER_APPS);
               break;
           case QMessageBox::No:
               return;
               break;
           default:
               assert(NULL);
               break;
         }
    }
}

}//namespace
