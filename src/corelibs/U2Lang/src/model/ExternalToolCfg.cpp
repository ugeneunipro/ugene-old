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

#include <U2Core/BaseDocumentFormats.h>

#include <U2Lang/BaseTypes.h>
#include <U2Lang/ExternalToolCfg.h>

namespace U2 {

DocumentFormatId DataConfig::StringValue = DocumentFormatId("string-value");
DocumentFormatId DataConfig::OutputFileUrl = DocumentFormatId("output-file-url");

bool DataConfig::isStringValue() const {
    return (BaseTypes::STRING_TYPE()->getId() == type) && (StringValue == format);
}

bool DataConfig::isFileUrl() const {
    return (OutputFileUrl == format);
}

bool DataConfig::isSequence() const {
    return (BaseTypes::DNA_SEQUENCE_TYPE()->getId() == type);
}

bool DataConfig::isAnnotations() const {
    return (BaseTypes::ANNOTATION_TABLE_TYPE()->getId() == type);
}

bool DataConfig::isAnnotatedSequence() const {
    return (SEQ_WITH_ANNS == type);
}

bool DataConfig::isAlignment() const {
    return (BaseTypes::MULTIPLE_ALIGNMENT_TYPE()->getId() == type);
}

bool DataConfig::isText() const {
    return (BaseTypes::STRING_TYPE()->getId() == type) && (BaseDocumentFormats::PLAIN_TEXT == format);
}

bool DataConfig::operator ==(const DataConfig &other) const {
    return attrName == other.attrName
        && type == other.type
        && format == other.format
        && description == other.description;
}

bool AttributeConfig::operator ==(const AttributeConfig &other) const {
    return attrName == other.attrName
        && type == other.type
        && description == other.description;
}

#define CHECK_EQ(expr1, expr2) \
    if (!(expr1 == expr2)) {\
    return false;\
    }

bool ExternalProcessConfig::operator ==(const ExternalProcessConfig &other) const {
    CHECK_EQ(inputs.size(), other.inputs.size());
    CHECK_EQ(outputs.size(), other.outputs.size());
    CHECK_EQ(attrs.size(), other.attrs.size());
    CHECK_EQ(name, other.name);
    CHECK_EQ(description, other.description);

    foreach (const DataConfig &in, inputs) {
        CHECK_EQ(other.inputs.contains(in), true);
    }
    foreach (const DataConfig &out, outputs) {
        CHECK_EQ(other.outputs.contains(out), true);
    }
    foreach (const AttributeConfig &at, attrs) {
        CHECK_EQ(other.attrs.contains(at), true);
    }

    return true;
}

} // U2
