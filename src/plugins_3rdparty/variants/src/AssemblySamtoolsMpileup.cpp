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

#include "AssemblySamtoolsMpileup.h"

#include <cstring>
#include <stdio.h>
extern "C" {
#include <bam2bcf.h>
#include <bam_plcmd.c>
#include <call1.c>
}

#include <U2Core/GUrlUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/VariantTrackObject.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/DocumentUtils.h>

#include <U2Lang/DbiDataHandler.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#define CALL_VARIANTS_DIR "variants"

namespace U2 {
namespace LocalWorkflow{

CallVariantsTask::CallVariantsTask( const CallVariantsTaskSettings& _settings, DbiDataStorage* _store )
:Task(tr("Call variants for %1").arg(_settings.refSeqUrl), TaskFlag_None)
,settings(_settings)
,loadTask(NULL)
,mpileupTask(NULL)
,storage(_store)
{
    setMaxParallelSubtasks(1);
}


void CallVariantsTask::prepare(){
    if (settings.assemblyUrls.size() < 1){
        stateInfo.setError(tr("No assembly files"));
        return;
    }

    if (storage == NULL){
        stateInfo.setError(tr("No dbi storage"));
        return;
    }
    if (settings.refSeqUrl.isEmpty()){
        stateInfo.setError(tr("No sequence URL"));
        return;
    }

    mpileupTask = new SamtoolsMpileupTask(settings);
    addSubTask(mpileupTask );
}


void CallVariantsTask::run(){
        
}
QList<Task*> CallVariantsTask::onSubTaskFinished( Task* subTask ){
    QList<Task*>res;

    if(subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    if(hasError() || isCanceled()) {
        return res;
    }

    if(subTask == mpileupTask){
        GUrl url(mpileupTask->getBcfOutputFilePath());
        if( url.isEmpty() ) {
            return res;
        }
        IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( IOAdapterUtils::url2io( url ) );
        if ( iof == NULL ) {
            return res;
        }
        QList<FormatDetectionResult> dfs = DocumentUtils::detectFormat(url);
        if( dfs.isEmpty() ) {
            return res;
        }
        DocumentFormat * df = dfs.first().format;
        QVariantMap cfg;
        cfg.insert(DocumentFormat::DBI_REF_HINT, qVariantFromValue(storage->getDbiRef()));
        loadTask =  new LoadDocumentTask( df->getFormatId(), url, iof, cfg );
        res.append(loadTask);

    }else if(subTask == loadTask){
        QScopedPointer<Document> doc (loadTask->takeDocument(false));
        SAFE_POINT(doc!=NULL, tr("No document loaded"), res);
        doc->setDocumentOwnsDbiResources(false);
        foreach(GObject* go, doc->findGObjectByType(GObjectTypes::VARIANT_TRACK)) {
            VariantTrackObject *varObj = dynamic_cast<VariantTrackObject*>(go);
            CHECK_EXT(NULL != varObj, taskLog.error(tr("Incorrect variant track object in %1").arg(doc->getURLString())), res);

            QVariantMap m;
            SharedDbiDataHandler handler = storage->getDataHandler(varObj->getEntityRef());
            m.insert(BaseSlots::VARIATION_TRACK_SLOT().getId(), qVariantFromValue<SharedDbiDataHandler>(handler));
            results.append(m);
       }
   }

    return res;
}

Task::ReportResult CallVariantsTask::report(){
    if (!mpileupTask || mpileupTask->hasError()){
        return ReportResult_Finished;
    }
    
    GUrlUtils::removeFile(mpileupTask->getBcfOutputFilePath(), stateInfo);

    return ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////
//SamtoolsMpileupTask

static char ** createArgv(QByteArray& firstElement, QByteArray& secElement, QList<QString>& urls){
    char ** res;
    //int size = urls.size() + 2;
    int size = 3;
    res = new char *[size];
    res[0] = new char [firstElement.size()+1];
    strncpy(res[0], firstElement.data(), firstElement.size());
    res[0][firstElement.size()] = '\0';

    SAFE_POINT(urls.size()>=1, "no assembly urls", res);

    res[1] = new char [urls.first().size()+1];
    QByteArray arr = urls.first().toLocal8Bit();
    strncpy(res[1], arr.data(), urls.first().size());
    res[1][urls.first().size()] = '\0';

    res[2] = new char [secElement.size()+1];
    strncpy(res[2], secElement.data(), secElement.size());
    res[2][secElement.size()] = '\0';
    //     for (int i = 2; i < size; i++){
    //         res[i] = new char [urls[i-2].size()+1];
    //         strncpy(res[i], urls[i-2].data(), urls[i-2].size());
    //         res[i][urls[i-2].size()] = '\0';        
    //     }

    return res;    
}

static void deleteArgv(char **arr, int size){
    for (int i = 0; i < size; i++){
        delete [] arr[i];
    }

    delete [] arr;
}

SamtoolsMpileupTask::SamtoolsMpileupTask( const CallVariantsTaskSettings& _settings)
:Task(tr("Samtool mpileup for %1 ").arg(_settings.refSeqUrl), TaskFlag_None)
,settings(_settings){

}

void SamtoolsMpileupTask::prepare(){
    if (settings.refSeqUrl.isEmpty()){
        setError(tr("No reference sequence URL to do pileup"));
        return ;
    }

    if (settings.assemblyUrls.isEmpty()){
        setError(tr("No assembly URL to do pileup"));
        return ;
    }

    foreach(const QString& aUrl, settings.assemblyUrls){
        if (aUrl.isEmpty()){
            setError(tr("There is an assembly with an empty path"));
            return ;
        }
    }
    
    //prepare tmp file
    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(CALL_VARIANTS_DIR);
    tmpMpileupOutputFile = GUrlUtils::prepareTmpFileLocation(tmpDirPath, "tmp", "bcf", stateInfo);
    tmpBcfViewOutputFile = GUrlUtils::prepareTmpFileLocation(tmpDirPath, "tmp", "vcf", stateInfo);
    CHECK_OP(stateInfo, );

}

void SamtoolsMpileupTask::run(){
    if(stateInfo.isCanceled() || stateInfo.hasError()){
        return;
    }

    int argc1 = 3;
    QByteArray mpileupArr = QString("mpileup").toLatin1();
    QByteArray seqArr = settings.refSeqUrl.toLocal8Bit();
    char** argv1 = createArgv(mpileupArr, seqArr, settings.assemblyUrls);

    UGENE_mpileup_settings mpileupSettings;

    mpileupSettings.b_illumina13 = settings.illumina13 == true ? 1 : 0;
    mpileupSettings.b_use_orphan = settings.use_orphan == true ? 1 : 0;
    mpileupSettings.b_disable_baq = settings.disable_baq == true ? 1 : 0;
    mpileupSettings.capq_thres = settings.capq_thres;
    mpileupSettings.max_depth = settings.max_depth;
    mpileupSettings.b_ext_baq = settings.ext_baq == true ? 1 : 0;
    mpileupSettings.bed = settings.bed.data();
    mpileupSettings.reg = settings.reg.data();
    mpileupSettings.min_mq = settings.min_mq;
    mpileupSettings.min_baseq = settings.min_baseq;
    mpileupSettings.extq = settings.extq;
    mpileupSettings.tandemq = settings.tandemq;
    mpileupSettings.b_no_indel = settings.no_indel == true ? 1 : 0;
    mpileupSettings.max_indel_depth = settings.max_indel_depth;
    mpileupSettings.openq = settings.openq;
    mpileupSettings.pl_list = settings.pl_list.data();

    int ret = bam_mpileup(argc1-1, argv1, &mpileupSettings, tmpMpileupOutputFile.toLocal8Bit().constData());

    deleteArgv(argv1, argc1);
    if (ret == -1){
        setError("mpileup finished with an error");
        return;
    }
    

    QByteArray ba = tmpMpileupOutputFile.toLocal8Bit();
    char* ar11 = "bcfview";
    char* ar12 = ba.data();
    char* bcfViewArgv[] = {ar11, ar12, (char*)0};

    UGENE_bcf_view_settings bcfSettings;
    bcfSettings.b_keepalt = settings.keepalt == true ? 1 : 0;
    bcfSettings.b_fix_pl = settings.fix_pl == true ? 1 : 0;
    bcfSettings.b_no_geno = settings.no_geno == true ? 1 : 0;
    bcfSettings.b_acgt_only = settings.acgt_only == true ? 1 : 0;
    bcfSettings.bcf_bed = settings.bcf_bed.data();
    bcfSettings.b_qcall = settings.qcall == true ? 1 : 0;
    bcfSettings.samples = settings.samples.data();
    bcfSettings.min_smpl_frac = settings.min_smpl_frac;
    bcfSettings.b_call_gt = settings.call_gt == true ? 1 : 0;
    bcfSettings.indel_frac = settings.indel_frac;
    bcfSettings.pref = settings.pref;
    bcfSettings.ptype = settings.ptype.data();
    bcfSettings.theta = settings.theta;
    bcfSettings.ccall = settings.ccall.data();
    bcfSettings.n1 = settings.n1;
    bcfSettings.n_perm = settings.n_perm;
    bcfSettings.min_perm_p = settings.min_perm_p;

   
    ret = bcfview(2, bcfViewArgv, &bcfSettings, tmpBcfViewOutputFile.toLocal8Bit().constData());
    if (ret == -1){
        setError("bcf view finished with an error");
        return;
    }


}

Task::ReportResult SamtoolsMpileupTask::report(){
    //remove tmp file
    GUrlUtils::removeFile(tmpMpileupOutputFile, stateInfo);

    return ReportResult_Finished;
}


}
} //namespace
