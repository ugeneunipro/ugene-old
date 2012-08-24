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

#ifndef _READ_ASSEMBLY_WORKER_
#define _READ_ASSEMBLY_WORKER_

#include "GenericReadWorker.h"

namespace U2 {

class Document;
class DocumentFormat;
class DocumentProviderTask;

namespace LocalWorkflow {

class ReadAssemblyWorker : public GenericDocReader {
    Q_OBJECT
public:
    ReadAssemblyWorker(Actor *p);
    virtual void init() ;

protected slots:
    virtual void sl_taskFinished();

protected:
    virtual Task *createReadTask(const QString &url);
}; // ReadAssemblyWorker

class ReadAssemblyWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;

    ReadAssemblyWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker *createWorker(Actor *a);

}; // ReadAssemblyWorkerFactory

class ConvertToIndexedBamTask : public Task {
public:
    ConvertToIndexedBamTask(const DocumentFormatId &formatId, const GUrl &url, WorkflowContext *ctx);

    virtual void run();
    GUrl getResultUrl() const;

private:
    DocumentFormatId formatId;
    GUrl url;
    GUrl result;
    WorkflowContext *ctx;
};

class ReadAssemblyTask : public Task {
    Q_OBJECT
public:
    ReadAssemblyTask(const QString &url, WorkflowContext *ctx);
    virtual void prepare();
    virtual void run();
    virtual QList<Task*> onSubTaskFinished(Task *subTask);

    QList<QVariantMap> results;

private:
    QString url;
    WorkflowContext *ctx;
    DocumentFormat *format;
    Document *doc;

    ConvertToIndexedBamTask *convertTask;
    DocumentProviderTask *importTask;
};

} // LocalWorkflow
} // U2

#endif // _READ_ASSEMBLY_WORKER_
