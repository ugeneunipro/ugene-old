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

#include <U2Lang/WorkflowEnv.h>

#include "BaseTypes.h"

namespace U2 {

static const QString DNA_SEQUENCE_TYPE_ID("seq");
static const QString ANNOTATION_TABLE_LIST_TYPE_ID("ann-table-list");
static const QString ANNOTATION_TABLE_TYPE_ID("ann_table");
static const QString MULTIPLE_ALIGNMENT_TYPE_ID("malignment");
static const QString VARIATION_TRACK_TYPE_ID("variation");
static const QString ASSEMBLY_TYPE_ID("assembly");
static const QString STRING_TYPE_ID("string");
static const QString STRING_LIST_TYPE_ID("string-list");
static const QString BOOL_TYPE_ID("bool");
static const QString NUM_TYPE_ID("number");
static const QString ANY_TYPE_ID("void");

using namespace Workflow;

DataTypePtr BaseTypes::DNA_SEQUENCE_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup) {
        dtr->registerEntry(DataTypePtr(new DataType(DNA_SEQUENCE_TYPE_ID, tr("Sequence"), tr("A sequence"))));
        startup = false;
    }
    return dtr->getById(DNA_SEQUENCE_TYPE_ID);
}

DataTypePtr BaseTypes::ANNOTATION_TABLE_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup) {
        dtr->registerEntry(DataTypePtr(new DataType(ANNOTATION_TABLE_TYPE_ID, tr("Set of annotations"), tr("A set of annotated features in a sequence"))));
        startup = false;
    }
    return dtr->getById(ANNOTATION_TABLE_TYPE_ID);
}

DataTypePtr BaseTypes::ANNOTATION_TABLE_LIST_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup) {
        dtr->registerEntry(DataTypePtr(new ListDataType(
            Descriptor(ANNOTATION_TABLE_LIST_TYPE_ID, tr("List of annotations"), tr("A list of sequence annotations")),
            BaseTypes::ANNOTATION_TABLE_TYPE())));
        startup = false;
    }
    return dtr->getById(ANNOTATION_TABLE_LIST_TYPE_ID);
}

DataTypePtr BaseTypes::MULTIPLE_ALIGNMENT_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup) {
        dtr->registerEntry(DataTypePtr(new DataType(MULTIPLE_ALIGNMENT_TYPE_ID, tr("Multiple alignment"), tr("Set of aligned sequences"))));
        startup = false;
    }
    return dtr->getById(MULTIPLE_ALIGNMENT_TYPE_ID);
}

DataTypePtr BaseTypes::VARIATION_TRACK_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup) {
        dtr->registerEntry(DataTypePtr(new DataType(VARIATION_TRACK_TYPE_ID, tr("Variation track"), tr("Set of variations"))));
        startup = false;
    }
    return dtr->getById(VARIATION_TRACK_TYPE_ID);
}

DataTypePtr BaseTypes::ASSEMBLY_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup) {
        dtr->registerEntry(DataTypePtr(new DataType(ASSEMBLY_TYPE_ID, tr("Assembly data"), tr("Assembly data"))));
        startup = false;
    }
    return dtr->getById(ASSEMBLY_TYPE_ID);
}

DataTypePtr BaseTypes::STRING_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup)
    {
        dtr->registerEntry(DataTypePtr(new DataType(STRING_TYPE_ID, tr("String"), tr("A string of characters"))));
        startup = false;
    }
    return dtr->getById(STRING_TYPE_ID);
}

DataTypePtr BaseTypes::STRING_LIST_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup)
    {
        dtr->registerEntry(DataTypePtr(new ListDataType(STRING_LIST_TYPE_ID, BaseTypes::STRING_TYPE())));
        startup = false;
    }
    return dtr->getById(STRING_LIST_TYPE_ID);
}

DataTypePtr BaseTypes::BOOL_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup)
    {
        dtr->registerEntry(DataTypePtr(new DataType(BOOL_TYPE_ID, tr("Boolean"), tr("A boolean value (true/false)"))));
        startup = false;
    }
    return dtr->getById(BOOL_TYPE_ID);
}

DataTypePtr BaseTypes::NUM_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup)
    {
        dtr->registerEntry(DataTypePtr(new DataType(NUM_TYPE_ID, tr("Number"), tr("A number"))));
        startup = false;
    }
    return dtr->getById(NUM_TYPE_ID);
}

DataTypePtr BaseTypes::ANY_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup)
    {
        dtr->registerEntry(DataTypePtr(new DataType(ANY_TYPE_ID, tr("Undefined"), tr("A void type"))));
        startup = false;
    }
    return dtr->getById(ANY_TYPE_ID);
}

static void setIfNotNull( bool * to, bool val ) {
    if( to != NULL ) {
        *to = val;
    }
}

/****************************************
* StringTypeValueFactory
****************************************/
QVariant StringTypeValueFactory::getValueFromString( const QString & str, bool * ok ) const {
    setIfNotNull( ok, true );
    return qVariantFromValue( str );
}

/****************************************
* BoolTypeValueFactory
****************************************/
const QString BoolTypeValueFactory::TRUE_STR    = "true";
const QString BoolTypeValueFactory::FALSE_STR   = "false";
const QString BoolTypeValueFactory::YES_STR     = "yes";
const QString BoolTypeValueFactory::NO_STR      = "no";
const QString BoolTypeValueFactory::ONE_STR     = "1";
const QString BoolTypeValueFactory::NIL_STR     = "0";

QVariant BoolTypeValueFactory::getValueFromString( const QString & s, bool * ok ) const {
    QString str = s.toLower();
    if( str == TRUE_STR || str == YES_STR || str == ONE_STR ) {
        setIfNotNull( ok, true );
        return qVariantFromValue( true );
    }
    if( str == FALSE_STR || str == NO_STR || str == NIL_STR ) {
        setIfNotNull( ok, true );
        return qVariantFromValue( false );
    }
    setIfNotNull( ok, false );
    return QVariant();
}

/****************************************
* NumTypeValueFactory
****************************************/
QVariant NumTypeValueFactory::getValueFromString( const QString & str, bool * okArg ) const {
    bool ok = false;
    int intCandidate = str.toInt(&ok);
    if(ok) {
        setIfNotNull(okArg, true);
        return qVariantFromValue(intCandidate);
    }

    double doubleCandidate = str.toDouble(&ok);
    if(ok) {
        setIfNotNull(okArg, true);
        return qVariantFromValue(doubleCandidate);
    }

    setIfNotNull(okArg, false);
    return QVariant();
}

} // U2
