/* Pushdown stacks for integers, pointers, and characters.
*
* Contents:
*   1. The <ESL_STACK> object.
*   2. Other functions in the API.
*   3. Shuffling stacks.      [eslAUGMENT_RANDOM]
*   7. Copyright and license.
*
* Augmentations:
*   eslAUGMENT_RANDOM  : adds function for shuffling a stack. 
* 
* SRE 1 March 2000 [Seattle]
* Incorp into Easel SRE, Sun Dec 26 07:42:12 2004 [Zaragoza]
* SVN $Id: esl_stack.c 249 2008-04-24 19:19:50Z eddys $
*/ 
#include <hmmer3/easel/esl_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hmmer3/easel/easel.h>
#include "esl_stack.h"
#ifdef eslAUGMENT_RANDOM
#include <hmmer3/easel/esl_random.h>
#endif

/*****************************************************************
*# 1. The <ESL_STACK> object.
*****************************************************************/

/* Function:  esl_stack_ICreate()
* Synopsis:  Create an integer stack.
* Incept:    SRE, Sun Dec 26 09:11:50 2004 [Zaragoza]
*
* Purpose:   Creates an integer stack.
*
* Returns:   a pointer to the new stack.
*
* Throws:    <NULL> on an allocation failure.
*/
ESL_STACK *
esl_stack_ICreate(void)
{
    int status;
    ESL_STACK *ns = NULL;

    ESL_ALLOC_WITH_TYPE(ns, ESL_STACK*, sizeof(ESL_STACK));
    ns->nalloc   = ESL_STACK_INITALLOC;
    ns->pdata    = NULL;
    ns->cdata    = NULL;
    ESL_ALLOC_WITH_TYPE(ns->idata, int*, sizeof(int) * ns->nalloc);
    ns->n        = 0;
    return ns;

ERROR:
    esl_stack_Destroy(ns);
    return NULL;
}

/* Function:  esl_stack_CCreate()
* Synopsis:  Create a character stack.
* Incept:    SRE, Sun Dec 26 09:15:35 2004 [Zaragoza]
*
* Purpose:   Creates a character stack.
*
* Returns:   a pointer to the new stack.
*
* Throws:    <NULL> on an allocation failure.
*/
ESL_STACK *
esl_stack_CCreate(void)
{
    int status;
    ESL_STACK *cs = NULL;

    ESL_ALLOC_WITH_TYPE(cs, ESL_STACK*, sizeof(ESL_STACK));
    cs->nalloc   = ESL_STACK_INITALLOC;
    cs->idata    = NULL;
    cs->pdata    = NULL;
    ESL_ALLOC_WITH_TYPE(cs->cdata, char*, sizeof(char) * cs->nalloc);
    cs->n        = 0;
    return cs;

ERROR:
    esl_stack_Destroy(cs);
    return NULL;
}

/* Function:  esl_stack_PCreate()
* Synopsis:  Create a pointer stack.
* Incept:    SRE, Sun Dec 26 09:16:07 2004 [Zaragoza]
*
* Purpose:   Creates a pointer stack.
*
* Returns:   a pointer to the new stack.
*
* Throws:    <NULL> on an allocation failure.
*/
ESL_STACK *
esl_stack_PCreate(void)
{
    int status;
    ESL_STACK *ps = NULL;

    ESL_ALLOC_WITH_TYPE(ps, ESL_STACK*, sizeof(ESL_STACK));
    ps->nalloc   = ESL_STACK_INITALLOC;
    ps->idata    = NULL;
    ps->cdata    = NULL;
    ESL_ALLOC_WITH_TYPE(ps->pdata, void**, sizeof(void *) * ps->nalloc);
    ps->n        = 0;
    return ps;

ERROR:
    esl_stack_Destroy(ps);
    return NULL;
}

/* Function:  esl_stack_Reuse()
* Synopsis:  Reuse a stack.
* Incept:    SRE, Tue Dec 28 04:21:36 2004 [Zaragoza]
*
* Purpose:   Empties stack <s> so it can be reused without
*            creating a new one. The stack <s>
*            can be of any data type; it retains its original
*            type.
*
* Returns:   <eslOK>
*/
int
esl_stack_Reuse(ESL_STACK *s)
{
    s->n = 0;	/* it's that simple in this implementation */
    return eslOK;
}

/* Function:  esl_stack_Destroy()
* Synopsis:  Free a stack.
* Incept:    SRE, Sun Dec 26 09:16:24 2004 [Zaragoza]
*
* Purpose:   Destroys a created stack <s>, of any data type.
*/
void
esl_stack_Destroy(ESL_STACK *s)
{
    if (s->idata != NULL) free(s->idata);
    if (s->cdata != NULL) free(s->cdata);
    if (s->pdata != NULL) free(s->pdata);
    free(s);
}


/*****************************************************************
*# 2. Other functions in the API.
*****************************************************************/

/* Function:  esl_stack_IPush()
* Synopsis:  Push an integer onto a stack.
* Incept:    SRE, Sun Dec 26 09:17:17 2004 [Zaragoza]
*
* Purpose:   Push an integer <x> onto an integer stack <ns>.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on reallocation failure.
*/
int
esl_stack_IPush(ESL_STACK *ns, int x)
{
    int  status;
    int *ptr;

    if (ns->n == ns->nalloc) {
        ESL_RALLOC_WITH_TYPE(ns->idata, int*, ptr, sizeof(int) * ns->nalloc * 2);
        ns->nalloc += ns->nalloc;	/* reallocate by doubling */
    }
    ns->idata[ns->n] = x;
    ns->n++;
    return eslOK;

ERROR:
    return status;
}

/* Function:  esl_stack_CPush()
* Synopsis:  Push a char onto a stack.
* Incept:    SRE, Sun Dec 26 09:18:24 2004 [Zaragoza]
*
* Purpose:   Push a character <c> onto a character stack <cs>.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on reallocation failure.
*/
int
esl_stack_CPush(ESL_STACK *cs, char c)
{
    int  status;
    char *ptr;

    if (cs->n == cs->nalloc) {
        ESL_RALLOC_WITH_TYPE(cs->cdata, char*, ptr, sizeof(char) * cs->nalloc * 2);
        cs->nalloc += cs->nalloc;	/* reallocate by doubling */
    }
    cs->cdata[cs->n] = c;
    cs->n++;
    return eslOK;

ERROR:
    return status;
}

/* Function:  esl_stack_PPush()
* Synopsis:  Push a pointer onto a stack.
* Incept:    SRE, Sun Dec 26 09:18:49 2004 [Zaragoza]
*
* Purpose:   Push a pointer <p> onto a pointer stack <ps>.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on reallocation failure.
*/
int
esl_stack_PPush(ESL_STACK *ps, void *p)
{
    int status;
    void *ptr;

    if (ps->n == ps->nalloc) {
        ESL_RALLOC_WITH_TYPE(ps->pdata, void**, ptr, sizeof(void *) * ps->nalloc * 2);
        ps->nalloc += ps->nalloc;	/* reallocate by doubling */
    }
    ps->pdata[ps->n] = p;
    ps->n++;
    return eslOK;

ERROR:
    return status;
}

/* Function:  esl_stack_IPop()
* Synopsis:  Pop an integer off a stack.
* Incept:    SRE, Sun Dec 26 09:19:12 2004 [Zaragoza]
*
* Purpose:   Pops an integer off the integer stack <ns>, and returns
*            it through <ret_x>.
*
* Returns:   <eslOK> on success. <eslEOD> if stack is empty.
*/
int
esl_stack_IPop(ESL_STACK *ns, int *ret_x)
{
    if (ns->n == 0) {*ret_x = 0; return eslEOD;}
    ns->n--;
    *ret_x = ns->idata[ns->n];
    return eslOK;
}

/* Function:  esl_stack_CPop()
* Synopsis:  Pop a char off a stack.
* Incept:    SRE, Sun Dec 26 09:21:27 2004 [Zaragoza]
*
* Purpose:   Pops a character off the character stack <cs>, and returns
*            it through <ret_c>.
*
* Returns:   <eslOK> on success. <eslEOD> if stack is empty.
*/
int
esl_stack_CPop(ESL_STACK *cs, char *ret_c)
{
    if (cs->n == 0) {*ret_c = 0; return eslEOD;}
    cs->n--;
    *ret_c = cs->cdata[cs->n];
    return eslOK;
}

/* Function:  esl_stack_PPop()
* Synopsis:  Pop a pointer off a stack.
* Incept:    SRE, Sun Dec 26 09:21:56 2004 [Zaragoza]
*
* Purpose:   Pops a pointer off the pointer stack <ps>, and returns
*            it through <ret_p>.
*
* Returns:   <eslOK> on success. <eslEOD> if stack is empty.
*/
int
esl_stack_PPop(ESL_STACK *ps, void **ret_p)
{
    if (ps->n == 0) {*ret_p = 0; return eslEOD;}
    ps->n--;
    *ret_p = ps->pdata[ps->n];
    return eslOK;
}

/* Function:  esl_stack_ObjectCount()
* Synopsis:  Return the number of objects in a stack.
* Incept:    SRE, Sun Dec 26 09:22:41 2004 [Zaragoza]
*
* Purpose:   Returns the number of data objects stored in the
*            stack <s>. The stack may be of any datatype.
*/
int 
esl_stack_ObjectCount(ESL_STACK *s)
{
    return s->n;
}

/* Function:  esl_stack_Convert2String()
* Synopsis:  Convert a char stack to a string.
* Incept:    SRE, Sun Dec 26 09:23:36 2004 [Zaragoza]
*
* Purpose:   Converts a character stack <cs> to a NUL-terminated
*            string, and returns a pointer to the string. The
*            characters in the string are in the same order they
*            were pushed onto the stack.  The stack is destroyed by
*            this operation, as if <esl_stack_Destroy()> had been
*            called on it. The caller becomes responsible for
*            free'ing the returned string.
*
* Returns:   Pointer to the string; caller must <free()> this.
*
* Throws:    NULL if a reallocation fails.
*/
char *
esl_stack_Convert2String(ESL_STACK *cs)
{
    char *s;

    if (esl_stack_CPush(cs, '\0') != eslOK)
    { free(cs->cdata); free(cs); return NULL; } /* nul-terminate the data or self-destruct */
    s = cs->cdata;		           /* data is already just a string - just return ptr to it */
    free(cs);			           /* free the stack around it. */
    return s;
}

/* Function:  esl_stack_DiscardTopN()
* Synopsis:  Discard the top elements on a stack.
* Incept:    SRE, Tue Dec 28 04:33:06 2004 [St. Louis]
*
* Purpose:   Throw away the top <n> elements on stack <s>.
*            Equivalent to <n> calls to a <Pop()> function.
*            If <n> equals or exceeds the number of elements 
*            currently in the stack, the stack is emptied
*            as if <esl_stack_Reuse()> had been called.
*
* Returns:   <eslOK> on success.
*/
int
esl_stack_DiscardTopN(ESL_STACK *s, int n)
{
    if (n <= s->n) s->n -= n;
    else           s->n = 0;
    return eslOK;
}

/*****************************************************************
*# 3. Shuffling stacks [with <eslAUGMENT_RANDOM>]
*****************************************************************/
#ifdef eslAUGMENT_RANDOM

/* Function:  esl_stack_Shuffle()
* Synopsis:  Randomly shuffle the elements in a stack.
* Incept:    SRE, Mon Mar 31 11:01:06 2008 [Janelia]
*
* Purpose:   Randomly shuffle the elements in stack <s>, using
*            random numbers from generator <r>.
*
* Returns:   <eslOK> on success, and the stack is randomly 
*            shuffled.
*/
int
esl_stack_Shuffle(ESL_RANDOMNESS *r, ESL_STACK *s)
{
    int   n = s->n;
    int   w;

    while (n > 1) {
        w = esl_rnd_Roll(r, n);	/* shuffling algorithm: swap last elem with w, decrement n. */
        if      (s->idata != NULL)  ESL_SWAP(s->idata[w], s->idata[n-1], int);
        else if (s->cdata != NULL)  ESL_SWAP(s->cdata[w], s->cdata[n-1], char);
        else if (s->pdata != NULL)  ESL_SWAP(s->pdata[w], s->pdata[n-1], void *);
        n--;
    }
    return eslOK;
}
#endif /*eslAUGMENT_RANDOM*/

/*****************************************************************
* Easel - a library of C functions for biological sequence analysis
* Version h3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* Easel is distributed under the Janelia Farm Software License, a BSD
* license. See the LICENSE file for more details.
*****************************************************************/
