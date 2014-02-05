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

#ifndef _U2_FILTERTABLEUTILS_H_
#define _U2_FILTERTABLEUTILS_H_

#include <U2Core/SNPTablesDbi.h>

#include <U2Core/Gene.h>

#include <U2Core/U2VariantDbi.h>
#include <U2Core/U2Variant.h>
#include <U2Core/U2Type.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SqlHelpers.h>
#include <U2Core/DNATranslation.h>

namespace U2 {

class SequenceQueryCache;

class U2CORE_EXPORT SNPTablesUtils{
public:
    
    enum ExcludeFromGeneQuery{
        ExcludeCDS,
        ExcludeSubfeatures,
        ExcludeNames,
        ExlcudeNonPromotersAround
    };

    static QList<Gene> findGenes(const QList<U2Feature> &features, U2FeatureDbi* dbi, U2OpStatus &opStatus);

    static QList<Gene> findGenes(const U2DataId& seqId, const U2Region &region, U2FeatureDbi* dbi, U2OpStatus &opStatus, const QList<int>& excludeList = QList<int>());

    static QList<Gene> findRegulatedGenes(const U2DataId& seqId, const U2Region &region, U2FeatureDbi* dbi, U2OpStatus &opStatus, const QList<int>& excludeList = QList<int>());




    static QList<Gene> findGenesAround(const U2DataId& seqId, const U2Region &region, U2FeatureDbi* dbi, U2OpStatus &opStatus, const QList<int>& excludeList = QList<int>());

    static Gene findGenesStep(const U2Feature &parentFeature, U2FeatureDbi* dbi, U2OpStatus &opStatus, const QList<int>& excludeList = QList<int>());

    //deprecated
    static void calcDamageEffectForTrack(const U2VariantTrack& track, U2Dbi* dbiDatabase, U2Dbi* dbiSession, DNATranslation* complTransl, DNATranslation* aaTransl, U2OpStatus& os);

    //deprecated
    static QList<DamageEffect> getDamageEffectForVariant(const U2Variant& var, const U2DataId& seqId, U2Dbi* dbiDatabase, U2Dbi* dbiSession, DNATranslation* complTransl, DNATranslation* aaTransl, U2OpStatus& os);

    static float damageEffectVal(const U2Variant& var, const U2DataId& seqId, const Gene& gene, U2Dbi* dbiDatabase, DNATranslation* complTransl, DNATranslation* aaTransl,  U2OpStatus& os, SequenceQueryCache* seqCache = NULL);


};
    
} //namespace

#endif
