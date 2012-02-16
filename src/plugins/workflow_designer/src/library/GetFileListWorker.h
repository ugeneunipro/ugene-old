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

#ifndef _GET_FILE_LIST_WORKER_
#define _GET_FILE_LIST_WORKER_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {
namespace LocalWorkflow {

class GetFileListPrompter : public PrompterBase<GetFileListPrompter> {
    Q_OBJECT
public:
    GetFileListPrompter(Actor *p = NULL) : PrompterBase<GetFileListPrompter>(p) {}

protected:
    QString composeRichDoc();

}; // GetFileListPrompter

class GetFileListWorker : public BaseWorker {
    Q_OBJECT
public:
    GetFileListWorker(Actor *p);

    virtual void init();
    virtual bool isReady();
    virtual Task *tick();
    virtual bool isDone();
    virtual void cleanup() {}

private slots:
    void sl_taskFinished();

private:
    IntegralBus *outChannel;
    bool done;
    QList<Message> cache;
    DataTypePtr mtype;

    QList<QString> dirUrls;
    bool absolute;
    bool recursive;
    QString fileName;
}; // GetFileListWorker

class GetFileListWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;

    GetFileListWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker *createWorker(Actor *a);

}; // GetFileListWorkerFactory

class ScanDirectoryTask : public Task {
    Q_OBJECT
public:
    ScanDirectoryTask(const QString &dirPath, const QString &nameFilter, bool absolute, bool recursive);
    virtual void run();

    const QStringList &getResults() {return results;}

private:
    QString dirPath;
    QString nameFilter;
    bool absolute;
    bool recursive;
    QStringList results;

    static QFileInfoList scanDirectory(const QDir &dir, QFileInfoList &nestedDirs);
};

} // LocalWorkflow
} // U2

#endif // _GET_FILE_LIST_WORKER_
