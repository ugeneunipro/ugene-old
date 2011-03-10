#include "ExternalToolSupportAction.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>


namespace U2 {

ExternalToolSupprotAction::ExternalToolSupprotAction(QObject* p, GObjectView* v, const QString& _text, int order, const QStringList& _toolNames)
    : GObjectViewAction(p,v,_text,order), toolNames(_toolNames)
{
    QFont isConfiguredToolFont;
    bool isOneOfToolConfigured=false;
    foreach(QString toolName, toolNames){
        if(!AppContext::getExternalToolRegistry()->getByName(toolName)->getPath().isEmpty()){
            isOneOfToolConfigured=true;
        }
        ExternalTool* exTool=AppContext::getExternalToolRegistry()->getByName(toolName);
        connect(exTool, SIGNAL(si_pathChanged()), SLOT(sl_pathChanged()));
    }
    connect(AppContext::getAppSettings()->getUserAppsSettings(), SIGNAL(si_temporaryPathChanged()), SLOT(sl_pathChanged()));
    if(!isOneOfToolConfigured ||
       (AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath().isEmpty())){
        isConfiguredToolFont.setItalic(true);
        setFont(isConfiguredToolFont);
//        setText(text()+"...");
        setIcon(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->getGrayIcon());
    }else{
        isConfiguredToolFont.setItalic(false);
        setFont(isConfiguredToolFont);
//        if(text().endsWith("...")){
//            setText(text().remove(text().length()-3,3));
//        }else{
//            setText(text());
//        }
        if(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->isValid()){
            setIcon(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->getIcon());
        }else{
            setIcon(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->getWarnIcon());
        }
    }
}

ExternalToolSupprotAction::ExternalToolSupprotAction(const QString& _text, QObject* p, const QStringList& _toolNames)
    : GObjectViewAction(p, NULL, _text), toolNames(_toolNames)
{
    QFont isConfiguredToolFont;
    bool isOneOfToolConfigured=false;
    foreach(QString toolName, toolNames){
        if(!AppContext::getExternalToolRegistry()->getByName(toolName)->getPath().isEmpty()){
            isOneOfToolConfigured=true;
        }
        ExternalTool* exTool=AppContext::getExternalToolRegistry()->getByName(toolName);
        connect(exTool, SIGNAL(si_pathChanged()), SLOT(sl_pathChanged()));
    }
    connect(AppContext::getAppSettings()->getUserAppsSettings(), SIGNAL(si_temporaryPathChanged()), SLOT(sl_pathChanged()));
    if(!isOneOfToolConfigured ||
       (AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath().isEmpty())){
        isConfiguredToolFont.setItalic(true);
        setFont(isConfiguredToolFont);
//        setText(text()+"...");
        setIcon(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->getGrayIcon());
    }else{
        isConfiguredToolFont.setItalic(false);
        setFont(isConfiguredToolFont);
//        if(text().endsWith("...")){
//            setText(text().remove(text().length()-3,3));
//        }else{
//            setText(text());
//        }
        if(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->isValid()){
            setIcon(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->getIcon());
        }else{
            setIcon(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->getWarnIcon());
        }
    }
}

void ExternalToolSupprotAction::sl_pathChanged() {
    QFont isConfiguredToolFont;
    bool isOneOfToolConfigured=false;
    foreach(QString toolName, toolNames){
        if(!AppContext::getExternalToolRegistry()->getByName(toolName)->getPath().isEmpty()){
            isOneOfToolConfigured=true;
        }
    }
    if(!isOneOfToolConfigured ||
       (AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath().isEmpty())){
        isConfiguredToolFont.setItalic(true);
        setFont(isConfiguredToolFont);
//        if(!text().endsWith("...")){
//            setText(text()+"...");
//        }
        setIcon(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->getGrayIcon());
    }else{
        isConfiguredToolFont.setItalic(false);
        setFont(isConfiguredToolFont);
//        if(text().endsWith("...")){
//            setText(text().remove(text().length()-3,3));
//        }
        if(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->isValid()){
            setIcon(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->getIcon());
        }else{
            setIcon(AppContext::getExternalToolRegistry()->getByName(toolNames.at(0))->getWarnIcon());
        }
    }
}
}//namespace
