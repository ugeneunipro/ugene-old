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

#ifndef _U2_BLASTPLUS_WORKER_H_
#define _U2_BLASTPLUS_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "BlastPlusSupportCommonTask.h"

namespace U2 {

class ExternalTool;

namespace LocalWorkflow {

class BlastPlusPrompter : public PrompterBase<BlastPlusPrompter> {
    Q_OBJECT
public:
    BlastPlusPrompter(Actor* p = 0);
protected:
    QString composeRichDoc();
};

class BlastPlusWorker : public BaseWorker {
    Q_OBJECT
public:
    BlastPlusWorker(Actor* a);
    
    virtual void init();
    virtual Task* tick();
    virtual void cleanup();
    
private slots:
    void sl_taskFinished();

protected:
    IntegralBus *input, *output;
    QString resultName,transId;
    BlastTaskSettings   cfg;
    
}; 

class BlastPlusWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;

    static QString getHitsName();
    static QString getHitsDescription();

    static void init();
    BlastPlusWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new BlastPlusWorker(a);}
};

class ToolsValidator : public ActorValidator {
public:
    virtual bool validate(const Actor *actor, ProblemList &problemList, const QMap<QString, QString> &options) const;

private:
    ExternalTool * getTool(const QString &program) const;
};

} // Workflow namespace
} // U2 namespace

#endif
