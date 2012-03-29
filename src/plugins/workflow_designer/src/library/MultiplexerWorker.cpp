/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "MultiplexerWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString MultiplexerWorkerFactory::ACTOR_ID("multiplexer");

static const QString EMPTY_TYPESET_ID("empty");
static const QString INPUT_PORT_1("input-data-1");
static const QString INPUT_PORT_2("input-data-2");
static const QString OUTPUT_PORT("output-data");

static const QString RULE_ID("multiplexing-rule");
static const QString EMPTY_ACTION_ID("empty-input-action");

/*******************************
 * MultiplexerWorker
 *******************************/
MultiplexerWorker::MultiplexerWorker(Actor *p)
: BaseWorker(p, false), inChannel1(NULL), inChannel2(NULL), outChannel(NULL), rule(ONE_TO_ONE)
{
}

void MultiplexerWorker::init() {
    rule = actor->getParameter(RULE_ID)->getAttributeValue<uint>(context);
    onEmptyAction = actor->getParameter(EMPTY_ACTION_ID)->getAttributeValue<uint>(context);

    if (MANY_TO_ONE == rule) {
        inChannel1 = ports.value(INPUT_PORT_2);
        inChannel2 = ports.value(INPUT_PORT_1);
    } else {
        inChannel1 = ports.value(INPUT_PORT_1);
        inChannel2 = ports.value(INPUT_PORT_2);
    }

    outChannel = ports.value(OUTPUT_PORT);
    hasMultiData = false;
    messagesInited = false;
}

bool MultiplexerWorker::isReady() {
    if (isDone()) {
        return false;
    }

    int hasMsg1 = inChannel1->hasMessage();
    int hasMsg2 = inChannel2->hasMessage();
    bool ended1 = inChannel1->isEnded();
    bool ended2 = inChannel2->isEnded();

    if (ONE_TO_MANY == rule || MANY_TO_ONE == rule) {
        if (hasMsg1 && hasMsg2) {
            return true;
        } else if (hasMsg1) {
            return ended2;
        } else if (hasMultiData || ended1) {
            return hasMsg2 || ended2;
        }
    } else if (ONE_TO_ONE == rule) {
        if (hasMsg1 && hasMsg2) {
            return true;
        } else if (hasMsg1) {
            return ended2;
        } else if (hasMsg2) {
            return ended1;
        }
    } else if (STREAM ==  rule) {
        return hasMsg1 || hasMsg2;
    }

    return ended1 && ended2;
}

void MultiplexerWorker::shutDown() {
    outChannel->setEnded();
    setDone();
}

bool MultiplexerWorker::checkIfEnded() {
    if (inChannel1->isEnded() && inChannel2->isEnded()) {
        shutDown();
        return true;
    }
    return false;
}

bool MultiplexerWorker::hasDataFotMultiplexing() const {
    return inChannel1->hasMessage() || hasMultiData;
}

inline void MultiplexerWorker::sendUnitedMessage(const QVariantMap &m1, QVariantMap &m2) {
    m2.unite(m1);
    outChannel->putWithoutContext(Message(outChannel->getBusType(), m2));
}

Task *MultiplexerWorker::tick() {
    if (ONE_TO_MANY == rule || MANY_TO_ONE == rule) {
        multiplexManyMode();
    } else if (ONE_TO_ONE == rule) {
        if (checkIfEnded()) {
            return NULL;
        }

        bool bothData = inChannel1->hasMessage() && inChannel2->hasMessage();
        if (!bothData && TRUNCATE == onEmptyAction) {
            shutDown();
            return NULL;
        }

        QVariantMap m1, m2;
        if (inChannel1->hasMessage()) {
            m1 = inChannel1->look().getData().toMap();
            inChannel1->get();
        }
        if (inChannel2->hasMessage()) {
            m2 = inChannel2->look().getData().toMap();
            inChannel2->get();
        }

        sendUnitedMessage(m1, m2);
        checkIfEnded();
    } else {
        QVariantMap m1, m2;
        bool hasMessage = inChannel1->hasMessage() || inChannel2->hasMessage();
        if (inChannel1->hasMessage()) {
            m1 = inChannel1->look().getData().toMap();
            inChannel1->get();
        } else if (inChannel2->hasMessage()) {
            m2 = inChannel2->look().getData().toMap();
            inChannel2->get();
        }

        if (hasMessage) {
            sendUnitedMessage(m1, m2);
        }
        checkIfEnded();
    }
    return NULL;
}

void MultiplexerWorker::multiplexManyMode() {
    if (!hasDataFotMultiplexing()) {
        if (checkIfEnded()) {
            return;
        }
    }
    QVariantMap m1;
    if (hasMultiData) {
        m1 = multiData;
    } else {
        if (inChannel1->hasMessage()) {
            m1 = inChannel1->look().getData().toMap();
            inChannel1->get(); // pop last message
        } else if (TRUNCATE == onEmptyAction) {
            shutDown();
        }
        hasMultiData = true;
        multiData = m1;
    }

    if (messagesInited) {
        if (messages.isEmpty()) {
            if (TRUNCATE == onEmptyAction) {
                shutDown();
            } else {
                QVariantMap m2;
                sendUnitedMessage(m1, m2);
            }
        } else {
            foreach (QVariantMap m2, messages) {
                sendUnitedMessage(m1, m2);
            }
        }
        hasMultiData = false;
        multiData.clear();
    } else {
        while (inChannel2->hasMessage()) {
            QVariantMap m2 = inChannel2->look().getData().toMap();
            inChannel2->get(); // pop last message
            messages << m2;

            sendUnitedMessage(m1, m2);
        }
        if (inChannel2->isEnded()) {
            if (messages.isEmpty()) {
                if (TRUNCATE == onEmptyAction) {
                    shutDown();
                } else {
                    QVariantMap m2;
                    sendUnitedMessage(m1, m2);
                }
            }
            messagesInited = true;
            hasMultiData = false;
            multiData.clear();
        }
    }
    if (!hasMultiData && inChannel1->isEnded()) { // nothing else to multiplex
        outChannel->setEnded();
        setDone();
    }
}

void MultiplexerWorker::cleanup() {
}

/*******************************
 * MultiplexerWorkerFactory
 *******************************/
void MultiplexerWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> emptyTypeMap;
        DataTypePtr emptyTypeSet(new MapDataType(Descriptor(DataType::EMPTY_TYPESET_ID), emptyTypeMap));

        // input ports
        Descriptor inputDesc1(INPUT_PORT_1, MultiplexerWorker::tr("First input data flow"), MultiplexerWorker::tr("First input data flow"));
        Descriptor inputDesc2(INPUT_PORT_2, MultiplexerWorker::tr("Second input data flow"), MultiplexerWorker::tr("Second input data flow"));
        portDescs << new PortDescriptor(inputDesc1, emptyTypeSet, true);
        portDescs << new PortDescriptor(inputDesc2, emptyTypeSet, true);

        // output port
        Descriptor outputDesc(OUTPUT_PORT, MultiplexerWorker::tr("Multiplexed output data flow"), MultiplexerWorker::tr("Multiplexed output data flow"));
        portDescs << new PortDescriptor(outputDesc, emptyTypeSet, false, true);
    }

    QList<Attribute*> attrs;
    {
        // attributes
        Descriptor ruleDesc(RULE_ID, MultiplexerWorker::tr("Multiplexing rule"), MultiplexerWorker::tr("Multiplexing rule"));
        Descriptor actionDesc(EMPTY_ACTION_ID, MultiplexerWorker::tr("If empty input"), MultiplexerWorker::tr("How to multiplex the data if one of input ports produces no data"));

        Attribute *ruleAttr = new Attribute(ruleDesc, BaseTypes::STRING_TYPE(), true, ONE_TO_ONE);
        Attribute *actionAttr = new Attribute(actionDesc, BaseTypes::STRING_TYPE(), true, FILL_EMPTY);

        attrs << ruleAttr;
        attrs << actionAttr;
    }

    QMap<QString, PropertyDelegate*> delegateMap;
    {
        // delegates
        QVariantMap rules;
        rules[MultiplexerWorker::tr("1 to many")] = ONE_TO_MANY;
        rules[MultiplexerWorker::tr("Many to 1")] = MANY_TO_ONE;
        rules[MultiplexerWorker::tr("1 to 1")] = ONE_TO_ONE;
        rules[MultiplexerWorker::tr("Streaming mode")] = STREAM;

        QVariantMap actions;
        actions[MultiplexerWorker::tr("Fill by empty values")] = FILL_EMPTY;
        actions[MultiplexerWorker::tr("Truncate")] = TRUNCATE;

        delegateMap[RULE_ID] = new ComboBoxDelegate(rules);
        delegateMap[EMPTY_ACTION_ID] = new ComboBoxDelegate(actions);
    }
   
    Descriptor protoDesc(MultiplexerWorkerFactory::ACTOR_ID,
        MultiplexerWorker::tr("Multiplexer"),
        MultiplexerWorker::tr("Multiplexes two input data flows using the set multiplexing rule."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);

    proto->setEditor(new DelegateEditor(delegateMap));
    proto->setPrompter(new MultiplexerPrompter());
    proto->setAllowsEmptyPorts(true);
    
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATAFLOW(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new MultiplexerWorkerFactory());
}

Worker *MultiplexerWorkerFactory::createWorker(Actor* a) {
    return new MultiplexerWorker(a);
}

/*******************************
 * MultiplexerPrompter
 *******************************/
QString MultiplexerPrompter::composeRichDoc() {
    uint rule = getParameter(RULE_ID).toUInt();

    IntegralBusPort* input1 = NULL;
    IntegralBusPort* input2 = NULL;
    if (MANY_TO_ONE == rule) {
        input1 = qobject_cast<IntegralBusPort*>(target->getPort(INPUT_PORT_2));
        input2 = qobject_cast<IntegralBusPort*>(target->getPort(INPUT_PORT_1));
    } else {
        input1 = qobject_cast<IntegralBusPort*>(target->getPort(INPUT_PORT_1));
        input2 = qobject_cast<IntegralBusPort*>(target->getPort(INPUT_PORT_2));
    }

    QString unsetStr = "<font color='red'>" + tr("unset") + "</font>";
    QString inputName1 = unsetStr;
    if (input1->getLinks().size() > 0) {
        Port *p = input1->getLinks().keys().first();
        inputName1 = p->owner()->getLabel() + "." + p->getDisplayName();
    }
    QString inputName2 = unsetStr;
    if (input2->getLinks().size() > 0) {
        Port *p = input2->getLinks().keys().first();
        inputName2 = p->owner()->getLabel() + "." + p->getDisplayName();
    }

    if (ONE_TO_ONE == rule) {
        return tr("For every message from <u>%1</u> it gets only one message from <u>%2</u> and puts them to the output").arg(inputName1).arg(inputName2);
    } else if (STREAM == rule) {
        return tr("Puts every message from <u>%1</u> and <u>%2</u> to the output").arg(inputName1).arg(inputName2);
    } else {
        return tr("For every message from <u>%1</u> it gets every message from <u>%2</u> and puts them to the output").arg(inputName1).arg(inputName2);
    }
}

} // LocalWorkflow
} // U2
