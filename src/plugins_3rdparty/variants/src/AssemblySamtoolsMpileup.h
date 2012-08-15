/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_ASSEMBLY_SAMTOOLS_MPILEUP_H_
#define _U2_ASSEMBLY_SAMTOOLS_MPILEUP_H_

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/LoadDocumentTask.h>

#include <U2Lang/LocalDomain.h>
namespace U2 {
namespace LocalWorkflow{

class CallVariantsTaskSettings{
    public:

    QList<QString> assemblyUrls;
    QString refSeqUrl; // TODO: change to sequence ID

    //mpileup
    bool    illumina13;
    bool    use_orphan;
    bool    disable_baq;
    int     capq_thres;
    int     max_depth;
    bool    ext_baq;
    QByteArray bed;
    QByteArray reg;
    int     min_mq;
    int     min_baseq;
    int     extq;
    int     tandemq;
    bool    no_indel;
    int     max_indel_depth;
    int     openq;
    QByteArray pl_list;
    
    //bcf view
    bool    keepalt;
    bool    fix_pl;
    bool    no_geno;
    bool    acgt_only;
    QByteArray bcf_bed;
    bool    qcall;
    QByteArray samples;
    float   min_smpl_frac;
    bool    call_gt;
    float   indel_frac;
    float   pref;
    QByteArray ptype;
    float   theta;
    QByteArray ccall;
    int     n1;
    int     n_perm;
    float   min_perm_p;

};

class SamtoolsMpileupTask : public Task{
    Q_OBJECT
public:
    SamtoolsMpileupTask ( const CallVariantsTaskSettings& _settings);

    void prepare();
    void run();
    ReportResult report();

    const QString& getBcfOutputFilePath(){return tmpBcfViewOutputFile;}

private:
    CallVariantsTaskSettings    settings;
    QString             tmpMpileupOutputFile;
    QString             tmpBcfViewOutputFile;

};

class CallVariantsTask : public Task {
    Q_OBJECT
public:
    CallVariantsTask(const CallVariantsTaskSettings& _settings, DbiDataStorage* _store);

    void prepare();
    void run();
    QList<Task*> onSubTaskFinished(Task* subTask);
    ReportResult report();

    const QList<QVariantMap>& getResults(){return results;}
    void clearResults(){results.clear();}
private:
    CallVariantsTaskSettings    settings;
    LoadDocumentTask*       loadTask;
    SamtoolsMpileupTask*    mpileupTask;
    DbiDataStorage*         storage;
    QList<QVariantMap>      results;

};

}
}//namespace

#endif //_U2_ASSEMBLY_SAMTOOLS_MPILEUP_H_
