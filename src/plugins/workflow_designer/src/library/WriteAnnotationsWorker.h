/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef __WRITE_ANNOTATIONS_WORKER_H_
#define __WRITE_ANNOTATIONS_WORKER_H_

#include <U2Core/SaveDocumentTask.h>

#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/LocalDomain.h>

namespace U2 {

class AnnotationTableObject;

namespace LocalWorkflow {

class WriteAnnotationsPrompter : public PrompterBase<WriteAnnotationsPrompter> {
    Q_OBJECT
public:
    WriteAnnotationsPrompter(Actor * p = NULL) : PrompterBase<WriteAnnotationsPrompter>(p) {}

protected:
    QString composeRichDoc();

}; // WriteAnnotationsPrompter

class WriteAnnotationsWorker : public BaseWorker {
    Q_OBJECT
public:
    enum DataStorage {
        LocalFs,
        SharedDb
    };

    WriteAnnotationsWorker(Actor * p) : BaseWorker(p), annotationsPort(NULL) {}
    ~WriteAnnotationsWorker();

    virtual void init();
    virtual Task * tick();
    virtual void cleanup();

private slots:
    void sl_saveDocTaskFinished();

private:
    Task * takeParameters(QString &formatId, SaveDocFlags &fl, QString &resultPath, U2DbiRef &dstDbiRef, DataStorage &storage);
    void updateResultPath(int metadataId, const QString &formatId, DataStorage storage, QString &resultPath);
    QString fetchIncomingSequenceName(const QVariantMap &incomingData);
    QString getAnnotationName() const;
    void fetchIncomingAnnotations(const QVariantMap &incomingData, const QString &resultPath);

    bool shouldAnnotationTablesBeMerged() const;
    AnnotationTableObject *mergeAnnotationTables(const QList<AnnotationTableObject *> &annTables, const QString &mergedTableName) const;

    Task * getSaveDocTask(const QString &formatId, SaveDocFlags &fl);
    Task * getSaveObjTask(const U2DbiRef &dstDbiRef) const;
    Task * createWriteMultitask(const QList<Task *> &taskList) const;

    IntegralBus * annotationsPort;
    QList<AnnotationTableObject *> createdAnnotationObjects;
    QMap<QString, QList<AnnotationTableObject *> > annotationsByUrl;

}; // WriteAnnotationsWorker

class WriteAnnotationsWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;

    WriteAnnotationsWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker * createWorker(Actor* a);

}; // WriteAnnotationsWorkerFactory

}
} // U2

#endif // __WRITE_ANNOTATIONS_WORKER_H_
