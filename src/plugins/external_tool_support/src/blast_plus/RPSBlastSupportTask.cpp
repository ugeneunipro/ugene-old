#include "RPSBlastSupportTask.h"
#include "BlastPlusSupport.h"


namespace U2 {

static Logger log(ULOG_CAT_BLASTPLUS_RUN_TASK);

ExternalToolRunTask* RPSBlastSupportTask::createBlastPlusTask() {
    QStringList arguments;
    arguments << "-db" << settings.databaseNameAndPath;
    arguments << "-evalue" << QString::number(settings.expectValue);
    arguments << "-query" << url;
    arguments << "-outfmt" << "5"; //Set output file format to xml
    arguments << "-out" << url+".xml"; //settings.outputRepFile;

    log.trace("RPSBlast arguments: "+arguments.join(" "));
    logParser=new ExternalToolLogParser;
    return new ExternalToolRunTask(RPSBLAST_TOOL_NAME, arguments, logParser);
}

LocalCDSearch::LocalCDSearch(const CDSearchSettings& settings) {
    BlastTaskSettings stngs;
    stngs.databaseNameAndPath = settings.localDbFolder + "/" + settings.dbName;
    stngs.querySequence = settings.query;
    stngs.expectValue = settings.ev;
    stngs.alphabet = settings.alp;
    stngs.needCreateAnnotations = false;
    task = new RPSBlastSupportTask(stngs);
}

QList<SharedAnnotationData> LocalCDSearch::getCDSResults() const {
    return task->getResultedAnnotations();
}

} //namespace
