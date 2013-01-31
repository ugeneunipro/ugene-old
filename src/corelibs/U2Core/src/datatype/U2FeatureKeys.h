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

#ifndef _U2_FEATUREKEYS_H_
#define _U2_FEATUREKEYS_H_


namespace U2 {

//Since feature keys are used to store groups and operations, the most rare symbols combination is used
//to avoid adding qualifier with the same name
//Also it is short to reduce required space in the db

#define U2FeatureKeyGroup           "#~g"

#define U2FeatureKeyOperation       "#~o"
#define U2FeatureKeyOperationOrder  "#~oo"
#define U2FeatureKeyOperationJoin   "#~oj"

} //namespace

#endif //_U2_FEATUREKEYS_H_
