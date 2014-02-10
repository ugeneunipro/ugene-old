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

/**
 * If you change something here do not forget to update an appropriate comment in the header file
 */

const U2DataType U2Type::Unknown                        = 0;

/** Object types */
const U2DataType U2Type::Sequence                       = 1;
const U2DataType U2Type::Msa                            = 2;
const U2DataType U2Type::PhyTree                        = 3;
const U2DataType U2Type::Assembly                       = 4;
const U2DataType U2Type::VariantTrack                   = 5;
const U2DataType U2Type::VariantType                    = 6;
const U2DataType U2Type::FilterTableType                = 7;
const U2DataType U2Type::KnownMutationsTrackType        = 8;
const U2DataType U2Type::AnnotationTable                = 10;
const U2DataType U2Type::RawData                        = 101;

const U2DataType U2Type::CrossDatabaseReference         = 999;

/** SCO (non-object, non-root) types */
const U2DataType U2Type::Annotation                     = 1000;
const U2DataType U2Type::AnnotationGroup                = 1001;

/**  Assembly read */
const U2DataType U2Type::AssemblyRead                   = 1100;

/** Sequence feature */
const U2DataType U2Type::Feature                        = 1300;

const U2DataType U2Type::UdrRecord                      = 1400;

/**  Attribute types */
const U2DataType U2Type::AttributeInteger               = 2001;
const U2DataType U2Type::AttributeReal                  = 2002;
const U2DataType U2Type::AttributeString                = 2003;
const U2DataType U2Type::AttributeByteArray             = 2004;

} // U2
