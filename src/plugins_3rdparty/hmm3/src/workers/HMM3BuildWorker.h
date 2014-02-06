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

#ifndef _U2_HMM3_BUILD_WORKER_H_
#define _U2_HMM3_BUILD_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include <hmmer3/hmmer.h>

namespace U2 {

namespace LocalWorkflow {

class HMM3BuildPrompter : public PrompterBase<HMM3BuildPrompter> {
    Q_OBJECT
public:
    HMM3BuildPrompter(Actor* p = 0) : PrompterBase<HMM3BuildPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class HMM3BuildWorker : public BaseWorker {
    Q_OBJECT
public:
    HMM3BuildWorker(Actor* a);
    
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();
    
private slots:
    void sl_taskFinished(Task*);
    void sl_taskFinished();
    
protected:
    IntegralBus *input, *output;
    UHMM3BuildSettings cfg;
}; 

class HMM3BuildWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR;
    static void init();
    static void cleanup();
    HMM3BuildWorkerFactory() : DomainFactory(ACTOR) {}
    virtual Worker* createWorker(Actor* a) {return new HMM3BuildWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
