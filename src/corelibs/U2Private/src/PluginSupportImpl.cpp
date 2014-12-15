/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "PluginSupportImpl.h"
#include "ServiceRegistryImpl.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QLibrary>
#include <QtCore/QDir>
#include <QtCore/QSet>

#include <U2Gui/MainWindow.h>

#include <algorithm>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace U2 {

/* TRANSLATOR U2::PluginSupportImpl */
#define PLUGINS_LIST_SETTINGS QString("plugin_support/list/")
#define SKIP_LIST_SETTINGS QString("plugin_support/skip_list/")
#define PLUGINS_ACCEPTED_LICENSE_LIST QString("plugin_support/accepted_list/")

QString PluginSupportImpl::versionAppendix("");
const QString PluginSupportImpl::OPENCL_CHECKED_SETTINGS("plugin_support/opencl/");
static QStringList findAllPluginsInDefaultPluginsDir();


PluginRef::PluginRef(Plugin* _plugin, QLibrary* _library, const PluginDesc& desc)
: plugin(_plugin), library(_library), pluginDesc(desc), removeFlag(false)
{
}

PluginSupportImpl::PluginSupportImpl(bool testingMode): allLoaded(false) {
    //read plugin names from settings
    Settings* settings = AppContext::getSettings();

    QSet<QString> pluginFiles = getPluginPaths();

    connect(this, SIGNAL(si_allStartUpPluginsLoaded()), SLOT(sl_registerServices()));

    if(testingMode) {
        foreach(const QString& pluginFile, pluginFiles) {
            if(pluginFile.contains("opencl_support")) {
                Task* loadStartUpPlugins = new LoadAllPluginsTask(this, QStringList(pluginFile));
                AppContext::getTaskScheduler()->registerTopLevelTask(loadStartUpPlugins);
                return;
            }
        }
    }

    QString openclCheckingVersion = settings->getValue(OPENCL_CHECKED_SETTINGS, "").toString();

    Task* loadStartUpPlugins = NULL;
#ifndef Q_OS_MAC
    if(openclCheckingVersion != Version::appVersion().text && !testingMode) {
        loadStartUpPlugins = new LoadAllPluginsTask(this, pluginFiles.toList(), QStringList("opencl_support"));
    }
    else {
        loadStartUpPlugins = new LoadAllPluginsTask(this, pluginFiles.toList(), QStringList());
        settings->setValue(OPENCL_CHECKED_SETTINGS, Version::appVersion().text);
    }
#else
    Q_UNUSED(openclCheckingVersion);
    loadStartUpPlugins = new LoadAllPluginsTask(this, pluginFiles.toList(), QStringList());
#endif

    AppContext::getTaskScheduler()->registerTopLevelTask(loadStartUpPlugins);
}


PluginSupportImpl::~PluginSupportImpl() {
    foreach(PluginRef* ref, plugRefs) {
        delete ref;
    }
}

bool PluginSupportImpl::isAllPluginsLoaded() const {
    return allLoaded;
}

LoadAllPluginsTask::LoadAllPluginsTask(PluginSupportImpl* _ps, const QStringList& _pluginFiles, const QStringList& verifiedPlugins)
: Task(tr("Loading start up plugins"), TaskFlag_NoRun), ps(_ps), pluginFiles(_pluginFiles), verifiedPlugins(verifiedPlugins)
{
    coreLog.trace("List of the plugins to be loaded:");
    foreach(const QString& path, pluginFiles) {
        coreLog.trace(path);
    }
    coreLog.trace("End of the list");

}
void LoadAllPluginsTask::prepare() {
    foreach (const QString& url, pluginFiles) {
        addToOrderingQueue(url);
    }

    QString err;
    orderedPlugins = PluginDescriptorHelper::orderPlugins(orderedPlugins, err);

    if (!err.isEmpty()) {
        setError(err);
        return;
    }

    if(!orderedPluginsWithVerification.isEmpty()) {
        orderedPluginsWithVerification = PluginDescriptorHelper::orderPlugins(orderedPluginsWithVerification, err);

        if (!err.isEmpty()) {
            setError(err);
            return;
        }
        foreach(const PluginDesc& desc, orderedPluginsWithVerification) {
            addSubTask(new VerifyPluginTask(ps, desc));
        }
    }

    foreach(const PluginDesc& desc, orderedPlugins) {
        addSubTask(new AddPluginTask(ps, desc));
    }
}

QList<Task*> LoadAllPluginsTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    VerifyPluginTask* verifyTask = qobject_cast<VerifyPluginTask*>(subTask);
    if(NULL != verifyTask) {
        if(verifyTask->isCorrectPlugin()) {
            res << new AddPluginTask(ps, verifyTask->getPluginDescriptor());
        }
        else {
            MainWindow* mw = AppContext::getMainWindow();
            if(NULL == mw) {
                return res;
            }
            mw->addNotification(tr("Problem occurred loading the OpenCL driver. Please try to update drivers if \
                                   you're going to make calculations on your video card. For details see this page: \
                                   <a href=\"%1\">%1</a>").arg("http://ugene.unipro.ru/using-video-cards.html"), Warning_Not);
        }
    }
    return res;
}

void LoadAllPluginsTask::addToOrderingQueue(const QString& url) {
    QFileInfo descFile(url);
    if (!descFile.exists()) {
        coreLog.trace(  tr("File not found: %1").arg(url) );
        return;
    }

    if (!descFile.isFile()) {
        coreLog.trace(  tr("Invalid file format: %1").arg(url) );
        return;
    }

    QString err;
    PluginDesc desc = PluginDescriptorHelper::readPluginDescriptor(url, err);
    if (!desc.isValid()) {
        assert(!err.isEmpty());
        coreLog.trace(err);
        return;
    }

    // now check plugin compatibility
    bool isUIMode = AppContext::getMainWindow() != NULL;
    bool modeIsOk = false;
    if (isUIMode) {
        modeIsOk = desc.mode.testFlag(PluginMode_UI);
    } else {
        modeIsOk = desc.mode.testFlag(PluginMode_Console);
    }
    if (!modeIsOk) {
        coreLog.trace(QString("Plugin is inactive in the current mode: %1, skipping load").arg(desc.id));
        return;
    }

    // check version
    Version ugeneVersion = Version::appVersion();
    Version qtVersion = Version::qtVersion();
    if (ugeneVersion.debug != desc.pluginVersion.debug) {
        coreLog.trace(QString("Plugin debug/release mode is not matched with UGENE binaries: %1").arg(desc.id));
        return;
    }
    if (qtVersion < desc.qtVersion) {
        coreLog.trace(QString("Plugin was built with higher QT version: %1").arg(desc.id));
        return;
    }
    if (ugeneVersion < desc.ugeneVersion) {
        coreLog.trace(QString("Plugin was built with higher UGENE version: %1, %2 vs %3").arg(desc.id).arg(desc.ugeneVersion.text).arg(ugeneVersion.text));
        return;
    }

    //check platform

    if (desc.platform.arch == PlatformArch_Unknown) {
        coreLog.trace(QString("Plugin platform arch is unknown: %1").arg(desc.id));
        return;
    }
    if (desc.platform.arch == PlatformArch_32 && QT_POINTER_SIZE != 4) {
        coreLog.trace(QString("Plugin was built on 32-bit platform: %1").arg(desc.id));
        return;
    }
    if (desc.platform.arch == PlatformArch_64 && QT_POINTER_SIZE != 8) {
        coreLog.trace(QString("Plugin was built on 64-bit platform: %1").arg(desc.id));
        return;
    }

    if (desc.platform.name == PlatformName_Unknown) {
        coreLog.trace(QString("Plugin platform name is unknown: %1").arg(desc.id));
        return;
    }

#if defined(Q_OS_WIN)
    if (desc.platform.name != PlatformName_Win) {
        coreLog.trace(QString("Plugin platform is not Windows: %1").arg(desc.id));
        return;
    }
#elif defined(Q_OS_MAC)
    if (desc.platform.name != PlatformName_Mac) {
        coreLog.trace(QString("Plugin platform is not Mac: %1").arg(desc.id));
        return;
    }
#else
    if (desc.platform.name != PlatformName_UnixNotMac) {
        coreLog.trace(QString("Plugin platform is not Unix/Linux: %1").arg(desc.id));
        return;
    }
#endif

    foreach(const QString& verifiedPlugin, verifiedPlugins) {
        if(url.contains(verifiedPlugin) && !verifiedPlugin.isEmpty()) {
            orderedPluginsWithVerification.append(desc);
            return;
        }
    }
    orderedPlugins.append(desc);
}





Task::ReportResult LoadAllPluginsTask::report()
{
    ps->allLoaded = true;
    emit ps->si_allStartUpPluginsLoaded();
    return ReportResult_Finished;
}

static QStringList findAllPluginsInDefaultPluginsDir() {
    QDir d = PluginSupportImpl::getDefaultPluginsDir();
    QStringList filter; filter << QString("*.") + PLUGIN_FILE_EXT;
    QStringList fileNames = d.entryList(filter, QDir::Readable | QDir::Files, QDir::NoSort);
    QStringList res;
    foreach(const QString& name, fileNames) {
        GUrl filePath(d.absolutePath() + "/" + name);
        QString path = filePath.getURLString();
        res.append(path);
        coreLog.trace(QString("Found plugin candidate in default dir: %1").arg(path));
    }
    return res;
}


PluginRef::~PluginRef() {
    assert(plugin!=NULL);
    delete plugin;
    plugin = NULL;
}

void PluginSupportImpl::sl_registerServices() {
    ServiceRegistry* sr = AppContext::getServiceRegistry();
    foreach(PluginRef* ref, plugRefs) {
        foreach(Service* s, ref->plugin->getServices()) {
            AppContext::getTaskScheduler()->registerTopLevelTask(sr->registerServiceTask(s));
        }
    }
}

void PluginSupportImpl::registerPlugin(PluginRef* ref) {
    plugRefs.push_back(ref);
    plugins.push_back(ref->plugin);
    updateSavedState(ref);
    emit si_pluginAdded(ref->plugin);
}


QString PluginSupportImpl::getPluginFileURL(Plugin* p) const {
    assert(plugins.size() == plugRefs.size());

    foreach(PluginRef* ref, plugRefs) {
        if (ref->plugin == p) {
            if (ref->library == NULL) {
                return "";
            }
            return ref->library->fileName();

        }
    }
    return QString::null;
}


Task* PluginSupportImpl::addPluginTask(const QString& pathToPlugin) {
    QString err;
    PluginDesc desc = PluginDescriptorHelper::readPluginDescriptor(pathToPlugin, err);
    Task* res = new AddPluginTask(this, desc);
    if (!err.isEmpty()) {
        res->setError(err);
    }
    return res;
}

PluginRef* PluginSupportImpl::findRef(Plugin* p) const {
    foreach(PluginRef* r, plugRefs) {
        if (r->plugin == p) {
            return r;
        }
    }
    return NULL;
}

PluginRef* PluginSupportImpl::findRefById(const QString& pluginId) const {
    foreach(PluginRef* r, plugRefs) {
        if (r->pluginDesc.id == pluginId) {
            return r;
        }
    }
    return NULL;
}


//plugin will not be removed from the plugin list during the next app run
void PluginSupportImpl::setRemoveFlag(Plugin* p, bool v) {
    PluginRef* r = findRef(p);
    assert(r!=NULL);
    if (r->removeFlag == v) {
        return;
    }
    r->removeFlag = v;
    updateSavedState(r);
    emit si_pluginRemoveFlagChanged(p);
}

bool PluginSupportImpl::getRemoveFlag(Plugin* p) const {
    PluginRef* r = findRef(p);
    return r->removeFlag;
}
void PluginSupportImpl::setLicenseAccepted(Plugin *p){
    p->acceptLicense();
    PluginRef* r = findRef(p);
    assert(r!=NULL);
    updateSavedState(r);
}
void PluginSupportImpl::updateSavedState(PluginRef* ref) {
    if (ref->library == NULL) {
        // skip core plugin
        return;
    }
    Settings* settings = AppContext::getSettings();
    QString pluginListSettingsDir = settings->toVersionKey(PLUGINS_LIST_SETTINGS);
    QString skipListSettingsDir = settings->toVersionKey(SKIP_LIST_SETTINGS);
    QString pluginAcceptedLicenseSettingsDir = settings->toVersionKey(PLUGINS_ACCEPTED_LICENSE_LIST);
    QString descUrl = ref->pluginDesc.descriptorUrl.getURLString();
    QString pluginId = ref->pluginDesc.id;
    if (ref->removeFlag) {
        settings->remove(pluginListSettingsDir + pluginId);

        //add to skip-list if auto-loaded
        if (isDefaultPluginsDir(descUrl)) {
            QStringList skipFiles = settings->getValue(skipListSettingsDir, QStringList()).toStringList();
            if (!skipFiles.contains(descUrl)) {
                skipFiles.append(descUrl);
                settings->setValue(skipListSettingsDir, skipFiles);
            }
        }
    } else {
        settings->setValue(pluginListSettingsDir + versionAppendix + pluginId, descUrl);

        //remove from skip-list if present
        if (isDefaultPluginsDir(descUrl)) {
            QStringList skipFiles = settings->getValue(skipListSettingsDir, QStringList()).toStringList();
            if (skipFiles.removeOne(descUrl)) {
                settings->setValue(skipListSettingsDir, skipFiles);
            }
        }
    }

    if (!ref->plugin->isFree()){
        settings->setValue(pluginAcceptedLicenseSettingsDir + versionAppendix + pluginId + "license",ref->plugin->isLicenseAccepted());
    }
}

QDir PluginSupportImpl::getDefaultPluginsDir() {
    return QDir(AppContext::getWorkingDirectoryPath() + "/plugins");
}

bool PluginSupportImpl::isDefaultPluginsDir(const QString& url) {
    QDir urlAbsDir = QFileInfo(url).absoluteDir();
    QDir plugsDir = getDefaultPluginsDir();
    return  urlAbsDir == plugsDir;
}

QSet<QString> PluginSupportImpl::getPluginPaths(){
    Settings* settings = AppContext::getSettings();
    QString pluginListSettingsDir = settings->toVersionKey(PLUGINS_LIST_SETTINGS);

    QStringList pluginsIds = settings->getAllKeys(pluginListSettingsDir);

    QSet<QString> pluginFiles;
    versionAppendix = Version::buildDate;
    if (!Version::appVersion().isDevVersion){
        versionAppendix.clear();
    }else{
        versionAppendix.replace(" ", ".");
        versionAppendix.append("-");
        if (pluginsIds.size() > 150){
            settings->remove(pluginListSettingsDir);
        }
    }
    foreach (const QString& pluginId, pluginsIds) {
        QString file = settings->getValue(pluginListSettingsDir + versionAppendix + pluginId).toString();
        if(!file.isEmpty()) {
            pluginFiles.insert(file);
        }
    }
    //read all plugins from the current folder and from ./plugins folder
    // use SKIP list to learn which plugin should not be loaded
    QStringList skipFiles = settings->getValue(settings->toVersionKey(SKIP_LIST_SETTINGS), QStringList()).toStringList();
    pluginFiles.unite(findAllPluginsInDefaultPluginsDir().toSet());
    pluginFiles.subtract(skipFiles.toSet());

    return pluginFiles;
}


//////////////////////////////////////////////////////////////////////////
/// Tasks

//todo: improve task naming
AddPluginTask::AddPluginTask(PluginSupportImpl* _ps, const PluginDesc& _desc)
: Task(tr("Add plugin task: %1").arg(_desc.id), TaskFlag_NoRun), ps(_ps), desc(_desc)
{
}

Task::ReportResult AddPluginTask::report() {
    PluginRef* ref = ps->findRefById(desc.id);
    if (ref != NULL) {
        stateInfo.setError(  tr("Plugin is already loaded: %1").arg(desc.id) );
        return ReportResult_Finished;
    }

    //check that plugin we depends on is already loaded
    foreach (const DependsInfo& di, desc.dependsList) {
        PluginRef* ref = ps->findRefById(desc.id);
        if (ref == NULL) {
            stateInfo.setError(  tr("Plugin %1 depends on %2 which is not loaded").arg(desc.id).arg(di.id) );
            return ReportResult_Finished;
        }
        if (ref->pluginDesc.pluginVersion < di.version) {
            stateInfo.setError(  tr("Plugin %1 depends on %2 which is available, but the version is too old").arg(desc.id).arg(di.id) );
            return ReportResult_Finished;
        }
    }

    //load library
    QString libUrl = desc.libraryUrl.getURLString();
    QScopedPointer<QLibrary> lib(new QLibrary(libUrl));
    bool loadOk = lib->load();

    if (!loadOk) {
        stateInfo.setError(  tr("Plugin loading error: %1, Error string %2").arg(libUrl).arg(lib->errorString()) );
        return ReportResult_Finished;
    }

    //instantiate plugin
    PLUG_INIT_FUNC init_fn = PLUG_INIT_FUNC((lib->resolve(U2_PLUGIN_INIT_FUNC_NAME)));
    if (!init_fn) {
        stateInfo.setError(  tr("Plugin initialization routine was not found: %1").arg(libUrl) );
        return ReportResult_Finished;
    }

    Plugin* p = init_fn();
    if (p == NULL) {
        stateInfo.setError(  tr("Plugin initialization failed: %1").arg(libUrl) );
        return ReportResult_Finished;
    }
    p->setLicensePath(desc.licenseUrl.getURLString());

    if (!p->isFree()){
        QString versionAppendix = Version::buildDate;
        if (!Version::appVersion().isDevVersion){
            versionAppendix.clear();
        }else{
            versionAppendix.replace(" ", ".");
            versionAppendix.append("-");
        }
        Settings* settings = AppContext::getSettings();
        QString pluginAcceptedLicenseSettingsDir = settings->toVersionKey(PLUGINS_ACCEPTED_LICENSE_LIST);
        if(settings->getValue(pluginAcceptedLicenseSettingsDir + versionAppendix + desc.id + "license",false).toBool()){
            p->acceptLicense();
        }
    }

    ref = new PluginRef(p, lib.take(), desc);
    ps->registerPlugin(ref);

    return ReportResult_Finished;
}

VerifyPluginTask::VerifyPluginTask(PluginSupportImpl* ps, const PluginDesc& desc)
: Task(tr("Verify plugin task: %1").arg(desc.id), TaskFlags(TaskFlag_ReportingIsSupported) | TaskFlag_ReportingIsEnabled), ps(ps), desc(desc), timeOut(100000), proc(NULL), pluginIsCorrect(false)
{
}
void VerifyPluginTask::run() {
    Settings* settings = AppContext::getSettings();

    QString executableDir = AppContext::getWorkingDirectoryPath();
    QString openclCheckerPath = executableDir + "/plugins_checker";
    if(Version::appVersion().debug) {
        openclCheckerPath += 'd';
    }
    #ifdef Q_OS_WIN
        openclCheckerPath += ".exe";
    #endif

    if(!QFileInfo(openclCheckerPath).exists()) {
        coreLog.error(QString("Can not find file: \"%1\"").arg(openclCheckerPath));
        return;
    }
    proc = new QProcess();
    proc->start(openclCheckerPath, QStringList());

    int elapsedTime = 0;
    while(!proc->waitForFinished(1000) && elapsedTime < timeOut) {
        if(isCanceled()) {
            proc->kill();
        }
        elapsedTime += 1000;
    }
    QString errorMessage = proc->readAllStandardError();

    if(0 != proc->exitCode() || !errorMessage.isEmpty()) {
        settings->setValue(settings->toVersionKey(SKIP_LIST_SETTINGS) + desc.id, desc.descriptorUrl.getURLString());
    }
    else {
        pluginIsCorrect = true;
    }
}
}//namespace
