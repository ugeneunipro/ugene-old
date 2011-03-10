/*****************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Copyright (C) 1992-2003 Washington University School of Medicine
* All Rights Reserved
* 
*     This source code is distributed under the terms of the
*     GNU General Public License. See the files COPYING and LICENSE
*     for details.
*****************************************************************/

/* sqerror.c
* 
* error handling for the squid library
* CVS $Id: sqerror.c,v 1.6 2003/05/26 16:21:50 eddy Exp $
*/

#include "funcs.h"
#include <stdarg.h>

int squid_errno;        /* a global errno equivalent */


/* Function: Die()
* 
* Purpose:  Print an error message and die. The arguments
*           are formatted exactly like arguments to printf().
*           
* Return:   None. Exits the program.
*/          
/* VARARGS0 */
void
Die(const char *format, ...)
{
    va_list  argp;

    /* format the error mesg */
    va_start(argp, format);
    HMMException e("");
    vsnprintf(e.error, 1024, format, argp);
    va_end(argp);

    /* exit  */
    throw e;
}



/* Function: Warn()
* 
* Purpose:  Print an error message and return. The arguments
*           are formatted exactly like arguments to printf().
*           
* Return:   (void)
*/          
/* VARARGS0 */
void
Warn(const char *format, ...)
{
    va_list  argp;
    /* format the error mesg */
    fprintf(stderr, "WARNING: ");
    va_start(argp, format);
    vfprintf(stderr, format, argp);
    va_end(argp);
    fprintf(stderr, "\n");
    fflush(stderr);
}


HMMException::HMMException(const char *err) {
    sre_strlcpy(error, err, 1024);
    error[1023]='\0';
}
