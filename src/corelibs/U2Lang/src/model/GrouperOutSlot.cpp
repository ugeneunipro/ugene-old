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

#include "GrouperOutSlot.h"

namespace U2 {

/************************************************************************/
/* ActionTypes */
/************************************************************************/
const QString ActionTypes::MERGE_SEQUENCE       = QString("merge-sequence");
const QString ActionTypes::SEQUENCE_TO_MSA      = QString("sequences-to-msa");
const QString ActionTypes::MERGE_MSA            = QString("merge-msa");
const QString ActionTypes::MERGE_STRING         = QString("merge-string");

bool ActionTypes::isValidType(const QString &type) {
    bool result = false;

    result |= (MERGE_SEQUENCE == type);
    result |= (SEQUENCE_TO_MSA == type);
    result |= (MERGE_MSA == type);
    result |= (MERGE_STRING == type);

    return result;
}

/************************************************************************/
/* ActionParameters */
/************************************************************************/
const QString ActionParameters::GAP             = QString("gap");
const QString ActionParameters::UNIQUE          = QString("unique");
const QString ActionParameters::SEPARATOR       = QString("separator");

ActionParameters::ParameterType ActionParameters::getType(const QString &parameter) {
    if (GAP == parameter) {
        return INTEGER;
    } else if (UNIQUE == parameter) {
        return BOOLEAN;
    } else if (SEPARATOR == parameter) {
        return STRING;
    }

    assert(NULL);
    return STRING;
}

bool ActionParameters::isValidParameter(const QString &actionType, const QString &parameter) {
    bool result = false;
    if (ActionTypes::MERGE_SEQUENCE == actionType) {
        result |= (GAP == parameter);
    } else if (ActionTypes::SEQUENCE_TO_MSA == actionType) {
        result |= (UNIQUE == parameter);
    } else if (ActionTypes::MERGE_MSA == actionType) {
        result |= (UNIQUE == parameter);
    } else if (ActionTypes::MERGE_STRING == actionType) {
        result |= (SEPARATOR == parameter);
    }

    return result;
}

/************************************************************************/
/* GrouperSlotAction */
/************************************************************************/
GrouperSlotAction::GrouperSlotAction(const QString &type)
: type(type)
{

}

QString GrouperSlotAction::getType() const {
    return type;
}

const QVariantMap &GrouperSlotAction::getParameters() const {
    return parameters;
}

QVariant GrouperSlotAction::getParameterValue(const QString &parameterId) const {
    return parameters.value(parameterId, QVariant());
}

void GrouperSlotAction::setParameterValue(const QString &parameterId, const QVariant &value) {
    parameters[parameterId] = value;
}

/************************************************************************/
/* GrouperOutSlot */
/************************************************************************/
GrouperOutSlot::GrouperOutSlot(const QString &outSlotId, const QString &inSlotStr)
: outSlotId(outSlotId), inSlotStr(inSlotStr), action(NULL)
{

}

GrouperOutSlot::~GrouperOutSlot() {
    delete action;
}

GrouperSlotAction *GrouperOutSlot::getAction() {
    return action;
}

GrouperSlotAction *const GrouperOutSlot::getAction() const {
    return action;
}

void GrouperOutSlot::setAction(const GrouperSlotAction &action) {
    this->action = new GrouperSlotAction(action);
}

QString GrouperOutSlot::getOutSlotId() const {
    return outSlotId;
}

void GrouperOutSlot::setOutSlotId(const QString &outSlotId) {
    this->outSlotId = outSlotId;
}

QString GrouperOutSlot::getInSlotStr() const {
    return inSlotStr;
}

void GrouperOutSlot::setInSlotStr(const QString &inSlotStr) {
    this->inSlotStr = inSlotStr;
}

} // U2
