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

#ifndef _U2_FEATURE_KEYS_H_
#define _U2_FEATURE_KEYS_H_


namespace U2 {

#define U2FeatureGeneName       "Gene"
#define U2FeatureExonName       "Exon"
#define U2FeatureCDSName        "CDS"

#define U2FeatureSuperDupName       "SD"
#define U2FeatureConservativeName   "CR"

#define U2FeatureKey_Name       "name"
#define U2FeatureKey_AltName    "alt_name"
#define U2FeatureKey_Note       "note"
#define U2FeatureKey_Group      "group"
#define U2FeatureKey_Accession  "accession"
#define U2FeatureKey_KnownType  "known"
#define U2FeatureKey_Disease    "disease"
    



//Since feature keys are used to store groups and operations, the most rare symbols combination is used
//to avoid adding qualifier with the same name
//Also it is short to reduce required space in the db

#define U2FeatureKeyGroup           "#~g"

#define U2FeatureKeyOperation       "#~o"
#define U2FeatureKeyOperationOrder  "#~oo"
#define U2FeatureKeyOperationJoin   "#~oj"

} //namespace

#endif //_U2_FEATURE_KEYS_H_
