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

#ifndef _U2_BLASTALL_WORKER_H_
#define _U2_BLASTALL_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "BlastAllSupportTask.h"
#include "utils/BlastTaskSettings.h"

namespace U2 {

namespace LocalWorkflow {

class BlastAllPrompter : public PrompterBase<BlastAllPrompter> {
    Q_OBJECT
public:
    BlastAllPrompter(Actor* p = 0);
protected:
    QString composeRichDoc();
};

class BlastAllWorker : public BaseWorker {
    Q_OBJECT
public:
    BlastAllWorker(Actor* a);
    
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();
    
private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *input, *output;
    QString resultName,transId;
    BlastTaskSettings cfg;
    
}; 

class BlastAllWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    BlastAllWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new BlastAllWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
