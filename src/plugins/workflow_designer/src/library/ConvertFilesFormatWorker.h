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
#include <U2Core/GUrl.h>

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
                                                        selectedFormat(QString()), selectedFormatExtensions(NULL), 
                                                        excludedFormats(NULL) {}
    void getSelectedFormatExtensions( );
    void getExcludedFormats( const QStringList &excludedFormatsIds );
    void init();
    void getWorkingDir( QString &workingDir );
    Task * tick();
    void cleanup();
private:
    IntegralBus *inputUrlPort;
    IntegralBus *outputUrlPort;
    QString selectedFormat;
    QStringList selectedFormatExtensions;
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

class BamSamConversionTask : public Task {
    Q_OBJECT
private:
    GUrl sourceURL;
    GUrl destinationURL;
    bool samToBam;
public:
    BamSamConversionTask( const GUrl &_sourceURL, const GUrl &_destinationURL, bool _samToBam) : 
                          Task("BAM/SAM conversion task", TaskFlag_None), 
                          sourceURL(_sourceURL), destinationURL(_destinationURL), samToBam(_samToBam){}
    void run();
    QString getDestinationURL();
}; //BamSamConversionTask

} //LocalWorkflow
} //U2

#endif
