/*****************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Copyright (C) 1992-2003 Washington University School of Medicine
* All Rights Reserved
* 
*     This source code is distributed under the terms of the
*     GNU General Public License. See the files COPYING and LICENSE
*     for details.
*****************************************************************/

#ifndef CONFIGH_INCLUDED
#define CONFIGH_INCLUDED


/* RAMLIMIT determines the point at which we switch from fast,
 * full dynamic programming to slow, linear-memory divide and conquer
 * dynamic programming algorithms. It is the minimum amount of available
 * RAM on the systems the package will run on. It can be overridden
 * from the Makefile.
 * By default, we assume we have 64 Mb RAM available (per thread).
 */
#ifndef RAMLIMIT
#define RAMLIMIT 64
#endif


/*****************************************************************
 * The following section probably shouldn't be edited, unless
 * you really know what you're doing. It controls some fundamental
 * parameters in HMMER that occasionally get reconfigured in
 * experimental versions, or for variants of HMMER that work on
 * non-biological alphabets.
 *****************************************************************/

#define INTSCALE    1000.0      /* scaling constant for floats to integer scores   */
#define MAXABET     20          /* maximum size of alphabet (4 or 20)              */
#define MAXCODE     24          /* maximum degenerate alphabet size (17 or 24)     */
#define MAXDCHLET   200         /* maximum # Dirichlet components in mixture prior */
#define NINPUTS     4           /* number of inputs into structural prior          */
#define INFTY       987654321   /* infinity for purposes of integer DP cells       */
#define NXRAY       4           /* number of structural inputs                */
#define LOGSUM_TBL  20000       /* controls precision of ILogsum()            */
#define ALILENGTH   50      /* length of displayed alignment lines        */


/* Version info - set once for whole package in configure.ac
*/
#define PACKAGE_NAME "HMMER"
#define PACKAGE_VERSION "2.3.2"
#define PACKAGE_DATE "Oct 2003"
#define PACKAGE_COPYRIGHT "Copyright (C) 1992-2003 HHMI/Washington University School of Medicine"
#define PACKAGE_LICENSE "Freely distributed under the GNU General Public License (GPL)"

#endif /*CONFIGH_INCLUDED*/
