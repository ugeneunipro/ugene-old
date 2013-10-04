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


#ifndef _U2_FILTERTABLEBI_H_
#define _U2_FILTERTABLEBI_H_

#include <U2Core/DamageEffect.h>

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Variant.h>

#define DEFAULT_FILTER_NAME QString("Default")
#define DEFAULT_FILTERTABLE_NAME QString("FTABLE")

namespace U2 {
    
    /*Dbi for snp tables.
        Tables are included for objects: 
            DamageEffect
            FilterTable
            FilterTableNames
            */
class SNPTablesDbi : public U2ChildDbi{
protected:
    SNPTablesDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}
public:

    /* Creates damage effect instance */
    virtual void createDamageEffect(DamageEffect& effect, U2OpStatus& os) = 0;

    /* Creates damage effect index */
    virtual void createDamageEffectIndex(U2OpStatus& os) = 0;

    /*Interface to the table that marks not annotated variations*/
    virtual void markNotAnnotated(const U2DataId& variant, U2OpStatus& os) = 0;
    virtual void markAnnotated(const U2DataId& variant, U2OpStatus& os) = 0;
    //returns 1 if not annotated and 0 if annotated
    virtual int checkNotAnnotated(const U2DataId& variant, U2OpStatus& os) = 0;
    virtual void createAnnotationsMarkerIndex(U2OpStatus& os) = 0;

    /* Remove the damage effect instance from the database */
    virtual void removeDamageEffect(const DamageEffect& effect, U2OpStatus& os) = 0;

    /* Remove all damage effects for given variant from the database */
    virtual void removeAllDamageEffectForVariant(const U2Variant& variant, U2OpStatus& os) = 0;

    /* returns damage effect instances for given variant*/
    virtual U2DbiIterator<DamageEffect>* getDamageEffectsForVariant(const U2DataId& variant, U2OpStatus& os) = 0;

    /* Remove the regulatory effect instance from the database */
    virtual void removeRegulatoryEffect(const RegulatoryEffect& effect, U2OpStatus& os) = 0;

    /* Remove all damage effects for given variant from the database */
    virtual void removeAllRegulatoryEffectForVariant(const U2Variant& variant, U2OpStatus& os) = 0;

    /* returns damage effect instances for given variant*/
    virtual U2DbiIterator<RegulatoryEffect>* getRegulatoryEffectsForVariant(const U2DataId& variant, U2OpStatus& os) = 0;

    /* Creates FilterTableNames instance with given name*/
  //  virtual void createFilterTable (FilterTable& table, const QString& filterName, U2OpStatus& os) = 0;
    
    /* Remove FilterTableNames instance and temporary filter table connected to the id of the instance*/
  //  virtual void removeFilterTable (const FilterTable& table, U2OpStatus& os) = 0;

    /* Rename filter name in FilterTableNames */
    virtual void renameFilterTable (const U2DataId& fTable, const QString& newName, U2OpStatus& os) = 0;

    /* Create index for filter table with given ID */
    virtual void createIndexForFilterTable (const U2DataId& fTable, U2OpStatus& os) = 0;

    virtual void updateVariantFilterTable (const U2DataId& fTable, const U2DataId& varId, VariantTrackType tType, U2OpStatus& os) = 0;

    /* Insert variants of the specific track into the FilterTable*/
    virtual void addVariantsToTable (const U2DataId& fTable, const U2DataId& track, VariantTrackType tType, const QString& visualSeqName, U2DbiIterator<U2Variant>* it, U2OpStatus& os) = 0;

    /* Get FilterTable id by given name*/
  //  virtual FilterTable getFilterTableByName (const QString& filterName, U2OpStatus& os ) = 0;

    /* Get Variants from temporary table*/
//    virtual U2DbiIterator<FilterTableItem>* getVariantsRange(const U2DataId& fTable, VariantTrackType tType, int offset, int limit, int sortColumn, bool sortAscending, U2OpStatus& os) = 0;

    /* Get Variants count in the table*/
    virtual int getVariantCount(const U2DataId& fTable, VariantTrackType tType, U2OpStatus& os) = 0;

};
    
} //namespace

#endif
