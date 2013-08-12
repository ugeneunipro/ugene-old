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

#ifndef _U2_CONVERT_FILES_FORMAT_WORKER_H_
#define _U2_CONVERT_FILES_FORMAT_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {
namespace LocalWorkflow {

class ConvertFilesFormatPrompter;
typedef PrompterBase<ConvertFilesFormatPrompter> ConvertFilesFormatBase;

class ConvertFilesFormatPrompter : public ConvertFilesFormatBase {
    Q_OBJECT
public:
    ConvertFilesFormatPrompter(Actor* p = 0) : ConvertFilesFormatBase(p) {}
protected:
    QString composeRichDoc();
}; //ConvertFilesFormatPrompter

class ConvertFilesFormatWorker: public BaseWorker {
    Q_OBJECT
public:
    ConvertFilesFormatWorker(Actor* a) : BaseWorker(a), inputUrlPort(NULL), outputUrlPort(NULL), 
                                                        selectedFormat(QString()), excludedFormats(NULL) {};
    void init();
    void getWorkingDir( QString &workingDir );
    Task * tick();
    void cleanup();
private:
    IntegralBus *inputUrlPort;
    IntegralBus *outputUrlPort;
    QString selectedFormat;
    QStringList excludedFormats;
public slots:
    void sl_taskFinished( Task *task );
}; //ConvertFilesFormatWorker

class ConvertFilesFormatWorkerFactory : public DomainFactory {
    static const QString ACTOR_ID;
public:
    static void init();
    ConvertFilesFormatWorkerFactory() : DomainFactory(ACTOR_ID) {}
    Worker* createWorker(Actor* a) { return new ConvertFilesFormatWorker(a); }
}; //ConvertFilesFormatWorkerFactory

class ConvertFilesFormatTask : public Task {
private:
    GUrl sourceURL;
    QString selectedFormat;
    QString workingDir;
    QList<SharedDbiDataHandler> result;
    bool isTaskLoadDocument;
public:
    ConvertFilesFormatTask( const GUrl &url, const QString &_selectedFormat, const QString &dir ) : 
                                             Task( "Conversion file format task", TaskFlags_NR_FOSCOE ), 
                                             sourceURL( url ), selectedFormat( _selectedFormat ),
                                             workingDir( dir ), isTaskLoadDocument(true) {}
    void prepare();
    QList<Task*> onSubTaskFinished( Task *subTask );
}; //ConvertFilesFormatTask

} //LocalWorkflow
} //U2

#endif
