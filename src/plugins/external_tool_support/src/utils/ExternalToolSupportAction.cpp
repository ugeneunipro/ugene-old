/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "ExternalToolSupportAction.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>


namespace U2 {

ExternalToolSupportAction::ExternalToolSupportAction(QObject* p, GObjectView* v, const QString& _text, int order, const QStringList& _toolNames)
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
       (AppContext::getAppSettings()->getUserAppsSettings()->getUserTemporaryDirPath().isEmpty())){
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

ExternalToolSupportAction::ExternalToolSupportAction(const QString& _text, QObject* p, const QStringList& _toolNames)
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
       (AppContext::getAppSettings()->getUserAppsSettings()->getUserTemporaryDirPath().isEmpty())){
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

void ExternalToolSupportAction::sl_pathChanged() {
    QFont isConfiguredToolFont;
    bool isOneOfToolConfigured=false;
    foreach(QString toolName, toolNames){
        if(!AppContext::getExternalToolRegistry()->getByName(toolName)->getPath().isEmpty()){
            isOneOfToolConfigured=true;
        }
    }
    if(!isOneOfToolConfigured ||
       (AppContext::getAppSettings()->getUserAppsSettings()->getUserTemporaryDirPath().isEmpty())){
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
