#include "DumpHelpTask.h"

#include <cstdio>

#include <QtCore/QDir>

#include <U2Core/AppContext.h>
#include <U2Core/Version.h>
#include <U2Core/Log.h>

#include <U2Lang/Schema.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/WorkflowIOTasks.h>

#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineHelpProvider.h>
#include <U2Core/CMDLineCoreOptions.h>

namespace U2 {

// TODO i18n help messages ?
// FIXME windows console encoding issue (see LogDriver.cpp)

static void dumpProgramNameAndUsage() {
    Version v = Version::ugeneVersion();
    fprintf( stdout, "\nConsole version of UGENE %s\n" , v.text.toAscii().constData());
    fprintf( stdout, "Usage: ugene [[--task=]taskName] [--task_parameter=value] [-task_parameter value] "
        "[--option[=value]] [-option [value]]\n\n");
}

static void dumpSectionName( const QString & name ) {
    fprintf(stdout, "   --%-20s", name.toLocal8Bit().constData() );
}

static void dumpSectionContent( const QString & content ) {
    fprintf(stdout, "\t%s", content.toLocal8Bit().constData());
}

static void dumpSectionIndent() {
    fprintf(stdout, "%28s", " " );
}

static void dumpTaskName(const QString & taskName) {
    fprintf(stdout, "     %-20s\n", taskName.toLocal8Bit().constData());
}

void DumpHelpTask::dumpHelp() {
    dumpProgramNameAndUsage();

    fprintf(stdout, "\nAvailable tasks (--task option):\n");
    QStringList dataDirs = QDir::searchPaths(PATH_PREFIX_DATA);
    foreach(const QString & url, dataDirs ) {
        QString dirUrl = url + "/cmdline/";
        QDir dir(dirUrl);
        if(dir.exists()) {
            QStringList entries = dir.entryList(QDir::Files | QDir::Readable);
            foreach( const QString & file, entries ) {
                foreach(const QString & ext, WorkflowUtils::WD_FILE_EXTENSIONS) {
                    if( file.endsWith(ext) ) {
                        dumpTaskName(file.mid(0, file.size() - ext.size() - 1)); // 1 comes from "."
                        break;
                    }
                }
            }
        }
    }
    fprintf(stdout, "Use ugene --help=<task-name> to get full help\n");

    fprintf( stdout, "\nOptions: \n" );
    QString prevSectionName;
    QList<CMDLineHelpProvider* > helpProviders = AppContext::getCMDLineRegistry()->listCMDLineHelpProviders();
    foreach (CMDLineHelpProvider* hProvider, helpProviders) {
        const QString& sectionName = hProvider->getHelpSectionName();
        if ( sectionName != prevSectionName) {
            dumpSectionName( sectionName );
            prevSectionName = sectionName;
        } else {
            dumpSectionIndent();
        }
        dumpSectionContent( hProvider->getHelpSectionContent() );
        fprintf(stdout, "\n");
    }
    fprintf( stdout, "\n" );
}

void DumpHelpTask::prepare() {
    QString paramName = AppContext::getCMDLineRegistry()->getParameterValue( CMDLineCoreOptions::HELP );
    if( paramName.isEmpty() ) {
        dumpHelp();
        return;
    }

    int ind = 0;
    QList<CMDLineHelpProvider* > helpProviders = AppContext::getCMDLineRegistry()->listCMDLineHelpProviders();
    int sz = helpProviders.size();
    for( ind = 0; ind < sz; ++ind ) {
        if( helpProviders.at(ind)->getHelpSectionName() == paramName ) {
            break;
        }
    }
    if(ind == sz) {
        // try to find help of workflow designer schema with such name
        QString pathToSchema = WorkflowUtils::findPathToSchemaFile(paramName);
        if( pathToSchema.isEmpty() ) {
            coreLog.error(tr("Can't find help for '%1'").arg(paramName));
            return;
        }

        // will be deleted in sl_loadSchemaStateChanged
        Schema * schema = new Schema();
        Metadata * meta = new Metadata();

        schema->setDeepCopyFlag(true);
        addSubTask(new LoadWorkflowTask( schema, meta, pathToSchema ));
        return;
    }

    dumpProgramNameAndUsage();

    // dump help of selected section in registered help pages
    dumpSectionName( helpProviders.at(ind)->getHelpSectionName() );
    dumpSectionContent( helpProviders.at(ind)->getHelpSectionContent() );
    for( int i = ind + 1; i < sz; ++i ) {
        if( helpProviders.at(i)->getHelpSectionName() != paramName ) {
            break;
        }
        fprintf(stdout, "\n");
        dumpSectionIndent();
        dumpSectionContent( helpProviders.at(i)->getHelpSectionContent() );
    }
    fprintf( stdout, "\n" );
}

static void dumpSchemaMetadata(Metadata * meta) {
    assert(meta != NULL);
    fprintf(stdout, "\n%s\n", meta->comment.toLocal8Bit().constData());
}

static void dumpSchemaCmdlineParameters( Schema * schema ) {
    assert(schema != NULL);
    fprintf(stdout, "\nParameters:\n");
    foreach( Actor * actor, schema->getProcesses() ) {
        assert(actor != NULL);
        QMap<QString, QString>::const_iterator it = actor->getParamAliases().constBegin();
        while( it != actor->getParamAliases().constEnd() ) {
            QString alias = it.value();
            dumpSectionName(alias);
            Attribute * attr = actor->getParameter(it.key());
            assert(attr != NULL);
            if(actor->getAliasHelp().contains(alias)) {
                dumpSectionContent( QString("%1 [%2]").arg(actor->getAliasHelp().value(alias)).arg(attr->getAttributeType()->getDisplayName()));
            } else {
                dumpSectionContent(DumpHelpTask::tr("No help available for this parameter"));
            }
            fprintf(stdout, "\n" );
            ++it;
        }
    }
    fprintf(stdout, "\n" );
}

QList<Task*> DumpHelpTask::onSubTaskFinished(Task* subTask) {
    LoadWorkflowTask * loadTask = qobject_cast<LoadWorkflowTask*>(subTask);
    assert(loadTask != NULL);

    Schema * schema = loadTask->getSchema();
    Metadata * meta = loadTask->getMetadata();

    dumpSchemaMetadata(meta);
    dumpSchemaCmdlineParameters(schema);

    delete schema;
    delete meta;

    return QList<Task*>();
}

void DumpHelpTask::dumpParameters() {
    QList<StringPair> params = AppContext::getCMDLineRegistry()->getParameters();
    QList<StringPair>::const_iterator it = params.constBegin();
    while( it != params.constEnd() ) {
        fprintf( stdout, "key: \"%s\" and value: \"%s\"\n", it->first.toLocal8Bit().constData(), it->second.toLocal8Bit().constData() );
        ++it;
    }
}

DumpHelpTask::DumpHelpTask(): Task(tr("Dump help task"), TaskFlags_NR_FOSCOE) {
}

} //namespace
