#ifndef _U2_PLUGIN_VEIWER_IMPL_H_

#include <U2Gui/PluginViewer.h>
#include <U2Core/Task.h>

namespace U2 {

class PluginViewerController;
class EnablePluginViewerTask;
class DisablePluginViewerTask;

class PluginViewerImpl : public PluginViewer {
    Q_OBJECT

    friend class DisablePluginViewerTask;
	friend class EnablePluginViewerTask;
public:
    PluginViewerImpl() : PluginViewer(tr("plugin_viewer_sname"), tr("plugin_viewer_sdesc")) {viewer = NULL;}

protected:
	virtual Task* createServiceEnablingTask();

	virtual Task* createServiceDisablingTask();

private:
    PluginViewerController* viewer;

};

//////////////////////////////////////////////////////////////////////////
/// Tasks
class EnablePluginViewerTask : public Task {
    Q_OBJECT

public:
	EnablePluginViewerTask(PluginViewerImpl* pv);
	virtual ReportResult report();
private:
	PluginViewerImpl* pv;
};


class DisablePluginViewerTask : public Task {
    Q_OBJECT

public:
	DisablePluginViewerTask(PluginViewerImpl* pv);
	virtual ReportResult report();
private:
	PluginViewerImpl* pv;
};


}//namespace

#endif
