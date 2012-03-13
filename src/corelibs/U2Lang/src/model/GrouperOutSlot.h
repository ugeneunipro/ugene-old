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

#ifndef _GROUPER_SLOT_ACTION_
#define _GROUPER_SLOT_ACTION_

#include <U2Core/global.h>

namespace U2 {

class U2LANG_EXPORT ActionTypes {
public:
    static const QString MERGE_SEQUENCE;
    static const QString SEQUENCE_TO_MSA;
    static const QString MERGE_MSA;
    static const QString MERGE_STRING;

    static bool isValidType(const QString &type);
};

class U2LANG_EXPORT ActionParameters {
public:
    enum ParameterType {
        INTEGER,
        BOOLEAN,
        STRING
    };

    static const QString GAP;
    static const QString UNIQUE;
    static const QString SEPARATOR;

    static ParameterType getType(const QString &parameter);
    static bool isValidParameter(const QString &actionType, const QString &parameter);
};

class U2LANG_EXPORT GrouperSlotAction {
public:
    GrouperSlotAction(const QString &type);

    QString getType() const;
    const QVariantMap &getParameters() const;
    QVariant getParameterValue(const QString &parameterId) const;
    void setParameterValue(const QString &parameterId, const QVariant &value);

private:
    QString type;
    QVariantMap parameters;
};

class U2LANG_EXPORT GrouperOutSlot {
public:
    GrouperOutSlot(const QString &outSlotId, const QString &inSlotStr);
    ~GrouperOutSlot();

    GrouperSlotAction *getAction();
    GrouperSlotAction *const getAction() const;
    void setAction(const GrouperSlotAction &action);

    QString getOutSlotId() const;
    void setOutSlotId(const QString &outSlotId);

    QString getInSlotStr() const;
    void setInSlotStr(const QString &inSlotStr);

private:
    QString outSlotId;
    QString inSlotStr;
    GrouperSlotAction *action;
};

} // U2

#endif _GROUPER_SLOT_ACTION_
