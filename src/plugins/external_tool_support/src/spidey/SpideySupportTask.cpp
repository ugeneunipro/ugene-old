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

#include "SpideySupportTask.h"
#include "SpideySupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/Counter.h>
#include <U2Core/ExternalToolRegistry.h>


namespace U2 {


//////////////////////////////////////////////////////////////////////////
////SpideySupportTask

SpideyAlignmentTask::SpideyAlignmentTask(const SplicedAlignmentTaskConfig& settings) :
        SplicedAlignmentTask("SpideySupportTask", TaskFlags_NR_FOSCOE, settings)
{
    GCOUNTER( cvar, tvar, "SpideySupportTask" );
    setMaxParallelSubtasks(1);
    logParser = NULL;
    spideyTask = NULL;
    prepareDataForSpideyTask = NULL;
    resultAlignment = NULL;
}


void SpideyAlignmentTask::prepare(){

    //Add new subdir for temporary files
    
    tmpDirUrl = ExternalToolSupportUtils::createTmpDir(SPIDEY_TMP_DIR, stateInfo);
    CHECK_OP(stateInfo, );


    prepareDataForSpideyTask =
            new PrepareInputForSpideyTask(config.getGenomicSequence(), config.getCDnaSequence(),
                                                             tmpDirUrl);
    addSubTask(prepareDataForSpideyTask);

}

QList<Task*> SpideyAlignmentTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    
    propagateSubtaskError();
    
    if(hasError() || isCanceled()) {
        return res;
    }
    
    if (subTask == prepareDataForSpideyTask) {

        assert(!prepareDataForSpideyTask->getResultPath().isEmpty());

        tmpOutputUrl = prepareDataForSpideyTask->getResultPath();
        const QStringList& arguments = prepareDataForSpideyTask->getArgumentsList();

        logParser = new SpideyLogParser();
        spideyTask = new ExternalToolRunTask(SPIDEY_TOOL_NAME, arguments, logParser);
        spideyTask->setSubtaskProgressWeight(95);
        res.append(spideyTask);

    } else if (subTask == spideyTask){

        if(!QFile::exists(tmpOutputUrl)){
            if(AppContext::getExternalToolRegistry()->getByName(SPIDEY_TOOL_NAME)->isValid()){
                stateInfo.setError(tr("Output file not found"));
            }else{
                stateInfo.setError(tr("Output file not found. May be %1 tool path '%2' not valid?")
                                   .arg(AppContext::getExternalToolRegistry()->getByName(SPIDEY_TOOL_NAME)->getName())
                                   .arg(AppContext::getExternalToolRegistry()->getByName(SPIDEY_TOOL_NAME)->getPath()));
            }
            return res;
        }
        
        // parse result

        QFile resultFile(tmpOutputUrl);
        static const int BUFSIZE = 1024;
        QList<Annotation*> anns;

        if (!resultFile.open(QFile::ReadOnly)) {
            setError(tr("Failed to open result file %1").arg(tmpOutputUrl));
            return res;
        }
        QByteArray buf;
        buf.reserve(BUFSIZE);

        while (!resultFile.atEnd()) {
            qint64 lineLength = resultFile.readLine(buf.data(), BUFSIZE);
            if (lineLength != -1) {
                if (buf.startsWith("Exon:")) {
                    int pos = buf.indexOf("(gen)");
                    if (pos == -1) {
                        continue;
                    }
                    QByteArray loc = buf.mid(5, pos).trimmed();
                    QList<QByteArray> loci = loc.split('-');
                    if (loci.size() < 2) {
                        continue;
                    }
                    int start = QString(loci.at(0)).toInt();
                    int finish = QString(loci.at(1)).toInt();

                    SharedAnnotationData data(new AnnotationData);
                    U2Location location;
                    location->regions.append(U2Region(start,finish));
                    // TODO; check strand
                    data->location = location;
                    data->name = "exon";


                    anns.append(new Annotation(data));

                }
            }
        }

        resultAlignment = new AnnotationTableObject("SplicedAlignment");
        resultAlignment->addAnnotations(anns);

    }

    return res;

}


Task::ReportResult SpideyAlignmentTask::report() {
    U2OpStatus2Log os;
    ExternalToolSupportUtils::removeTmpDir(tmpDirUrl,os);

    return ReportResult_Finished;
}

//////////////////////////////////////////
////SpideyLogParser

SpideyLogParser::SpideyLogParser() {
}

int SpideyLogParser::getProgress() {
    return 0;
}


//////////////////////////////////////////
////PrepareInput

PrepareInputForSpideyTask::PrepareInputForSpideyTask(const U2SequenceObject* dna, const U2SequenceObject* mRna,
                          const QString& outputDirPath)
    :Task("PrepareInputForSpideyTask", TaskFlags_FOSCOE),
      dnaObj(dna), mRnaObj(mRna),  outputDir(outputDirPath)
{


}

void PrepareInputForSpideyTask::prepare() {
    


}

#define SPIDEY_SUMMARY "spidey_output"

void PrepareInputForSpideyTask::run()
{
    if ( hasError() || isCanceled() ) {
        return;
    }
    // writing DNA

    QString dnaName = dnaObj->getSequenceName();
    QString dnaPath = outputDir + "/" + dnaName + ".fa";
    StreamShortReadWriter dnaWriter;
    dnaWriter.init(dnaPath);
    if (!dnaWriter.writeNextSequence(dnaObj->getWholeSequence())) {
        setError(tr("Failed to write DNA sequence  %1").arg(dnaName));
        return;
    }
    dnaWriter.close();
    argumentList.append("-i");
    argumentList.append(dnaPath);

    //writing mRNA

    QString mRnaName = mRnaObj->getSequenceName();
    if (mRnaName == dnaName) {
        setError(tr("DNA and mRNA sequences have equal names %1").arg(dnaName));
        return;
    }
    QString mRnaPath = outputDir + "/" + mRnaName + ".fa";

    StreamShortReadWriter mRnaWriter;
    mRnaWriter.init(dnaPath);
    if (!mRnaWriter.writeNextSequence(mRnaObj->getWholeSequence())) {
       setError(tr("Failed to write DNA sequence  %1").arg(dnaName));
       return;
    }
    mRnaWriter.close();
    argumentList.append("-m");
    argumentList.append(mRnaPath);


    // adding additional arguments

    resultPath = QString("%1/%2").arg(outputDir).arg(SPIDEY_SUMMARY);
    argumentList.append("-p");
    argumentList.append("1");
    argumentList.append("-o");
    argumentList.append(resultPath);



}


}//namespace
