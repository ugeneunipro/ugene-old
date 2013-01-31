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

#ifndef _U2_APPCONTEXT_H_
#define _U2_APPCONTEXT_H_

#include "global.h"

namespace U2 {

class PluginSupport;
class ServiceRegistry;
class ProjectLoader;
class MainWindow;
class ProjectView;
class Project;
class ProjectService;
class PluginViewer;
class Settings;
class AppSettings;
class AppSettingsGUI;
class DocumentFormatRegistry;
class IOAdapterRegistry;
class DNATranslationRegistry;
class DNAAlphabetRegistry;
class GObjectViewFactoryRegistry;
class ResourceTracker;
class TaskScheduler;
class DocumentFormatConfigurators;
class AnnotationSettingsRegistry;
class TestFramework;
class DBXRefRegistry;
class SubstMatrixRegistry;
class SmithWatermanTaskFactoryRegistry;
class SWMulAlignResultNamesTagsRegistry;
class MolecularSurfaceFactoryRegistry;
class SWResultFilterRegistry;
class MSAColorSchemeRegistry;
class SecStructPredictAlgRegistry;
class CudaGpuRegistry;
class OpenCLGpuRegistry;
class RecentlyDownloadedCache;
class ProtocolInfoRegistry;
class RemoteMachineMonitor;
class PhyTreeGeneratorRegistry;
class CMDLineRegistry;
class MSAConsensusAlgorithmRegistry;
class AssemblyConsensusAlgorithmRegistry;
class MSADistanceAlgorithmRegistry;
class PWMConversionAlgorithmRegistry;
class VirtualFileSystemRegistry;
class DnaAssemblyAlgRegistry;
class MSAAlignAlgRegistry;
class AppGlobalObject;
class DataBaseRegistry;
class ExternalToolRegistry;
class RepeatFinderTaskFactoryRegistry;
class U2DbiRegistry;
class QDActorPrototypeRegistry;
class StructuralAlignmentAlgorithmRegistry;
class AutoAnnotationsSupport;
class CDSearchFactoryRegistry;
class GUITestBase;
class SplicedAlignmentTaskRegistry;
class OPWidgetFactoryRegistry;
class WorkflowScriptRegistry;
class AppFileStorage;
class U2DataPathRegistry;

class U2CORE_EXPORT AppContext  : public QObject {
    Q_OBJECT
public:
    virtual ~AppContext(){}

    static PluginSupport*   getPluginSupport()  {return getInstance()->_getPluginSupport();}

    static ServiceRegistry* getServiceRegistry()  {return getInstance()->_getServiceRegistry();}

    static  ProjectLoader*  getProjectLoader() {return getInstance()->_getProjectLoader();}

    static  Project*        getProject() {return getInstance()->_getProject();}

    static  ProjectService* getProjectService() {return getInstance()->_getProjectService();}

    static  PluginViewer*   getPluginViewer() {return getInstance()->_getPluginViewer();}

    /** admin level, readonly settings*/
    static  const Settings* getGlobalSettings() {return getInstance()->_getGlobalSettings();}

    /** per-user settings*/
    static  Settings*       getSettings() {return getInstance()->_getSettings();}

    /** structured and preloaded settings */
    static AppSettings * getAppSettings() { return getInstance()->_getAppSettings();}

    /** GUI for structured and preloaded settings */
    static AppSettingsGUI* getAppSettingsGUI() {return getInstance()->_getAppSettingsGUI();}


    static  MainWindow*     getMainWindow() {return getInstance()->_getMainWindow();}

    static  ProjectView*    getProjectView() {return getInstance()->_getProjectView();}

    static  DocumentFormatRegistry* getDocumentFormatRegistry() {return getInstance()->_getDocumentFormatRegistry();}

    static  IOAdapterRegistry* getIOAdapterRegistry() {return getInstance()->_getIOAdapterRegistry();}

    static  DNATranslationRegistry* getDNATranslationRegistry() {return getInstance()->_getDNATranslationRegistry();}

    static  DNAAlphabetRegistry* getDNAAlphabetRegistry() {return getInstance()->_getDNAAlphabetRegistry();}

    static  GObjectViewFactoryRegistry* getObjectViewFactoryRegistry() {return getInstance()->_getObjectViewFactoryRegistry();}

    static  TaskScheduler* getTaskScheduler() {return getInstance()->_getTaskScheduler();}

    static  ResourceTracker* getResourceTracker() {return getInstance()->_getResourceTracker();}

    static DocumentFormatConfigurators* getDocumentFormatConfigurators() {return getInstance()->_getDocumentFormatConfigurators();}

    static AnnotationSettingsRegistry* getAnnotationsSettingsRegistry() {return getInstance()->_getAnnotationsSettingsRegistry();}

    static TestFramework* getTestFramework() {return getInstance()->_getTestFramework();}

    static DBXRefRegistry* getDBXRefRegistry() {return getInstance()->_getDBXRefRegistry();}

    static SubstMatrixRegistry* getSubstMatrixRegistry() {return getInstance()->_getSubstMatrixRegistry();}

    static SmithWatermanTaskFactoryRegistry* getSmithWatermanTaskFactoryRegistry() {return getInstance()->_getSmithWatermanTaskFactoryRegistry();}

    static SWMulAlignResultNamesTagsRegistry * getSWMulAlignResultNamesTagsRegistry() {return getInstance()->_getSWMulAlignResultNamesTagsRegistry();}

    static MolecularSurfaceFactoryRegistry* getMolecularSurfaceFactoryRegistry() {return getInstance()->_getMolecularSurfaceFactoryRegistry();}

    static SWResultFilterRegistry* getSWResultFilterRegistry() {return getInstance()->_getSWResultFilterRegistry();}

    static MSAColorSchemeRegistry* getMSAColorSchemeRegistry() { return getInstance()->_getMSAColorSchemeRegistry();}

    static SecStructPredictAlgRegistry* getSecStructPredictAlgRegistry() { return getInstance()->_getSecStructPredictAlgRegistry(); }

    static CudaGpuRegistry * getCudaGpuRegistry() { return getInstance()->_getCudaGpuRegistry(); }

    static OpenCLGpuRegistry * getOpenCLGpuRegistry() { return getInstance()->_getOpenCLGpuRegistry(); }

    static RecentlyDownloadedCache* getRecentlyDownloadedCache() { return getInstance()->_getRecentlyDownloadedCache(); }   

    static ProtocolInfoRegistry * getProtocolInfoRegistry() { return getInstance()->_getProtocolInfoRegistry(); }

    static RemoteMachineMonitor * getRemoteMachineMonitor() { return getInstance()->_getRemoteMachineMonitor(); }

    static PhyTreeGeneratorRegistry *getPhyTreeGeneratorRegistry() {return getInstance()->_getPhyTreeGeneratorRegistry();}

    static CMDLineRegistry* getCMDLineRegistry() {return getInstance()->_getCMDLineRegistry();}

    static MSAConsensusAlgorithmRegistry* getMSAConsensusAlgorithmRegistry() {return getInstance()->_getMSAConsensusAlgorithmRegistry(); }

    static MSADistanceAlgorithmRegistry* getMSADistanceAlgorithmRegistry() {return getInstance()->_getMSADistanceAlgorithmRegistry(); }
    
    static AssemblyConsensusAlgorithmRegistry* getAssemblyConsensusAlgorithmRegistry() {return getInstance()->_getAssemblyConsensusAlgorithmRegistry(); }

    static PWMConversionAlgorithmRegistry* getPWMConversionAlgorithmRegistry() {return getInstance()->_getPWMConversionAlgorithmRegistry(); }

    static VirtualFileSystemRegistry * getVirtualFileSystemRegistry() { return getInstance()->_getVirtualFileSystemRegistry(); }

    static DnaAssemblyAlgRegistry* getDnaAssemblyAlgRegistry() { return getInstance()->_getDnaAssemblyAlgRegistry(); }
    
    static MSAAlignAlgRegistry* getMSAAlignAlgRegistry() { return getInstance()->_getMSAAlignAlgRegistry(); }

    static RepeatFinderTaskFactoryRegistry* getRepeatFinderTaskFactoryRegistry() {return getInstance()->_getRepeatFinderTaskFactoryRegistry();}
    
    static AutoAnnotationsSupport* getAutoAnnotationsSupport() { return getInstance()->_getAutoAnnotationsSupport(); }

    static CDSearchFactoryRegistry* getCDSFactoryRegistry() { return getInstance()->_getCDSFactoryRegistry(); }

    /** Registers application global resource */
    static void registerGlobalObject(AppGlobalObject* obj) {return getInstance()->_registerGlobalObject(obj);}

    /** Unregisters application global resource */
    static void unregisterGlobalObject(const QString& id) {return getInstance()->_unregisterGlobalObject(id);}

    /** Finds application global resource */
    static AppGlobalObject* getGlobalObjectById(const QString& id) {return getInstance()->_getGlobalObjectById(id);}

    static DataBaseRegistry *getDataBaseRegistry() {return getInstance()->_getDataBaseRegistry();}

    static ExternalToolRegistry * getExternalToolRegistry() {return getInstance()->_getExternalToolRegistry();}

    static QDActorPrototypeRegistry* getQDActorProtoRegistry() { return getInstance()->_getQDActorFactoryRegistry(); }

    static StructuralAlignmentAlgorithmRegistry* getStructuralAlignmentAlgorithmRegistry() { return getInstance()->_getStructuralAlignmentAlgorithmRegistry(); }

    static SplicedAlignmentTaskRegistry* getSplicedAlignmentTaskRegistry() { return getInstance()->_getSplicedAlignmentTaskRegistry(); }

    static OPWidgetFactoryRegistry* getOPWidgetFactoryRegistry() { return getInstance()->_getOPWidgetFactoryRegistry(); }

    static U2DbiRegistry *getDbiRegistry() { return getInstance()->_getDbiRegistry(); }

    static U2DataPathRegistry* getDataPathRegistry() {return getInstance()->_getDataPathRegistry();}
    
    static bool isGUIMode() {return getInstance()->_isGUIMode(); }

    static WorkflowScriptRegistry* getWorkflowScriptRegistry() { return getInstance()->_getWorkflowScriptRegistry(); }

    static QString getActiveWindowName() {return getInstance()->_getActiveWindowName();}
    static void setActiveWindowName(const QString &name) {getInstance()->_setActiveWindowName(name);}
    static GUITestBase *getGUITestBase() {return getInstance()->_getGUITestBase();}

    static AppFileStorage *getAppFileStorage() {return getInstance()->_getAppFileStorage();}
    
protected:
    static AppContext* getInstance() {assert(instance); return instance;}

    virtual PluginSupport*   _getPluginSupport() const = 0;
    virtual ServiceRegistry* _getServiceRegistry() const = 0;
    virtual ProjectLoader*  _getProjectLoader() const = 0;
    virtual Project*        _getProject() const = 0;
    virtual ProjectService* _getProjectService() const = 0;
    virtual PluginViewer*   _getPluginViewer() const = 0;
    virtual Settings*       _getSettings() const = 0;
    virtual Settings*       _getGlobalSettings() const = 0;
    virtual MainWindow*     _getMainWindow() const = 0;
    virtual ProjectView*    _getProjectView() const = 0;
    virtual DocumentFormatRegistry*     _getDocumentFormatRegistry() const = 0;
    virtual IOAdapterRegistry*          _getIOAdapterRegistry() const = 0;
    virtual DNATranslationRegistry*     _getDNATranslationRegistry() const = 0;
    virtual DNAAlphabetRegistry*        _getDNAAlphabetRegistry() const = 0;
    virtual GObjectViewFactoryRegistry* _getObjectViewFactoryRegistry() const = 0;
    virtual TaskScheduler*              _getTaskScheduler() const = 0;
    virtual ResourceTracker*            _getResourceTracker() const = 0;
    virtual DocumentFormatConfigurators* _getDocumentFormatConfigurators() const = 0;
    virtual AnnotationSettingsRegistry* _getAnnotationsSettingsRegistry() const = 0;
    virtual AppSettings *               _getAppSettings() const = 0;
    virtual AppSettingsGUI*             _getAppSettingsGUI() const = 0;
    virtual TestFramework*              _getTestFramework() const = 0;
    virtual DBXRefRegistry*             _getDBXRefRegistry() const = 0;
    virtual SubstMatrixRegistry*        _getSubstMatrixRegistry() const = 0;
    virtual SmithWatermanTaskFactoryRegistry* _getSmithWatermanTaskFactoryRegistry() const = 0;
    virtual MolecularSurfaceFactoryRegistry* _getMolecularSurfaceFactoryRegistry()const = 0;
    virtual SWMulAlignResultNamesTagsRegistry * _getSWMulAlignResultNamesTagsRegistry() const = 0;
    virtual SWResultFilterRegistry*     _getSWResultFilterRegistry() const = 0;
    virtual MSAColorSchemeRegistry*     _getMSAColorSchemeRegistry() const = 0;
    virtual SecStructPredictAlgRegistry* _getSecStructPredictAlgRegistry() const = 0;
    virtual CudaGpuRegistry *           _getCudaGpuRegistry() const = 0;
    virtual OpenCLGpuRegistry *          _getOpenCLGpuRegistry() const = 0;
    virtual RecentlyDownloadedCache*    _getRecentlyDownloadedCache() const = 0;
    virtual ProtocolInfoRegistry *      _getProtocolInfoRegistry() const = 0;
    virtual RemoteMachineMonitor *      _getRemoteMachineMonitor() const = 0;
    virtual PhyTreeGeneratorRegistry *  _getPhyTreeGeneratorRegistry() const = 0;
    virtual CMDLineRegistry*            _getCMDLineRegistry() const  = 0;
    virtual MSAConsensusAlgorithmRegistry* _getMSAConsensusAlgorithmRegistry() const = 0;
    virtual MSADistanceAlgorithmRegistry* _getMSADistanceAlgorithmRegistry() const = 0;
    virtual AssemblyConsensusAlgorithmRegistry* _getAssemblyConsensusAlgorithmRegistry() const = 0;
    virtual PWMConversionAlgorithmRegistry* _getPWMConversionAlgorithmRegistry() const = 0;
    virtual VirtualFileSystemRegistry * _getVirtualFileSystemRegistry() const = 0;
    virtual DnaAssemblyAlgRegistry*     _getDnaAssemblyAlgRegistry() const = 0;
    virtual MSAAlignAlgRegistry*        _getMSAAlignAlgRegistry() const = 0;
    virtual DataBaseRegistry *          _getDataBaseRegistry() const = 0;
    virtual ExternalToolRegistry *          _getExternalToolRegistry() const = 0;
    virtual RepeatFinderTaskFactoryRegistry*  _getRepeatFinderTaskFactoryRegistry() const = 0;
    virtual QDActorPrototypeRegistry*             _getQDActorFactoryRegistry() const = 0;
    virtual StructuralAlignmentAlgorithmRegistry* _getStructuralAlignmentAlgorithmRegistry() const = 0;
    virtual AutoAnnotationsSupport*     _getAutoAnnotationsSupport() const = 0;
    virtual U2DbiRegistry *             _getDbiRegistry() const = 0;
    virtual CDSearchFactoryRegistry* _getCDSFactoryRegistry() const = 0;
    virtual GUITestBase* _getGUITestBase() const = 0;
    virtual SplicedAlignmentTaskRegistry* _getSplicedAlignmentTaskRegistry() const = 0;
    virtual OPWidgetFactoryRegistry* _getOPWidgetFactoryRegistry() const = 0;
    virtual WorkflowScriptRegistry* _getWorkflowScriptRegistry() const = 0;
    virtual AppFileStorage *_getAppFileStorage() const = 0;
    virtual U2DataPathRegistry* _getDataPathRegistry() const = 0; 

    virtual void _registerGlobalObject(AppGlobalObject* go) = 0;
    virtual void _unregisterGlobalObject(const QString& id) = 0;
    virtual AppGlobalObject* _getGlobalObjectById(const QString& id) const = 0;
    virtual bool _isGUIMode() const = 0;
    virtual QString _getActiveWindowName() const = 0;
    virtual void _setActiveWindowName(const QString &name) = 0;

    static AppContext* instance;
};

}//namespace

#endif
