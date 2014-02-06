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

#ifndef _U2_HMM3SEARCH_WORKER_H_
#define _U2_HMM3SEARCH_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include "hmmer3/hmmer.h"

namespace U2 {

namespace LocalWorkflow {

class HMM3SearchPrompter : public PrompterBase<HMM3SearchPrompter> {
    Q_OBJECT
public:
    HMM3SearchPrompter(Actor* p = 0) : PrompterBase<HMM3SearchPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class HMM3SearchWorker : public BaseWorker {
    Q_OBJECT
public:
    HMM3SearchWorker(Actor* a);
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual void cleanup();
    
private slots:
    void sl_taskFinished(Task*);

protected:
    IntegralBus *hmmPort, *seqPort, *output;
    QString resultName;
    UHMM3SearchSettings cfg;
    QList<const P7_HMM*> hmms;
    
}; 

class HMM3SearchWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR;
    static void init();
    HMM3SearchWorkerFactory() : DomainFactory(ACTOR) {}
    virtual Worker* createWorker(Actor* a) {return new HMM3SearchWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
