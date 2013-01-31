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

#ifndef _U2_VARIANT_DBI_H_
#define _U2_VARIANT_DBI_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Variant.h>

namespace U2 {

/**
    An interface to access to Variant tracks
*/
class U2VariantDbi : public U2ChildDbi {
protected:
    U2VariantDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}

public:
    
    /** Returns available VariantTracks */
    virtual U2DbiIterator<U2VariantTrack>* getVariantTracks(U2OpStatus& os) = 0;

    /** Returns available VariantTracks for given sequence id*/
    virtual U2DbiIterator<U2VariantTrack>* getVariantTracks(const U2DataId& seqId, U2OpStatus& os) = 0;


    /** Returns VariantTrack instance by the given id */
    virtual U2VariantTrack getVariantTrack(const U2DataId& id, U2OpStatus& os) = 0;
// 
//     /** 
//         Creates new VariantTrack instance and adds all Variants provided by iterator to it 
//         Requires U2DbiFeature_WriteVariants feature support
//     */
//     virtual void createVariantTrack(U2VariantTrack& track, U2DbiIterator<U2Variant>* it, const QString& folder, U2OpStatus& os) = 0;
// 
    /** 
        Adds all Variants provided by iterator to the track 
        Requires U2DbiFeature_WriteVariants feature support
    */
    virtual void addVariantsToTrack(const U2VariantTrack& track, U2DbiIterator<U2Variant>* it, U2OpStatus& os) = 0;
     /** 
        Creates new VariantTrack instance
        Requires U2DbiFeature_WriteVariants feature support
    */
    virtual void createVariantTrack(U2VariantTrack& track, const QString& folder, U2OpStatus& os) = 0;

    /** 
        Create Variant and add it to the track
        Requires U2DbiFeature_WriteVariants feature support
    */
    //virtual void addVariantToTrack(U2Variant& variant, const U2VariantTrack& track, U2OpStatus& os) = 0;
 
    /** 
        Updates VariantTrack instance 
        Requires U2DbiFeature_WriteVariants feature support
    */
    virtual void updateVariantTrack(const U2VariantTrack& track, U2OpStatus& os) = 0;

    /** Returns all Variants from the given sequence region 
    U2_REGION_MAX to get all variants*/
    virtual U2DbiIterator<U2Variant>* getVariants(const U2DataId& track, const U2Region& region, U2OpStatus& os) = 0;

    /** Returns a selection of variants of the track starting from the offset */
    virtual U2DbiIterator<U2Variant>* getVariantsRange(const U2DataId& track, int offset, int limit, U2OpStatus& os ) = 0;

    /** Return number of variants in track */
    virtual int getVariantCount(const U2DataId& track, U2OpStatus& os) = 0;

    /** Delete the variant track from the database */
    virtual void removeTrack(const U2DataId& track, U2OpStatus& os) = 0;
};


} //namespace

#endif
