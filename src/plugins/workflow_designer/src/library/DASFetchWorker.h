/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_DAS_FETCHER_H_
#define _U2_DAS_FETCHER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {
namespace LocalWorkflow {

class DASFetcherPrompter : public PrompterBase<DASFetcherPrompter>
{
    Q_OBJECT

public:
    DASFetcherPrompter(Actor *p = 0) : PrompterBase<DASFetcherPrompter>(p) {}

protected:
    virtual QString composeRichDoc();
};

class DASFetcherWorker : public BaseWorker
{
    Q_OBJECT

public:
    DASFetcherWorker (Actor *a);

    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();

private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *output;

    QString dbid;
    QString fullPathDir;
    QStringList featureids;
    QStringList seqids;
    
};

class DASFetcherFactory : public DomainFactory
{
public:
    static const QString ACTOR_ID;
    
public:
    DASFetcherFactory() : DomainFactory(ACTOR_ID) {}

    static void init();
    virtual Worker* createWorker(Actor *a) { return new DASFetcherWorker(a); }
};

}   // namespace U2
}   // namespace LocalWorkflow

#endif  // #ifndef _U2_DAS_FETCHER_H_
