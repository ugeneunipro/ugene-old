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

#include "BioStruct3DObject.h"
#include "U2Core/DNASequence.h"
#include "GObjectTypes.h"

namespace U2 { 

BioStruct3DObject::BioStruct3DObject(const BioStruct3D& struct3D, const QString& objectName, const QVariantMap& hintsMap)
: GObject(GObjectTypes::BIOSTRUCTURE_3D, objectName, hintsMap), bioStruct3D(struct3D)
{
}

GObject* BioStruct3DObject::clone(const U2DbiRef&, U2OpStatus&) const {
    BioStruct3DObject* cln = new BioStruct3DObject(bioStruct3D,getGObjectName(), getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}


} //namespace

