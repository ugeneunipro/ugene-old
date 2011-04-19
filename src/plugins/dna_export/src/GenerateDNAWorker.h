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

#ifndef _U2_GENERATE_DNA_WORKER_H_
#define _U2_GENERATE_DNA_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>


namespace U2 {
namespace LocalWorkflow {

class GenerateDNAPrompter : public PrompterBase<GenerateDNAPrompter> {
    Q_OBJECT
public:
    GenerateDNAPrompter(Actor* p = 0) : PrompterBase<GenerateDNAPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class GenerateDNAWorker : public BaseWorker {
    Q_OBJECT
public:
    GenerateDNAWorker(Actor* a) : BaseWorker(a), ch(NULL), done(false) {}

    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup() {}

private slots:
    void sl_taskFinished(Task*);

private:
    CommunicationChannel* ch;
    bool done;
};

class GenerateDNAWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    GenerateDNAWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) { return new GenerateDNAWorker(a); }
};

} // LocalWorkflow namespace
} // U2 namespace

#endif
