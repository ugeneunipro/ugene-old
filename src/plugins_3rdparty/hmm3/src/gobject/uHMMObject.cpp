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

#include <U2Core/GObjectTypes.h>

#include "uHMMObject.h"

namespace U2 {

const QString UHMMObject::OT_ID    = "OT_HMM";
const QString UHMMObject::OT_NAME  = "Hidden Markov model";
const QString UHMMObject::OT_PNAME = "Hidden Markov models";
const QString UHMMObject::OT_SIGN  = "hmm";
const QString UHMMObject::OT_ICON  = "";
const GObjectType UHMMObject::UHMM_OT = 
    GObjectTypes::registerTypeInfo( GObjectTypeInfo( OT_ID, OT_NAME, OT_PNAME, OT_SIGN, OT_ICON ) );


UHMMObject::UHMMObject( P7_HMM* ahmm, const QString& name ) : GObject( UHMM_OT, name ), hmm( ahmm ) {
}

UHMMObject::~UHMMObject() {
    if( NULL != hmm ) {
        p7_hmm_Destroy( hmm );
    }
}

const P7_HMM* UHMMObject::getHMM() const {
    return hmm;
}

P7_HMM* UHMMObject::takeHMM() {
    P7_HMM* ret = hmm;
    hmm = NULL;
    return ret;
}

GObject* UHMMObject::clone(const U2DbiRef&, U2OpStatus&) const {
    P7_HMM* newHMM = p7_hmm_Clone( hmm );
    UHMMObject* cln = new UHMMObject( newHMM, getGObjectName() );
    cln->setIndexInfo( getIndexInfo() );
    
    return cln;
}

} // U2
