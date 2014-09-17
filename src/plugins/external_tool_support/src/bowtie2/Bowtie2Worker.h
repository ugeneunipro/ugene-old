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

#ifndef _U2_BOWTIE2_SUPPORT_WORKER_
#define _U2_BOWTIE2_SUPPORT_WORKER_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

class DnaAssemblyToRefTaskSettings;
class U2OpStatus;

namespace U2 {
namespace LocalWorkflow {

class Bowtie2Worker : public BaseWorker {
    Q_OBJECT
public:
    Bowtie2Worker(Actor *p);

    virtual void init();
    virtual Task *tick();
    virtual void cleanup();

private:
    IntegralBus *inChannel;
    IntegralBus *output;

private:
    DnaAssemblyToRefTaskSettings getSettings(U2OpStatus &os);

private slots:
    void sl_taskFinished();
}; // Bowtie2Worker

class Bowtie2WorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;

    Bowtie2WorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker *createWorker(Actor *a);
}; // Bowtie2WorkerFactory

class Bowtie2Prompter : public PrompterBase<Bowtie2Prompter> {
    Q_OBJECT
public:
    Bowtie2Prompter(Actor *p = NULL) : PrompterBase<Bowtie2Prompter>(p) {}

protected:
    QString composeRichDoc();

}; // Bowtie2Prompter

} // LocalWorkflow
} // U2

#endif // _U2_BOWTIE2_SUPPORT_WORKER_
