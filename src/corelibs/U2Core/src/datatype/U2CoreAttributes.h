/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CORE_ATTRIBUTES_H_
#define _U2_CORE_ATTRIBUTES_H_

#include <U2Core/global.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
// Reserved attribute names

class U2CORE_EXPORT U2BaseAttributeName {

    /** 
    Reserved to represent length: sequence or alignment 
    This attribute can be artificial and derived directly from structure values 
    If structure length is updated, system is responsible to update the attribute too
    */
    static const QString length;//          = "length";

    /** 
    Reserved to represent alphabet: sequence or alignment 
    This attribute can be artificial and derived directly from structure values 
    If structure alphabet is updated, system is responsible to update the attribute too
    */
    static const QString alphabet;//        = "alphabet";

    /** 
    Reserved to represent GC-content: sequence or alignment 
    This attribute can be artificial and derived directly from structure values 
    If structure gc-content is updated, system is responsible to update the attribute too
    */
    static const QString gc_content;//      = "GC-content";

    /** Structure modification date */
    static const QString update_time;//     = "update-time";

    /** Structure creation date */
    static const QString create_date; //    = "creation-time";

};

#endif
