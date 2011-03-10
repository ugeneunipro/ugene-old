#include "PluginViewerImpl.h"
#include <U2Gui/MainWindow.h>

#include <AppContextImpl.h>
#include "PluginViewerController.h"

#include <QtGui/QAction>

namespace U2 {

Task* PluginViewerImpl::createServiceEnablingTask() {
	return new EnablePluginViewerTask(this);
}

Task* PluginViewerImpl::createServiceDisablingTask() {
	return new DisablePluginViewerTask(this);	
}

//////////////////////////////////////////////////////////////////////////
// tasks

EnablePluginViewerTask::EnablePluginViewerTask(PluginViewerImpl* _pv)
: Task (tr("enable_plugin_viewer_task"), TaskFlag_NoRun), pv(_pv)
{
}

Task::ReportResult EnablePluginViewerTask::report() {
	AppContextImpl::getApplicationContext()->setPluginViewer(pv);
	pv->viewer = new PluginViewerController();
	return ReportResult_Finished;
}


DisablePluginViewerTask::DisablePluginViewerTask(PluginViewerImpl* _pv)
: Task (tr("disable_plugin_viewer_task"), TaskFlag_NoRun), pv(_pv)
{
}

Task::ReportResult DisablePluginViewerTask::report() {
    AppContextImpl::getApplicationContext()->setPluginViewer(NULL);
    delete pv->viewer;
    pv->viewer = NULL;
	return ReportResult_Finished;
}


}//namespace
