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

#include <U2Core/Timer.h>
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

    if (subTask == mpileupTask) {
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

    } else if(subTask == loadTask){
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

QString CallVariantsTask::tmpFilePath(const QString &baseName, const QString &ext, U2OpStatus &os) {
    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(CALL_VARIANTS_DIR);
    return GUrlUtils::prepareTmpFileLocation(tmpDirPath, baseName, ext, os);
}

/************************************************************************/
/* SamtoolsMpileupTask */
/************************************************************************/
SamtoolsMpileupTask::SamtoolsMpileupTask(const CallVariantsTaskSettings &_settings)
:Task(tr("Samtool mpileup for %1 ").arg(_settings.refSeqUrl), TaskFlags(TaskFlag_None)),settings(_settings)
{

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

    //prepare tmp files
    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(CALL_VARIANTS_DIR);
    filteredFile = GUrlUtils::prepareTmpFileLocation(tmpDirPath, "filtered", "vcf", stateInfo);
    CHECK_OP(stateInfo, );
}

void SamtoolsMpileupTask::run() {
    ProcessRun samtools = ExternalToolSupportUtils::prepareProcess("SAMtools", settings.getMpiliupArgs(), "", QStringList(), stateInfo);
    CHECK_OP(stateInfo, );
    QScopedPointer<QProcess> sp(samtools.process);
    QScopedPointer<ExternalToolRunTaskHelper> sh(new ExternalToolRunTaskHelper(samtools.process, new ExternalToolLogParser(), stateInfo));

    ProcessRun bcftools = ExternalToolSupportUtils::prepareProcess("BCFtools", settings.getBcfViewArgs(), "", QStringList(), stateInfo);
    CHECK_OP(stateInfo, );
    QScopedPointer<QProcess> bp(bcftools.process);
    QScopedPointer<ExternalToolRunTaskHelper> bh(new ExternalToolRunTaskHelper(bcftools.process, new ExternalToolLogParser(), stateInfo));

    ProcessRun vcfutils = ExternalToolSupportUtils::prepareProcess("vcfutils", settings.getVarFilterArgs(), "", QStringList(), stateInfo);
    CHECK_OP(stateInfo, );
    QScopedPointer<QProcess> vp(vcfutils.process);
    QScopedPointer<ExternalToolRunTaskHelper> vh(new ExternalToolRunTaskHelper(vcfutils.process, new ExternalToolLogParser(), stateInfo));

    samtools.process->setStandardOutputProcess(bcftools.process);
    bcftools.process->setStandardOutputProcess(vcfutils.process);
    vcfutils.process->setStandardOutputFile(filteredFile);

    start(samtools, "SAMtools");
    CHECK_OP(stateInfo, );
    start(bcftools, "BCFtools");
    CHECK_OP(stateInfo, );
    start(vcfutils, "vcfutils");
    CHECK_OP(stateInfo, );

    while(!vcfutils.process->waitForFinished(1000)){
        if (isCanceled()) {
            samtools.process->kill();
            bcftools.process->kill();
            vcfutils.process->kill();
            return;
        }
    }

    checkExitCode(vcfutils.process, "vcfutils");
    checkExitCode(bcftools.process, "BCFtools");
    checkExitCode(samtools.process, "SAMtools");
}

void SamtoolsMpileupTask::start(const ProcessRun &pRun, const QString &toolName) {
    pRun.process->start(pRun.program, pRun.arguments);
    bool started = pRun.process->waitForStarted();
    CHECK_EXT(started, setError(tr("Can not run %1 tool").arg(toolName)), );
}

void SamtoolsMpileupTask::checkExitCode(QProcess *process, const QString &toolName) {
    int exitCode = process->exitCode();
    if (exitCode != EXIT_SUCCESS && !hasError()) {
        setError(tr("%1 tool exited with code %2").arg(toolName).arg(exitCode));
    } else {
        algoLog.details(tr("Tool %1 finished successfully").arg(toolName));
    }
}

/************************************************************************/
/* CallVariantsTaskSettings */
/************************************************************************/
QStringList CallVariantsTaskSettings::getMpiliupArgs() const {
    QStringList result;
    result << "mpileup"
        << "-uf"
        << refSeqUrl
        << "-C"
        << QString::number(capq_thres)
        << "-d"
        << QString::number(max_depth)
        << "-q"
        << QString::number(min_mq)
        << "-Q"
        << QString::number(min_baseq)
        << "-e"
        << QString::number(extq)
        << "-h"
        << QString::number(tandemq)
        << "-L"
        << QString::number(max_indel_depth)
        << "-o"
        << QString::number(openq);

    if (illumina13) {
        result << "-6";
    }
    if (use_orphan) {
        result << "-A";
    }
    if (disable_baq) {
        result << "-B";
    }
    if (ext_baq) {
        result << "-E";
    }
    if (!bed.isEmpty()) {
        result << "-l";
        result << bed;
    }
    if (!reg.isEmpty()) {
        result << "-r";
        result << reg;
    }
    if (no_indel) {
        result << "-I";
    }
    if (!pl_list.isEmpty()) {
        result << "-P";
        result << pl_list;
    }
    result << assemblyUrls;
    return result;
}

QStringList CallVariantsTaskSettings::getBcfViewArgs() const {
    QStringList result;
    result << "view"
        << "-vc"
        << "-d"
        << QString::number(min_smpl_frac)
        << "-i"
        << QString::number(indel_frac)
        << "-p"
        << QString::number(pref)
        << "-t"
        << QString::number(theta)
        << "-1"
        << QString::number(n1)
        << "-U"
        << QString::number(n_perm)
        << "-X"
        << QString::number(min_perm_p);

    if (keepalt) {
        result << "-A";
    }
    if (fix_pl) {
        result << "-F";
    }
    if (no_geno) {
        result << "-G";
    }
    if (!bcf_bed.isEmpty()) {
        result<< "-l";
        result << bcf_bed;
    }
    if (acgt_only) {
        result << "-N";
    }
    if (qcall) {
        result << "-Q";
    }
    if (!samples.isEmpty()) {
        result << "-s";
        result << samples;
    }
    if (call_gt) {
        result << "-g";
    }
    if (!ptype.isEmpty()) {
        result<< "-P";
        result << ptype;
    }
    if (!ccall.isEmpty()) {
        result<< "-T";
        result << ccall;
    }
    result << "-";
    return result;
}

QStringList CallVariantsTaskSettings::getVarFilterArgs() const {
    QStringList result;
    result << "varFilter"
        << "-Q"
        << QString::number(minQual)
        << "-d"
        << QString::number(minDep)
        << "-D"
        << QString::number(maxDep)
        << "-a"
        << QString::number(minAlt)
        << "-w"
        << QString::number(gapSize)
        << "-W"
        << QString::number(window)
        << "-1"
        << QString::number(pvalue1)
        << "-2"
        << QString::number(pvalue2)
        << "-3"
        << QString::number(pvalue3)
        << "-4"
        << QString::number(pvalue4)
        << "-e"
        << QString::number(pvalueHwe);
    if (printFiltered) {
        result << "-p";
    }
    return result;
}

}
} //namespace
