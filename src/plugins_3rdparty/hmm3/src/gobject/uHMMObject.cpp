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

GObject* UHMMObject::clone() const {
    P7_HMM* newHMM = p7_hmm_Clone( hmm );
    UHMMObject* cln = new UHMMObject( newHMM, getGObjectName() );
    cln->setIndexInfo( getIndexInfo() );
    
    return cln;
}

} // U2
