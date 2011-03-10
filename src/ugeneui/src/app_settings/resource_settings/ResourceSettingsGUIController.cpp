#include "ResourceSettingsGUIController.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>

namespace U2
{
#define TRANSMAP_FILE_NAME "translations.txt"

ResourceSettingsGUIPageController::ResourceSettingsGUIPageController(QObject* p) 
: AppSettingsGUIPageController(tr("Resources"), APP_SETTINGS_RESOURCES, p)
{
}


AppSettingsGUIPageState* ResourceSettingsGUIPageController::getSavedState() {
    ResourceSettingsGUIPageState* state = new ResourceSettingsGUIPageState();
    AppResourcePool* s = AppContext::getAppSettings()->getAppResourcePool();
    state->nCpus = s->getIdealThreadCount();
    state->nThreads = s->getMaxThreadCount();
    state->maxMem = s->getMaxMemorySizeInMB();
    return state;
}

void ResourceSettingsGUIPageController::saveState(AppSettingsGUIPageState* _state) {
    ResourceSettingsGUIPageState* state = qobject_cast<ResourceSettingsGUIPageState*>(_state);
    AppResourcePool* s = AppContext::getAppSettings()->getAppResourcePool();
    assert(state->nCpus <= state->nThreads);
    s->setIdealThreadCount(state->nCpus);
    s->setMaxThreadCount(state->nThreads);
    s->setMaxMemorySizeInMB(state->maxMem);
}

AppSettingsGUIPageWidget* ResourceSettingsGUIPageController::createWidget(AppSettingsGUIPageState* state) {
    ResourceSettingsGUIPageWidget* r = new ResourceSettingsGUIPageWidget(this);
    r->setState(state);
    return r;
}

ResourceSettingsGUIPageWidget::ResourceSettingsGUIPageWidget(ResourceSettingsGUIPageController*) {
    setupUi(this);
    connect(threadBox, SIGNAL(valueChanged(int)), SLOT(sl_threadsCountChanged(int)));
    connect(cpuBox, SIGNAL(valueChanged(int)), SLOT(sl_cpuCountChanged(int)));
}

void ResourceSettingsGUIPageWidget::setState(AppSettingsGUIPageState* s) {
    ResourceSettingsGUIPageState* state = qobject_cast<ResourceSettingsGUIPageState*>(s);
    cpuBox->setValue(state->nCpus);
    threadBox->setValue(state->nThreads);
    memBox->setValue(state->maxMem);
}

AppSettingsGUIPageState* ResourceSettingsGUIPageWidget::getState(QString& err) const {
    Q_UNUSED(err);
    ResourceSettingsGUIPageState* state = new ResourceSettingsGUIPageState();
    state->nCpus =cpuBox->value();
    state->nThreads = threadBox->value();
    state->maxMem = memBox->value();
    return state;
}

void ResourceSettingsGUIPageWidget::sl_threadsCountChanged(int n) {
    if (cpuBox->value() > n) {
        cpuBox->setValue(n);
    }
}

void ResourceSettingsGUIPageWidget::sl_cpuCountChanged(int n) {
    if (threadBox->value() < n) {
        threadBox->setValue(n);
    }
}

} //namespace
