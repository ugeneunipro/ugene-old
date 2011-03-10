/* General routines used throughout HMMER.
*
* SRE, Fri Jan 12 13:19:38 2007 [Janelia] [Franz Ferdinand, eponymous]
* SVN $Id: hmmer.c 2821 2009-06-05 11:13:23Z eddys $
*/

#include <hmmer3/p7_config.h>

#include <math.h>
#include <float.h>
#include <assert.h>

#include <hmmer3/easel/easel.h>
#include "hmmer.h"


/*****************************************************************
* 1. Miscellaneous functions for H3
*****************************************************************/

// ! here were a banner function. we don't need it !


/* Function:  p7_AminoFrequencies()
* Incept:    SRE, Fri Jan 12 13:46:41 2007 [Janelia]
*
* Purpose:   Fills a vector <f> with amino acid background frequencies,
*            in [A..Y] alphabetic order, same order that Easel digital
*            alphabet uses. Caller must provide <f> allocated for at
*            least 20 floats.
*            
*            These were updated 4 Sept 2007, from SwissProt 50.8,
*            (Oct 2006), counting over 85956127 (86.0M) residues.
*
* Returns:   <eslOK> on success.
*/
int
p7_AminoFrequencies(float *f)
{
    f[0] = 0.0787945;		/* A */
    f[1] = 0.0151600;		/* C */
    f[2] = 0.0535222;		/* D */
    f[3] = 0.0668298;		/* E */
    f[4] = 0.0397062;		/* F */
    f[5] = 0.0695071;		/* G */
    f[6] = 0.0229198;		/* H */
    f[7] = 0.0590092;		/* I */
    f[8] = 0.0594422;		/* K */
    f[9] = 0.0963728;		/* L */
    f[10]= 0.0237718;		/* M */
    f[11]= 0.0414386;		/* N */
    f[12]= 0.0482904;		/* P */
    f[13]= 0.0395639;		/* Q */
    f[14]= 0.0540978;		/* R */
    f[15]= 0.0683364;		/* S */
    f[16]= 0.0540687;		/* T */
    f[17]= 0.0673417;		/* V */
    f[18]= 0.0114135;		/* W */
    f[19]= 0.0304133;		/* Y */
    return eslOK;
}

// ugene's funvtions for working with task settings
// TODO: remove to another source file?

void setDefaultUHMM3SearchSettings( UHMM3SearchSettings* it ) {
    assert( NULL != it );
    it->e                 = 10.0;
    it->t                 = OPTION_NOT_SET;
    it->z                 = OPTION_NOT_SET;
    it->domE              = 10.0;
    it->domT              = OPTION_NOT_SET;
    it->domZ              = OPTION_NOT_SET;
    it->useBitCutoffs     = -1;
    it->incE              = 0.01;
    it->incT              = OPTION_NOT_SET;
    it->incDomE           = 0.01;
    it->incDomT           = OPTION_NOT_SET;
    it->f1                = 0.02;
    it->f2                = 1e-3;
    it->f3                = 1e-5;
    it->doMax             = FALSE;
    it->noBiasFilter      = FALSE;
    it->noNull2           = FALSE;
    it->seed              = 42;
}

bool checkUHMM3SearchSettings( UHMM3SearchSettings * it ) {
    assert( NULL != it );
    
    if( !( 0 < it->e ) ) { return false; }
    if( !( 0 < it->t || OPTION_NOT_SET == it->t ) ) { return false; }
    if( !( 0 < it->z || OPTION_NOT_SET == it->z ) ) { return false; }
    if( !( 0 < it->domE ) ) { return false; }
    if( !( 0 < it->domT || OPTION_NOT_SET == it->domT ) ) { return false; }
    if( !( 0 < it->domZ || OPTION_NOT_SET == it->domZ ) ) { return false; }
    if( !( 0 < it->incE ) ) { return false; }
    if( !( 0 < it->incT || OPTION_NOT_SET == it->incT ) ) { return false; }
    if( !( p7H_GA == it->useBitCutoffs || p7H_TC == it->useBitCutoffs || p7H_NC == it->useBitCutoffs
        || -1 == it->useBitCutoffs ) ) { return false; }
    if( !( 0 < it->incDomE ) ) { return false; }
    if( !( 0 < it->incDomT || OPTION_NOT_SET == it->incDomT ) ) { return false; }
    if( !( it->doMax == TRUE || it->doMax == FALSE ) ) { return false; }
    if( !( it->noBiasFilter == TRUE || it->noBiasFilter == FALSE ) ) { return false; }
    if( !( it->noNull2 == TRUE || it->noNull2 == FALSE ) ) { return false; }
    if( !( 0 <= it->seed ) ) { return false; }
    
    return true;
}

void setDefaultUHMM3BuildSettings( UHMM3BuildSettings * it ) {
    assert( NULL != it );
    it->archStrategy = p7_ARCH_FAST;
    it->wgtStrategy  = p7_WGT_PB;
    it->effnStrategy = p7_EFFN_ENTROPY;
    
    it->eset = -1.0;
    it->seed = 42;
    
    it->symfrac     = 0.5;
    it->fragtresh   = 0.5;
    it->wid         = 0.62;
    it->ere         = -1.0;
    it->esigma      = 45.0;
    it->eid         = 0.62;
    it->eml         = 200;
    it->emn         = 200;
    it->evl         = 200;
    it->evn         = 200;
    it->efl         = 100;
    it->efn         = 200;
    it->eft         = 0.04;
}

bool checkUHMM3BuildSettings( UHMM3BuildSettings * it ) {
    assert( NULL != it );

    if( it->archStrategy != p7_ARCH_FAST && it->archStrategy != p7_ARCH_HAND ) {
        return false;
    }
    if( it->wgtStrategy != p7_WGT_GSC && it->wgtStrategy != p7_WGT_BLOSUM && it->wgtStrategy != p7_WGT_PB &&
        it->wgtStrategy != p7_WGT_NONE && it->wgtStrategy != p7_WGT_GIVEN ) {
            return false;
    }
    if( it->effnStrategy != p7_EFFN_ENTROPY && it->effnStrategy != p7_EFFN_CLUST && 
        it->effnStrategy != p7_EFFN_NONE && it->effnStrategy != p7_EFFN_SET ) {
            return false;
    }

    if( !(0 <= it->symfrac && it->symfrac <= 1 ) ) { return false; }
    if( !(0 <= it->wid && it->wid <= 1 ) ) { return false; }
    if( !( 0 < it->eset ) ) { return false; }
    if( !( -1 == it->ere || 0 < it->ere ) ) { return false; }
    if( !( 0 <= it->fragtresh && it->fragtresh <= 1 ) ) { return false; }
    if( !( 0 < it->esigma ) ) { return false; }
    if( !(0 <= it->eid && it->eid <= 1 ) ) { return false; }
    if( !( 0 < it->eml ) ) { return false; }
    if( !( 0 < it->emn ) ) { return false; }
    if( !( 0 < it->evl ) ) { return false; }
    if( !( 0 < it->evn ) ) { return false; }
    if( !( 0 < it->efl ) ) { return false; }
    if( !( 0 < it->efn ) ) { return false; }
    if( !(0 < it->wid && it->wid < 1 ) ) { return false; }
    if( !( 0 < it->seed ) ) { return false; }

    return true;
}

/************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Version 3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* HMMER is distributed under the terms of the GNU General Public License
* (GPLv3). See the LICENSE file for details.
************************************************************/
