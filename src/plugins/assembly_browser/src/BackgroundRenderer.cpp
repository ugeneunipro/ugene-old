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
renderTask(0)
{
}

BackgroundRenderer::~BackgroundRenderer() {
    if(renderTask != NULL && renderTask->isRunning()) {
        renderTask->cancel();
    }
}

void BackgroundRenderer::render(BackgroundRenderTask * task)  {
    if(renderTask) {
        renderTask->cancel();
    }
    renderTask = task;
    connect(renderTask, SIGNAL(si_stateChanged()), SLOT(sl_redrawFinished()));
    AppContext::getTaskScheduler()->registerTopLevelTask(renderTask);
}

QImage BackgroundRenderer::getImage() const {
    if(renderTask != NULL) {
        return QImage();
    }
    return result;
}

void BackgroundRenderer::sl_redrawFinished() {
    BackgroundRenderTask * senderr = qobject_cast<BackgroundRenderTask*>(sender());
    assert(senderr);
    if(renderTask != senderr) {
        return;
    }
    if(Task::State_Finished != senderr->getState()) {
        return;
    }
    result = renderTask->getResult();
    renderTask = NULL;
    emit(si_rendered());
}

}
