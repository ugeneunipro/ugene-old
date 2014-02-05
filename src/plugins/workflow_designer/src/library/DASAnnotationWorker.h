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

#ifndef _U2_DAS_ANNOTATION_WORKER_H_
#define _U2_DAS_ANNOTATION_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {

namespace LocalWorkflow {

class DASAnnotationPrompter;
typedef PrompterBase<DASAnnotationPrompter> DASAnnotationPrompterBase;

class DASAnnotationPrompter : public DASAnnotationPrompterBase {
    Q_OBJECT
public:
    DASAnnotationPrompter(Actor* p = 0) : DASAnnotationPrompterBase(p) {}
protected:
    QString composeRichDoc();
};

class DASAnnotationWorker : public BaseWorker {
    Q_OBJECT
public:
    DASAnnotationWorker(Actor* a);
    
    virtual void init();
    virtual Task* tick();
    virtual void cleanup();
    
private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *input, *output;
}; 

class DASAnnotationWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    DASAnnotationWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new DASAnnotationWorker(a);}
};

}// Workflow namespace
}// U2 namespace

#endif //_U2_DAS_ANNOTATION_WORKER_H_
