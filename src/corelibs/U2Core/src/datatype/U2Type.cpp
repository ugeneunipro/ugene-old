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

#include "U2Type.h"

namespace U2 {

const U2DataType U2Type::Unknown;

/** Object types */
const U2DataType U2Type::Sequence;
const U2DataType U2Type::Msa;
const U2DataType U2Type::PhyTree;
const U2DataType U2Type::Assembly;
const U2DataType U2Type::VariantTrack;
const U2DataType U2Type::VariantType;
const U2DataType U2Type::FilterTableType;
const U2DataType U2Type::KnownMutationsTrackType;
const U2DataType U2Type::AnnotationTable;
const U2DataType U2Type::RawData;

const U2DataType U2Type::CrossDatabaseReference;

/** SCO (non-object, non-root) types */
const U2DataType U2Type::Annotation;
const U2DataType U2Type::AnnotationGroup;

/**  Assembly read */
const U2DataType U2Type::AssemblyRead;

/** Sequence feature */
const U2DataType U2Type::Feature;

const U2DataType U2Type::UdrRecord;

/**  Attribute types */
const U2DataType U2Type::AttributeInteger;
const U2DataType U2Type::AttributeReal;
const U2DataType U2Type::AttributeString;
const U2DataType U2Type::AttributeByteArray;

} // U2
