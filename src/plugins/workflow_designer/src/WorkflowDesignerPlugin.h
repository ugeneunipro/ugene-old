/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_FLOWCHART_H_
#define _U2_FLOWCHART_H_

#include <U2Core/PluginModel.h>
#include <U2Core/ServiceModel.h>
#include <U2Core/Task.h>

#include <U2Remote/RemoteWorkflowRunTask.h>

class QAction;
class QEvent;

namespace U2 {

class WorkflowDesignerPlugin : public Plugin {
    Q_OBJECT
public:
    static const QString RUN_WORKFLOW;
    static const QString REMOTE_MACHINE;
    static const QString PRINT;
    
public:
    WorkflowDesignerPlugin ();
    //~WorkflowDesignerPlugin ();
private:
    void registerCMDLineHelp();
    void registerWorkflowTasks();
    void processCMDLineOptions();
    
private slots:
    void sl_saveSchemaImageTaskFinished();
    
};

class WorkflowDesignerService : public Service {
    Q_OBJECT
public:
    WorkflowDesignerService();
    bool closeViews();
protected:
    virtual Task* createServiceEnablingTask();

    virtual Task* createServiceDisablingTask();

    virtual void serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged);

private slots:
    void sl_showDesignerWindow();
    void sl_showManagerWindow();
    void sl_startWorkflowPlugin();

private:
    QAction*        designerAction;
    QAction*        managerAction;
};


} //namespace

#endif
