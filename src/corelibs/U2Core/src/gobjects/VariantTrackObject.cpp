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

#include "VariantTrackObject.h"
#include "GObjectTypes.h"

#include <U2Core/AppContext.h>
#include <U2Core/U2VariantDbi.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/U2SafePoints.h>


namespace U2 {

//////////////////////////////////////////////////////////////////////////
//VariantTrackObject

VariantTrackObject::VariantTrackObject( const QString& objectName, const U2EntityRef& trackRef, const QVariantMap& hintsMap )
: GObject(GObjectTypes::VARIANT_TRACK, objectName+"_variations", hintsMap) {

    entityRef = trackRef;
}

VariantTrackObject::~VariantTrackObject(){

}

U2DbiIterator<U2Variant>* VariantTrackObject::getVariants( const U2Region& reg, U2OpStatus& os ) const {
    DbiConnection con;
    con.open(entityRef.dbiRef, os);
    CHECK_OP(os, NULL);

    U2VariantDbi* vdbi = con.dbi->getVariantDbi();
    SAFE_POINT(vdbi != NULL, "Varian DBI is NULL", NULL);
    
    return vdbi->getVariants(entityRef.entityId, reg, os);
}

U2VariantTrack VariantTrackObject::getVariantTrack(U2OpStatus &os) const {
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, U2VariantTrack());

    U2VariantDbi* vdbi = con.dbi->getVariantDbi();
    SAFE_POINT(vdbi != NULL, "Varian DBI is NULL", U2VariantTrack());

    return vdbi->getVariantTrack(entityRef.entityId, os);
}

GObject* VariantTrackObject::clone(const U2DbiRef &dstDbiRef, U2OpStatus &os) const{
    DbiConnection dstCon(dstDbiRef, true, os);
    CHECK_OP(os, NULL);

    U2VariantDbi *dstVDbi = dstCon.dbi->getVariantDbi();
    SAFE_POINT(dstVDbi != NULL, "NULL destination variant DBI", NULL);

    U2VariantTrack track = this->getVariantTrack(os);
    CHECK_OP(os, NULL);
    U2VariantTrack clonedTrack = track;
    dstVDbi->createVariantTrack(clonedTrack, "", os);
    CHECK_OP(os, NULL);

    QScopedPointer< U2DbiIterator<U2Variant> > varsIter(this->getVariants(U2_REGION_MAX, os));
    CHECK_OP(os, NULL);
    dstVDbi->addVariantsToTrack(clonedTrack, varsIter.data(), os);
    CHECK_OP(os, NULL);

    U2EntityRef clonedTrackRef(dstDbiRef, clonedTrack.id);
    VariantTrackObject *clonedObj = new VariantTrackObject(getGObjectName(), clonedTrackRef, getGHintsMap());
    return clonedObj;
}

void VariantTrackObject::addVariants( const QList<U2Variant>& variants, U2OpStatus& os ){
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );

    U2VariantDbi* vdbi = con.dbi->getVariantDbi();
    SAFE_POINT(vdbi != NULL, "Varian DBI is NULL", );

    U2VariantTrack track = vdbi->getVariantTrack(entityRef.entityId, os);
    CHECK_OP(os, );

    BufferedDbiIterator<U2Variant> bufIter(variants);
    vdbi->addVariantsToTrack(track, &bufIter, os);
}

}//namespace
