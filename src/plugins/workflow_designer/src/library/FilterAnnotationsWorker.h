/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_FILTER_ANNOTATIONS_WORKER_H_
#define _U2_FILTER_ANNOTATIONS_WORKER_H_

#include <U2Core/AnnotationData.h>

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>


namespace U2 {
namespace LocalWorkflow {

class FilterAnnotationsPrompter : public PrompterBase<FilterAnnotationsPrompter> {
    Q_OBJECT
public:
    FilterAnnotationsPrompter(Actor* p = 0) : PrompterBase<FilterAnnotationsPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class FilterAnnotationsWorker : public BaseWorker {
    Q_OBJECT
public:
    FilterAnnotationsWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {};

    virtual void init();
    virtual Task* tick();
    virtual void cleanup();
private slots:
    void sl_taskFinished(Task *t);
private:
    IntegralBus *input, *output;
    QList<AnnotationData> inputAnns;
}; 

class FilterAnnotationsWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    FilterAnnotationsWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) { return new FilterAnnotationsWorker(a); }
};

class FilterAnnotationsTask : public Task {
    Q_OBJECT
public:
    FilterAnnotationsTask(QList<AnnotationData>& annotations, const QString& names, bool accept)
        : Task(tr("Filter annotations task"), TaskFlag_None), annotations_(annotations), names_(names), accept_(accept) {}
    void run();

private:
    QStringList readAnnotationNames();
    
private:
    QList<AnnotationData>& annotations_;
    QString names_;
    bool accept_;
};

} // LocalWorkflow namespace
} // U2 namespace

#endif
