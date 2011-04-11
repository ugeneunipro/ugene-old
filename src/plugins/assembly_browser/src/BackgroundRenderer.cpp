#include "BackgroundRenderer.h"

#include <QtGui/QPainter>

#include <U2Core/AppContext.h>

#include "AssemblyBrowser.h"

namespace U2 {

//==============================================================================
// BackgroundRenderTask
//==============================================================================
BackgroundRenderTask::BackgroundRenderTask(const QString& _name, TaskFlags f) : 
Task(_name, f) {
}

//==============================================================================
// BackgroundRenderer
//==============================================================================

BackgroundRenderer::BackgroundRenderer() :
renderTask(0), redrawRunning(false), redrawNeeded(true)
{
}

void BackgroundRenderer::render(BackgroundRenderTask * task)  {
    renderTask = task;
    if(redrawRunning) {
        assert(renderTask);
        redrawNeeded = true;
        return;
    }    
    redrawRunning = true;
    redrawNeeded = false;
    connect(renderTask, SIGNAL(si_stateChanged()), SLOT(sl_redrawFinished()));
    AppContext::getTaskScheduler()->registerTopLevelTask(renderTask);
}

QImage BackgroundRenderer::getImage() const {
    if(redrawRunning) {
        return QImage();
    }
    return result;
}

void BackgroundRenderer::sl_redrawFinished() {
    BackgroundRenderTask * senderr = qobject_cast<BackgroundRenderTask*>(sender());
    assert(senderr);
    if(Task::State_Finished != senderr->getState()) {
        return;
    }
    assert(redrawRunning);
    redrawRunning = false;
    if(redrawNeeded) {
        assert(renderTask != senderr);
        render(renderTask);
        redrawRunning = true;
        redrawNeeded = false;
    } else {
        assert(renderTask == senderr);
        result = renderTask->getResult();
        emit(si_rendered());
    }
}

}
