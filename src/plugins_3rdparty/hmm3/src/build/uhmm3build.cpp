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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignmentInfo.h>

#include <hmmer3/easel/esl_msa.h>
#include <hmmer3/hmmer.h>
#include <hmmer3/p7_config.h>

#include <util/uhmm3Utilities.h>

#include "uhmm3build.h"

using namespace U2;

static void destoryAllIfYouCan( ESL_ALPHABET* abc, P7_BG* bg, P7_BUILDER* bld, ESL_MSA* msa, P7_HMM* hmm ) {
    if( NULL != hmm )   p7_hmm_Destroy( hmm );
    if( NULL != bld )   p7_builder_Destroy( bld );
    if( NULL != bg )    p7_bg_Destroy( bg );
    if( NULL != msa )   esl_msa_Destroy( msa );
    if( NULL != abc )   esl_alphabet_Destroy( abc );
}

namespace U2 {

/* if error occurs we return hmms that are already created, so caller should delete them anyway
   Note, that we do not destroy hmm->abc here, caller should delete it too.
   Don't worry, on exception we'll catch it here
 */
P7_HMM * UHMM3Build::build( const MAlignment & malignment, const UHMM3BuildSettings & settings ,TaskStateInfo & ti ) {
    ESL_ALPHABET*   abc = NULL;
    P7_BG*          bg  = NULL;
    P7_BUILDER*     bld = NULL;
    ESL_MSA*        msa = NULL;
    P7_HMM*         hmm = NULL;
    QByteArray      errStr;
    
    ti.progress = 0;
    try {
        int alType = UHMM3Utilities::convertAlphabetType( malignment.getAlphabet() );
        if( UHMM3Utilities::BAD_ALPHABET == alType ) {
            errStr = tr( "cannot_convert_alphabet" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        ESL_ALPHABET* abc = esl_alphabet_Create( alType );
        if( NULL == abc ) {
            errStr = tr( "no_memory: cannot_create_alphabet" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        
        P7_BG* bg = p7_bg_Create( abc );
        if( NULL == bg ) {
            errStr = tr( "no_memory: cannot_create_null_model" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        P7_BUILDER* bld = p7_builder_Create( &settings, abc );
        if( NULL == bld ) {
            errStr = tr( "no_memory: cannot_create_builder" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        
        ESL_MSA* msa = UHMM3Utilities::convertMSA( malignment );
        if( NULL == msa ) {
            errStr = tr( "no_memory: cannot_convert_msa" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        int ret = esl_msa_Digitize( abc, msa, NULL );
        if( eslOK != ret ) {
            errStr = tr( "no_memory: cannot_digitize_msa" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        ret = p7_Builder( bld, msa, bg, &hmm, NULL, NULL, NULL, NULL, ti );
        if ( eslOK != ret ) {
            if( eslCANCELED == ret ) {
                errStr = tr( HMMER3_CANCELED_ERROR ).toAscii();
            } else {
                errStr = tr( "builder_error_occurred" ).toAscii();
            }
            assert( !errStr.isEmpty() );
            throwUHMMER3Exception( errStr.data() );
        }
        
        destoryAllIfYouCan( abc, bg, bld, msa, NULL );
    } catch( const UHMMER3Exception& ex ) {
        ti.setError( ex.msg );
        destoryAllIfYouCan( abc, bg, bld, msa, hmm );
        return NULL;
    } catch(...) {
        ti.setError( tr( HMMER3_UNKNOWN_ERROR ) );
        destoryAllIfYouCan( abc, bg, bld, msa, hmm );
        return NULL;
    }
    
    return hmm;
}

} // GB2
