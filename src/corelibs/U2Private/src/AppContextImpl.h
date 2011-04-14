/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_APPCONTEXT_IMPL_
#define _U2_APPCONTEXT_IMPL_

#include "private.h"
#include <U2Core/AppContext.h>

namespace U2 {

class U2PRIVATE_EXPORT AppContextImpl : public AppContext {
    Q_OBJECT
public:
    AppContextImpl()  {
        ps = NULL;
        sr = NULL;
        pl = NULL;
        prj = NULL;
        prs = NULL;
        mw = NULL;
        pv = NULL;
        plv = NULL;
        ss = NULL;
        gs = NULL;
        dfr = NULL;
        io = NULL;
        dtr = NULL;
        dal = NULL;
        ovfr = NULL;
        ts = NULL;
        rt = NULL;
        dfc = NULL;
        asr = NULL;
        as = NULL;
        tf = NULL;
        dbxr = NULL;
        asg = NULL;
        smr = NULL;
        swar = NULL;
        msfr = NULL;
        swrfr = NULL;
        mcsr = NULL;
        secStructPredictRegistry = NULL;
        cgr = NULL;
        rdc = NULL;
        protocolInfoRegistry = NULL;
        remoteMachineMonitor = NULL;
        treeGeneratorRegistry = NULL;
        cmdLineRegistry = NULL;
        instance = this;
        msaConsensusAlgoRegistry = NULL;
        msaDistanceAlgoRegistry = NULL;
        pwmConversionAlgoRegistry = NULL;
        dnaAssemblyAlgRegistry = NULL;
        msaAlignAlgRegistry = NULL;
        virtualFileSystemRegistry = NULL;
        dataBaseRegistry = NULL;
        externalToolRegistry = NULL;
        aaSupport = NULL;
        dbiRegistry = NULL;
        rfr = NULL;
        qdafr = NULL;
        cdsfr = NULL;
        saar = NULL;
        guiMode = false;
        activeWindow = "";
        tb = NULL;
    }

    ~AppContextImpl();

    void setPluginSupport(PluginSupport* _ps) {assert(ps == NULL || _ps == NULL); ps = _ps;}

    void setServiceRegistry(ServiceRegistry* _sr) {assert(sr == NULL || _sr == NULL); sr = _sr;}

    void setProjectLoader(ProjectLoader* _pl) {assert(pl == NULL || _pl == NULL); pl = _pl;}

    void setProject(Project* _prj) {assert(prj == NULL || _prj == NULL); prj = _prj;}

    void setProjectService(ProjectService* _prs) {assert(prs == NULL || _prs == NULL); prs = _prs;}

    void setMainWindow(MainWindow* _mw) {assert(mw == NULL || _mw == NULL); mw = _mw;}

    void setProjectView(ProjectView* _pv) {assert(pv == NULL || _pv == NULL); pv = _pv;}

    void setPluginViewer(PluginViewer* _plv) {assert(plv == NULL || _plv == NULL); plv = _plv;}

    void setSettings(Settings* _ss) {assert(ss == NULL || _ss == NULL); ss= _ss;}

    void setGlobalSettings(Settings* _gs) {assert(gs == NULL || _gs == NULL); gs= _gs;}

    void setAppSettings( AppSettings* _as) { assert( as|| _as); as= _as; }

    void setAppSettingsGUI( AppSettingsGUI* _asg) { assert( asg == NULL || _asg == NULL ); asg= _asg; }

    void setDocumentFormatRegistry(DocumentFormatRegistry* _dfr) {assert(dfr == NULL || _dfr == NULL); dfr = _dfr;}

    void setIOAdapterRegistry(IOAdapterRegistry* _io) {assert(io == NULL || _io == NULL); io = _io;}

    void setDNATranslationRegistry(DNATranslationRegistry* _dtr) {assert(dtr == NULL || _dtr == NULL);dtr = _dtr;}
    
    void setDNAAlphabetRegistry(DNAAlphabetRegistry* _dal) {assert(dal == NULL || _dal == NULL);dal = _dal;}

    void setObjectViewFactoryRegistry(GObjectViewFactoryRegistry* _ovfr) {assert(ovfr == NULL || _ovfr == NULL); ovfr = _ovfr;}

    void setTaskScheduler(TaskScheduler* _ts) {assert(ts == NULL || _ts == NULL); ts = _ts;}

    void setResourceTracker(ResourceTracker* _rt) {assert(rt == NULL || _rt == NULL); rt = _rt;}
    
    void setDocumentFormatConfigurators(DocumentFormatConfigurators* _dfc)  {assert(dfc == NULL || _dfc == NULL); dfc = _dfc;}

    void setAnnotationSettingsRegistry(AnnotationSettingsRegistry* _asr)  {assert(asr == NULL || _asr == NULL); asr = _asr;}

    void setTestFramework( TestFramework* _tf) { assert( tf || _tf ); tf = _tf; }

    void setDBXRefRegistry( DBXRefRegistry* _dbxr) { assert( dbxr == NULL || _dbxr == NULL ); dbxr = _dbxr; }

    void setSubstMatrixRegistry(SubstMatrixRegistry* _smr) { assert( smr == NULL || _smr == NULL ); smr = _smr; }

    void setSmithWatermanTaskFactoryRegistry (SmithWatermanTaskFactoryRegistry* _swar) { assert( swar == NULL || _swar == NULL ); swar = _swar; }
    
    void setMolecularSurfaceFactoryRegistry (MolecularSurfaceFactoryRegistry* _msfr) { assert( msfr == NULL || _msfr == NULL ); msfr = _msfr; }

    void setSWResultFilterRegistry (SWResultFilterRegistry* _swrfr) { assert( swrfr == NULL || _swrfr == NULL ); swrfr = _swrfr; }

    void setMSAColorSchemeRegistry(MSAColorSchemeRegistry* _mcsr) {assert( mcsr == NULL || _mcsr == NULL ); mcsr = _mcsr;}
    
    void setSecStructPedictAlgRegistry(SecStructPredictAlgRegistry* _sspar) {assert( secStructPredictRegistry == NULL || _sspar == NULL ); secStructPredictRegistry = _sspar;}

    void setCudaGpuRegistry( CudaGpuRegistry * _cgr ) { assert( cgr == NULL || _cgr == NULL ); cgr = _cgr; }

    void setOpenCLGpuRegistry( OpenCLGpuRegistry* _oclgr ) { assert( oclgr == NULL || _oclgr == NULL ); oclgr = _oclgr; }

    void setRecentlyDownloadedCache( RecentlyDownloadedCache* _rdc) { assert( rdc == NULL || _rdc == NULL ); rdc = _rdc;}

    void setProtocolInfoRegistry( ProtocolInfoRegistry * pr ) { assert( NULL == protocolInfoRegistry || NULL == pr );
        protocolInfoRegistry = pr; }

    void setRemoteMachineMonitor( RemoteMachineMonitor * rm ) { assert( NULL == remoteMachineMonitor || NULL == rm ); 
        remoteMachineMonitor = rm; }

    void setPhyTreeGeneratorRegistry(PhyTreeGeneratorRegistry* genRegistry) {
        assert(NULL == treeGeneratorRegistry || NULL == genRegistry);
        treeGeneratorRegistry = genRegistry;
    }

    void setMSAConsensusAlgorithmRegistry(MSAConsensusAlgorithmRegistry* reg) {
        assert(reg == NULL || msaConsensusAlgoRegistry == NULL);
        msaConsensusAlgoRegistry = reg;
    }

    void setMSADistanceAlgorithmRegistry(MSADistanceAlgorithmRegistry* reg) {
        assert(reg == NULL || msaDistanceAlgoRegistry == NULL);
        msaDistanceAlgoRegistry = reg;
    }

    void setPWMConversionAlgorithmRegistry(PWMConversionAlgorithmRegistry* reg) {
        assert(reg == NULL || pwmConversionAlgoRegistry == NULL);
        pwmConversionAlgoRegistry = reg;
    }

    void setCMDLineRegistry(CMDLineRegistry* r) { assert(cmdLineRegistry == NULL || r == NULL); cmdLineRegistry = r; }
    
    void setVirtualFileSystemRegistry( VirtualFileSystemRegistry * r ) { 
        assert( virtualFileSystemRegistry == NULL || r == NULL );
        virtualFileSystemRegistry = r;
    }

    void setDnaAssemblyAlgRegistry( DnaAssemblyAlgRegistry * r ) { 
        assert( dnaAssemblyAlgRegistry == NULL || r == NULL );
        dnaAssemblyAlgRegistry = r;
    }

    void setMSAAlignAlgRegistry( MSAAlignAlgRegistry * r ) { 
        assert( msaAlignAlgRegistry == NULL || r == NULL );
        msaAlignAlgRegistry = r;
    }

    void setDataBaseRegistry( DataBaseRegistry *dbr) {
        assert (dataBaseRegistry == NULL || dbr == NULL );
        dataBaseRegistry = dbr;
    }
    void setExternalToolRegistry( ExternalToolRegistry * _etr) { assert( externalToolRegistry == NULL || _etr == NULL ); externalToolRegistry = _etr; }
    void setRepeatFinderTaskFactoryRegistry (RepeatFinderTaskFactoryRegistry* _rfr) {
        assert( rfr == NULL || _rfr == NULL ); rfr = _rfr;
    }

    void setQDActorFactoryRegistry(QDActorPrototypeRegistry* _queryfactoryRegistry) {
        assert( qdafr == NULL || _queryfactoryRegistry == NULL );
        qdafr = _queryfactoryRegistry;
    }
    
    void setAutoAnnotationsSupport(AutoAnnotationsSupport* _aaSupport) {
        assert( aaSupport == NULL || _aaSupport == NULL );
        aaSupport = _aaSupport;
    }

    void setDbiRegistry(U2DbiRegistry *_dbiRegistry) {
        assert((NULL == dbiRegistry) || (NULL == _dbiRegistry));
        dbiRegistry = _dbiRegistry;
    }

    void setCDSearchFactoryRegistry(CDSearchFactoryRegistry* _cdsfr) {
        assert((NULL == cdsfr) || (NULL == _cdsfr));
        cdsfr= _cdsfr;
    }

    void setStructuralAlignmentAlgorithmRegistry(StructuralAlignmentAlgorithmRegistry *_saar) { assert(saar == NULL || _saar == NULL); saar = _saar; }

    void setGUIMode(bool v) {
        guiMode = v;
    }

    void _setActiveWindowName(const QString& name) {
        activeWindow = name;
    }

    void setGUITestBase(GUITestBase *_tb) {assert(tb == NULL || _tb == NULL); tb = _tb;}

    static AppContextImpl* getApplicationContext();

protected:
    virtual PluginSupport*  _getPluginSupport() const {return ps;}
    virtual ServiceRegistry*  _getServiceRegistry() const {return sr;}
    virtual ProjectLoader*  _getProjectLoader() const {return pl;}
    virtual Project*        _getProject() const {return prj;}
    virtual ProjectService* _getProjectService() const {return prs;}
    virtual MainWindow*     _getMainWindow() const {return mw;}
    virtual ProjectView*    _getProjectView() const {return pv;}
    virtual PluginViewer*   _getPluginViewer() const {return plv;}
    virtual Settings*       _getSettings() const {return ss;}
    virtual Settings*       _getGlobalSettings() const {return gs;}
    virtual AppSettings*    _getAppSettings() const{return as;};
    virtual AppSettingsGUI* _getAppSettingsGUI() const{return asg;};

    virtual DocumentFormatRegistry*         _getDocumentFormatRegistry() const {return dfr;}
    virtual IOAdapterRegistry*              _getIOAdapterRegistry() const  {return io;}
    virtual DNATranslationRegistry*         _getDNATranslationRegistry() const  {return dtr;}
    virtual DNAAlphabetRegistry*            _getDNAAlphabetRegistry() const {return dal;}
    virtual GObjectViewFactoryRegistry*     _getObjectViewFactoryRegistry() const  {return ovfr;}
    virtual TaskScheduler*                  _getTaskScheduler() const  {return ts;}
    virtual ResourceTracker*                _getResourceTracker() const {return rt;}
    virtual AnnotationSettingsRegistry*     _getAnnotationsSettingsRegistry() const {return asr;}
    virtual TestFramework*                  _getTestFramework() const {return tf;}
    virtual DBXRefRegistry*                 _getDBXRefRegistry() const {return dbxr;}
    virtual SubstMatrixRegistry*            _getSubstMatrixRegistry() const {return smr;}
    virtual SmithWatermanTaskFactoryRegistry*   _getSmithWatermanTaskFactoryRegistry() const {return swar;}
    virtual PhyTreeGeneratorRegistry*         _getPhyTreeGeneratorRegistry() const {return treeGeneratorRegistry;}
    
    virtual MolecularSurfaceFactoryRegistry*   _getMolecularSurfaceFactoryRegistry() const {return msfr;}
    virtual SWResultFilterRegistry*     _getSWResultFilterRegistry() const {return swrfr;}
    virtual MSAColorSchemeRegistry*     _getMSAColorSchemeRegistry() const {return mcsr;}
    virtual SecStructPredictAlgRegistry* _getSecStructPredictAlgRegistry() const {return secStructPredictRegistry;}
    virtual CudaGpuRegistry *            _getCudaGpuRegistry() const { return cgr; }
    virtual OpenCLGpuRegistry *          _getOpenCLGpuRegistry() const { return oclgr; }
    virtual RecentlyDownloadedCache*     _getRecentlyDownloadedCache() const {return rdc;}
    virtual DocumentFormatConfigurators*    _getDocumentFormatConfigurators() const {return dfc;}
    virtual ProtocolInfoRegistry *          _getProtocolInfoRegistry() const { return protocolInfoRegistry; }
    virtual RemoteMachineMonitor *          _getRemoteMachineMonitor() const { return remoteMachineMonitor; }
    virtual CMDLineRegistry*                _getCMDLineRegistry() const {return cmdLineRegistry;}
    virtual MSAConsensusAlgorithmRegistry*  _getMSAConsensusAlgorithmRegistry() const {return msaConsensusAlgoRegistry;}
    virtual MSADistanceAlgorithmRegistry*  _getMSADistanceAlgorithmRegistry() const {return msaDistanceAlgoRegistry;}
    virtual PWMConversionAlgorithmRegistry* _getPWMConversionAlgorithmRegistry() const {return pwmConversionAlgoRegistry;}
    virtual VirtualFileSystemRegistry *     _getVirtualFileSystemRegistry() const { return virtualFileSystemRegistry; }
    virtual DnaAssemblyAlgRegistry*         _getDnaAssemblyAlgRegistry() const {return dnaAssemblyAlgRegistry; }
    virtual MSAAlignAlgRegistry*         _getMSAAlignAlgRegistry() const {return msaAlignAlgRegistry; }
    virtual DataBaseRegistry *              _getDataBaseRegistry() const {return dataBaseRegistry;}
    virtual ExternalToolRegistry *          _getExternalToolRegistry() const {return externalToolRegistry;}
    virtual RepeatFinderTaskFactoryRegistry*   _getRepeatFinderTaskFactoryRegistry() const {return rfr;}
    virtual QDActorPrototypeRegistry*            _getQDActorFactoryRegistry() const { return qdafr; }
    virtual StructuralAlignmentAlgorithmRegistry* _getStructuralAlignmentAlgorithmRegistry() const { return saar; }
    virtual AutoAnnotationsSupport*         _getAutoAnnotationsSupport() const { return aaSupport; }
    virtual CDSearchFactoryRegistry*        _getCDSFactoryRegistry() const { return cdsfr; }
    virtual U2DbiRegistry *                 _getDbiRegistry() const { return dbiRegistry; }
    virtual GUITestBase*                    _getGUITestBase() const {return tb;}

    virtual void _registerGlobalObject(AppGlobalObject* go);
    virtual void _unregisterGlobalObject(const QString& id);
    virtual AppGlobalObject* _getGlobalObjectById(const QString& id) const;
    virtual bool _isGUIMode() const {return guiMode;}
    virtual QString _getActiveWindowName() const {return activeWindow;}

private:
    PluginSupport* ps;
    ServiceRegistry* sr;
    ProjectLoader* pl;
    Project*    prj;
    ProjectService* prs;
    MainWindow* mw;
    ProjectView* pv;
    PluginViewer* plv;
    Settings* ss;
    Settings* gs;
    DocumentFormatRegistry* dfr;
    IOAdapterRegistry* io;
    DNATranslationRegistry* dtr;
    DNAAlphabetRegistry* dal;
    GObjectViewFactoryRegistry* ovfr;
    TaskScheduler* ts;
    ResourceTracker* rt;
    DocumentFormatConfigurators* dfc;
    AnnotationSettingsRegistry* asr;
    AppSettings * as;
    TestFramework* tf;
    DBXRefRegistry* dbxr;
    SubstMatrixRegistry* smr;
    SmithWatermanTaskFactoryRegistry* swar;
    MolecularSurfaceFactoryRegistry* msfr;
    SWResultFilterRegistry*  swrfr;
    AppSettingsGUI* asg;
    MSAColorSchemeRegistry* mcsr;
    SecStructPredictAlgRegistry* secStructPredictRegistry;
    CudaGpuRegistry * cgr;   
    OpenCLGpuRegistry * oclgr;
    RecentlyDownloadedCache* rdc;
    ProtocolInfoRegistry * protocolInfoRegistry;
    RemoteMachineMonitor * remoteMachineMonitor;
    PhyTreeGeneratorRegistry *treeGeneratorRegistry;
    CMDLineRegistry* cmdLineRegistry;
    MSAConsensusAlgorithmRegistry* msaConsensusAlgoRegistry;
    MSADistanceAlgorithmRegistry* msaDistanceAlgoRegistry;
    PWMConversionAlgorithmRegistry* pwmConversionAlgoRegistry;
    VirtualFileSystemRegistry * virtualFileSystemRegistry;
    DnaAssemblyAlgRegistry* dnaAssemblyAlgRegistry;
    MSAAlignAlgRegistry* msaAlignAlgRegistry;
    DataBaseRegistry* dataBaseRegistry;
    ExternalToolRegistry * externalToolRegistry;
    RepeatFinderTaskFactoryRegistry* rfr;
    QDActorPrototypeRegistry* qdafr;
    CDSearchFactoryRegistry* cdsfr;
    StructuralAlignmentAlgorithmRegistry* saar;
    AutoAnnotationsSupport* aaSupport;
    U2DbiRegistry *dbiRegistry;
    GUITestBase *tb;
    bool guiMode;
    QString activeWindow;

    QList<AppGlobalObject*> appGlobalObjects;
};

}//namespace

#endif 

