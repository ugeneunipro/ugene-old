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

#include <U2Lang/MarkerUtils.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Core/QVariantUtils.h>

#include "Marker.h"

static const QString LENGTH_SLOT_ID("length-marker");
static const QString ANN_COUNT_SLOT_ID("ann-count-marker");
static const QString ANN_VALUE_SLOT_ID("ann-value-marker");
static const QString FILENAME_SLOT_ID("filename-marker");

namespace U2 {

const QString MarkerTypes::SEQ_LENGTH_MARKER_ID("sequence-length");
const QString MarkerTypes::ANNOTATION_COUNT_MARKER_ID("annotations-count");
const QString MarkerTypes::ANNOTATION_LENGTH_MARKER_ID("annotation-length");
const QString MarkerTypes::QUAL_INT_VALUE_MARKER_ID("qualifier-int-value");
const QString MarkerTypes::QUAL_TEXT_VALUE_MARKER_ID("qualifier-text-value");
const QString MarkerTypes::QUAL_FLOAT_VALUE_MARKER_ID("qualifier-float-value");
const QString MarkerTypes::TEXT_MARKER_ID("text");

MarkerDataType MarkerTypes::getDataTypeById(const QString &typeId) {
    if (SEQ_LENGTH_MARKER_ID == typeId) {
        return INTEGER;
    } else if (ANNOTATION_COUNT_MARKER_ID == typeId) {
        return INTEGER;
    } else if (ANNOTATION_LENGTH_MARKER_ID == typeId) {
        return INTEGER;
    } else if (QUAL_INT_VALUE_MARKER_ID == typeId) {
        return INTEGER;
    } else if (QUAL_TEXT_VALUE_MARKER_ID == typeId) {
        return STRING;
    } else if (QUAL_FLOAT_VALUE_MARKER_ID == typeId) {
        return FLOAT;
    } else if (TEXT_MARKER_ID == typeId) {
        return STRING;
    } else {
        assert(0);
        return MarkerDataType();
    }
}

const Descriptor MarkerAttributes::LENGTH_MARKER_ATTRIBUTE() {
    return Descriptor(MarkerTypes::SEQ_LENGTH_MARKER_ID, tr("Length markers"), tr("Length markers group."));
}
const Descriptor MarkerAttributes::ANNOTATION_COUNT_MARKER_ATTRIBUTE() {
    return Descriptor(MarkerTypes::ANNOTATION_COUNT_MARKER_ID, tr("Annotation count markers"), tr("Annotation count markers group."));
}
const Descriptor MarkerAttributes::ANNOTATION_VALUE_MARKER_ATTRIBUTE() {
    return Descriptor(MarkerTypes::QUAL_INT_VALUE_MARKER_ID, tr("Annotation value markers"), tr("Annotation value markers group."));
}
const Descriptor MarkerAttributes::FILENAME_MARKER_ATTRIBUTE() {
    return Descriptor(MarkerTypes::TEXT_MARKER_ID, tr("Filename markers"), tr("Filename markers group."));
}

const Descriptor MarkerSlots::getSlotByMarkerType(const QString &markerId, const QString &slotName) {
    if (markerId == MarkerTypes::SEQ_LENGTH_MARKER_ID) {
        return Descriptor(slotName, tr("Sequence length marker"), tr("Sequence length marker."));
    } else if (markerId == MarkerTypes::ANNOTATION_COUNT_MARKER_ID) {
        return Descriptor(slotName, tr("Annotation count marker"), tr("Annotation count marker."));
    } else if (markerId == MarkerTypes::ANNOTATION_LENGTH_MARKER_ID) {
        return Descriptor(slotName, tr("Annotation length marker"), tr("Annotation length marker."));
    } else if (markerId == MarkerTypes::QUAL_INT_VALUE_MARKER_ID) {
        return Descriptor(slotName, tr("Qualifier integer value marker"), tr("Qualifier integer value marker."));
    } else if (markerId == MarkerTypes::QUAL_TEXT_VALUE_MARKER_ID) {
        return Descriptor(slotName, tr("Qualifier text value marker"), tr("Qualifier text value marker."));
    } else if (markerId == MarkerTypes::QUAL_FLOAT_VALUE_MARKER_ID) {
        return Descriptor(slotName, tr("Qualifier float value marker"), tr("Qualifier float value marker."));
    } else if (markerId == MarkerTypes::TEXT_MARKER_ID) {
        return Descriptor(slotName, tr("Text marker"), tr("Text marker."));
    } else {
        assert(0);
        return Descriptor();
    }
}

const QString MarkerPorts::IN_MARKER_SEQ_PORT() {
    return "in-marked-seq";
}
const QString MarkerPorts::OUT_MARKER_SEQ_PORT() {
    return "out-marked-seq";
}

/* ***********************************************************************/
/* Marker
/* ***********************************************************************/
Marker::Marker(const QString &markerType, const QString &markerName)
: type(markerType), name(markerName)
{
    dataType = MarkerTypes::getDataTypeById(markerType);
}

void Marker::addValue(QString name, QString value) {
    values.insert(name, value);
}

QString Marker::getMarkingResult(const QVariant &object) {
    foreach(QString val, values.keys()) {
        QVariantList expr;
        bool res = MarkerUtils::stringToValue(dataType, val, expr);
        if (!res) {
            continue;
        }

        bool marked = false;
        switch (dataType) {
            case INTEGER:
                marked = getMarkerIntResult(object, expr);
                break;
            case FLOAT:
                marked = getMarkerFloatResult(object, expr);
                break;
            case BOOLEAN:
                // marked = getMarkerBooleanResult(object, expr);
                break;
            case STRING:
                marked = getMarkerStringResult(object, expr);
                break;
        }
        if (marked) {
            return values.value(val);
        }
    }

    return MarkerUtils::REST_OPERATION;
}

bool Marker::getMarkerIntResult(const QVariant &object, QVariantList &expr) {
    int obj = object.toInt();
    QString operation = expr.at(0).toString();

    if (MarkerUtils::LESS_OPERATION == operation) {
        int val = expr.at(1).toInt();
        if (obj <= val) {
            return true;
        }
    } else if (MarkerUtils::GREATER_OPERATION == operation) {
        int val = expr.at(1).toInt();
        if (obj >= val) {
            return true;
        }
    } else if (MarkerUtils::INTERVAL_OPERATION == operation) {
        int val1 = expr.at(1).toInt();
        int val2 = expr.at(2).toInt();

        if (obj >= val1 && obj <= val2) {
            return true;
        }
    }

    return false;
}

bool Marker::getMarkerFloatResult(const QVariant &object, QVariantList &expr) {
    float obj = object.toFloat();
    QString operation = expr.at(0).toString();

    if (MarkerUtils::LESS_OPERATION == operation) {
        float val = expr.at(1).toFloat();
        if (obj <= val) {
            return true;
        }
    } else if (MarkerUtils::GREATER_OPERATION == operation) {
        float val = expr.at(1).toFloat();
        if (obj >= val) {
            return true;
        }
    } else if (MarkerUtils::INTERVAL_OPERATION == operation) {
        float val1 = expr.at(1).toFloat();
        float val2 = expr.at(2).toFloat();

        if (obj >= val1 && obj <= val2) {
            return true;
        }
    }

    return false;
}

bool Marker::getMarkerStringResult(const QVariant &object, QVariantList &expr) {
    QString obj = object.toString();
    QString operation = expr.at(0).toString();
    QString val = expr.at(1).toString();

    if (MarkerUtils::STARTS_OPERATION == operation) {
        return obj.startsWith(val);
    } else if (MarkerUtils::ENDS_OPERATION == operation) {
        return obj.endsWith(val);
    } else if (MarkerUtils::CONTAINS_OPERATION == operation) {
        return obj.contains(val);
    } else if (MarkerUtils::REGEXP_OPERATION == operation) {
        QRegExp rx(val);

        return rx.exactMatch(obj);
    }

    return false;
}

const QString &Marker::getName() const {
    return name;
}

const QString &Marker::getType() const {
    return type;
}

const QMap<QString, QString> &Marker::getValues() const {
    return values;
}

const QString Marker::toString() const {
    QString res;

    foreach(QString key, values.keys()) {
        res += key+" : "+values.value(key)+"; ";
    }
    return res;
}

/* ***********************************************************************/
/* SequencerMarker
/* ***********************************************************************/
QString SequenceMarker::getMarkingResult(const QVariant &object) {
    DNASequence seq = qVariantValue<DNASequence>(object);

    if (MarkerTypes::SEQ_LENGTH_MARKER_ID == type) {
        return Marker::getMarkingResult(seq.length());
    } else {
        assert(0);
        return MarkerUtils::REST_OPERATION;
    }
}

MarkerGroup SequenceMarker::getGroup() {
    return SEQUENCE;
}

/* ***********************************************************************/
/* QualifierMarker
/* ***********************************************************************/
QString QualifierMarker::getMarkingResult(const QVariant &object) {
    QList<SharedAnnotationData> anns = QVariantUtils::var2ftl(object.toList());

    foreach (SharedAnnotationData ann, anns) {
        foreach (U2Qualifier qual, ann->qualifiers) {
            if (qual.name == qualName) {
                bool ok = false;
                QVariant value;
                if (MarkerTypes::QUAL_INT_VALUE_MARKER_ID == type) {
                    value = qVariantFromValue(qual.value.toInt(&ok));
                } else if (MarkerTypes::QUAL_FLOAT_VALUE_MARKER_ID == type) {
                    value = qVariantFromValue(qual.value.toFloat(&ok));
                } else if (MarkerTypes::QUAL_TEXT_VALUE_MARKER_ID == type) {
                    value = qVariantFromValue(qual.value);
                    ok = true;
                } else {
                    assert(0);
                    return MarkerUtils::REST_OPERATION;
                }
                assert(ok);
                return Marker::getMarkingResult(value);
            }
        }
    }

    return MarkerUtils::REST_OPERATION;
}

MarkerGroup QualifierMarker::getGroup() {
    return QUALIFIER;
}

const QString &QualifierMarker::getQualifierName() const {
    return qualName;
}

/* ***********************************************************************/
/* AnnotationMarker
/* ***********************************************************************/
QString AnnotationMarker::getMarkingResult(const QVariant &object) {
    QList<SharedAnnotationData> anns = QVariantUtils::var2ftl(object.toList());

    if (MarkerTypes::ANNOTATION_COUNT_MARKER_ID == type) {
        return Marker::getMarkingResult(qVariantFromValue(anns.size()));
    } else if (MarkerTypes::ANNOTATION_LENGTH_MARKER_ID == type) {
        return MarkerUtils::REST_OPERATION;
    } else {
        assert(0);
    }

    return MarkerUtils::REST_OPERATION;
}

MarkerGroup AnnotationMarker::getGroup() {
    return ANNOTATION;
}

const QString &AnnotationMarker::getAnnotationName() const {
    return annName;
}

/* ***********************************************************************/
/* TextMarker
/* ***********************************************************************/
QString TextMarker::getMarkingResult(const QVariant &object) {
    if (MarkerTypes::TEXT_MARKER_ID == type) {
        return Marker::getMarkingResult(object);
    } else {
        assert(0);
    }

    return MarkerUtils::REST_OPERATION;
}

MarkerGroup TextMarker::getGroup() {
    return TEXT;
}

} // U2
