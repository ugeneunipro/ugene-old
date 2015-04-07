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

#ifndef _U2_GENEBYGENEREPORT_WORKER_H_
#define _U2_GENEBYGENEREPORT_WORKER_H_

#include <U2Core/U2OpStatus.h>
#include <U2Core/AnnotationData.h>

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include <QtCore/QMap>
#include <QtCore/QList>

namespace U2 {
namespace LocalWorkflow {

class GeneByGeneReportWorker : public BaseWorker {
    Q_OBJECT
public:
    GeneByGeneReportWorker(Actor *p);

    virtual void init();
    virtual Task *tick();
    virtual void cleanup();

    //for files in the report
    virtual QStringList getOutputFiles();

private:
    IntegralBus *inChannel;
    QStringList outFiles;
    QMap<QString, QPair<DNASequence, QList<SharedAnnotationData> > > geneData;

private slots:
    void sl_taskFinished();
}; // GeneByGeneReportWorker

class GeneByGeneReportWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;

    GeneByGeneReportWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker *createWorker(Actor *a);
}; // GeneByGeneReportWorkerFactory

class GeneByGeneReportPrompter : public PrompterBase<GeneByGeneReportPrompter> {
    Q_OBJECT
public:
    GeneByGeneReportPrompter(Actor *p = NULL) : PrompterBase<GeneByGeneReportPrompter>(p) {}

protected:
    QString composeRichDoc();

}; // GeneByGeneReportPrompter

} // LocalWorkflow
} // U2

#endif //_U2_GENEBYGENEREPORT_WORKER_H_
