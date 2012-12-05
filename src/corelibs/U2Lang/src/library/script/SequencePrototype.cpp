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

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "SequencePrototype.h"

namespace U2 {

const QString SequenceScriptClass::SCRIPT_CLASS_NAME("Sequence");

/************************************************************************/
/* SequencePrototype */
/************************************************************************/
SequencePrototype::SequencePrototype(QObject *parent)
: DbiClassPrototype(parent)
{

}

U2SequenceObject * SequencePrototype::getSequenceObject() const {
    CHECK(NULL != thisData(), NULL);
    Workflow::DbiDataStorage *storage = dataStorage();
    CHECK(NULL != storage, NULL);
    return Workflow::StorageUtils::getSequenceObject(storage, thisData()->getId());
}

U2SequenceObject * SequencePrototype::getValidSequenceObject() const {
    U2SequenceObject *result = getSequenceObject();
    if (NULL == result) {
        coreLog.error("Invalid sequence object");
    }
    return result;
}

qint64 SequencePrototype::length() {
    QScopedPointer<U2SequenceObject> seqObj(getValidSequenceObject());
    CHECK(!seqObj.isNull(), 0);
    return seqObj->getSequenceLength();
}

QString SequencePrototype::getData() {
    QScopedPointer<U2SequenceObject> seqObj(getValidSequenceObject());
    CHECK(!seqObj.isNull(), "");

    U2Region r = U2_REGION_MAX;
    if (context()->argumentCount() > 0) {
        r.startPos = context()->argument(0).toNumber();
        CHECK(r.startPos >= 0, "");
        if (context()->argumentCount() > 1) {
            r.length = context()->argument(1).toNumber();
        } else {
            r.length -= r.startPos;
        }
    }
    return seqObj->getSequenceData(r);
}

/************************************************************************/
/* SequenceScriptClass */
/************************************************************************/
SequenceScriptClass::SequenceScriptClass(QScriptEngine *engine)
: DbiScriptClass(engine)
{
    qScriptRegisterMetaType<ScriptDbiData>(engine, toScriptValue<SequenceScriptClass>, fromScriptValue);
    proto = engine->newQObject(new SequencePrototype(this));
}

QScriptValue SequenceScriptClass::constructor(QScriptContext *ctx, QScriptEngine *engine) {
    SequenceScriptClass *sClass = qscriptvalue_cast<SequenceScriptClass*>(ctx->callee().data());
    if (!sClass) {
        return QScriptValue();
    }

    if (1 == ctx->argumentCount()) {
        QScriptValue arg = ctx->argument(0);
        if (arg.instanceOf(ctx->callee())) {
            return sClass->DbiScriptClass::newInstance(qscriptvalue_cast<ScriptDbiData>(arg));
        } else if (arg.isVariant()) {
            QVariant var = arg.toVariant();
            if (var.canConvert<Workflow::SharedDbiDataHandler>()) {
                Workflow::SharedDbiDataHandler id = var.value<Workflow::SharedDbiDataHandler>();
                return sClass->DbiScriptClass::newInstance(ScriptDbiData(id));
            }
        }
    } else if (2 == ctx->argumentCount()) {
        QString data = ctx->argument(0).toString();
        QString name = ctx->argument(1).toString();
        return sClass->newInstance(data, name);
    }
    return QScriptValue();
}

QScriptValue SequenceScriptClass::newInstance(const QString &data, const QString &name) {
    Workflow::DbiDataStorage *storage = workflowEngine()->getWorkflowContext()->getDataStorage();
    Workflow::SharedDbiDataHandler id = storage->putSequence(DNASequence(name, data.toAscii()));
    return DbiScriptClass::newInstance(ScriptDbiData(id));
}

QString SequenceScriptClass::name() const {
    return SCRIPT_CLASS_NAME;
}

WorkflowScriptEngine * SequenceScriptClass::workflowEngine() const {
    return dynamic_cast<WorkflowScriptEngine*>(engine());
}

} // U2
