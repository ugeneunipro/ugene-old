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

#ifndef __TEXT_2_SEQUENCE_WORKER_H_
#define __TEXT_2_SEQUENCE_WORKER_H_

#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/LocalDomain.h>

namespace U2 {
namespace LocalWorkflow {

class Text2SequencePrompter : public PrompterBase<Text2SequencePrompter> {
    Q_OBJECT
public:
    Text2SequencePrompter(Actor * p = NULL) : PrompterBase<Text2SequencePrompter>(p) {}

protected:
    QString composeRichDoc();

}; // Text2SequencePrompter

class Text2SequenceWorker : public BaseWorker {
    Q_OBJECT
public:
    static QMap<QString, QString> cuteAlIdNames;

public:
    Text2SequenceWorker(Actor * p) : BaseWorker(p), txtPort(NULL), outSeqPort(NULL), tickedNum(0) {}

    virtual void init();
    virtual Task * tick();
    virtual void cleanup();

private:
    static QMap<QString, QString> initCuteAlNames();

private:
    IntegralBus * txtPort;
    IntegralBus * outSeqPort;
    int tickedNum;

}; // Text2SequenceWorker

class Text2SequenceWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;

    Text2SequenceWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker * createWorker(Actor* a);

}; // Text2SequenceWorkerFactory

} // LocalWorkflow
} // U2

#endif // __TEXT_2_SEQUENCE_WORKER_H_
