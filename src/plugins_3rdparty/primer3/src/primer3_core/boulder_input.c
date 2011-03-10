/*
Copyright (c) 1996,1997,1998,1999,2000,2001,2004,2006,2007
Whitehead Institute for Biomedical Research, Steve Rozen
(http://jura.wi.mit.edu/rozen), and Helen Skaletsky
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
   * Neither the names of the copyright holders nor contributors may
be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <limits.h>
#include <stdlib.h>  /* strtod, strtol,... */
#include <ctype.h> /* toupper */
#include "boulder_input.h"

#define INIT_BUF_SIZE 1024
#define INIT_LIB_SIZE  500

/* 
 * Hack to support old SunOS headers.  (We do not try to declare _all_
 * undeclared functions; only those with non-int return types.)
 */
#ifndef __cplusplus
extern double strtod();
#endif

/* 
 * Read data from input stream until a "=" line occurs.  Assign parameter
 * values for primer picking and perform primary data control. Return 0 for
 * end of data and 1 otherwise.  If sa->error is not NULL the data is
 * erroneous and should not be processed. Echo the input lines to stdout.
 */
#define COMPARE(TAG) (!strncmp(s, TAG, tag_len) \
                      && ('=' == s[tag_len] || ' ' == s[tag_len]) \
                      && '\0' == TAG[tag_len])

#define COMPARE_AND_MALLOC(TAG,T)                  \
   if (COMPARE(TAG)) {                             \
       if (T) {                                    \
           pr_append_new_chunk(parse_err,          \
                               "Duplicate tag: "); \
           pr_append(parse_err, TAG);              \
       } else {                                    \
           T = pr_safe_malloc(datum_len + 1);      \
           strcpy(T, datum);                       \
       }                                           \
       continue;                                   \
   }

#define COMPARE_ALIGN_SCORE(TAG,T)                     \
   if (COMPARE(TAG)) {                                 \
       parse_align_score(TAG, datum, &(T), parse_err); \
       continue;                                       \
   }

#define COMPARE_FLOAT(TAG,T)                      \
   if (COMPARE(TAG)) {                            \
       parse_double(TAG, datum, &(T), parse_err); \
       continue;                                  \
   }

#define COMPARE_INT(TAG,T)                     \
   if (COMPARE(TAG)) {                         \
       parse_int(TAG, datum, &(T), parse_err); \
       continue;                               \
   }

#define COMPARE_INTERVAL_LIST(TAG, SIZE, LIST)                   \
   if (COMPARE(TAG)) {                                           \
       parse_interval_list(TAG, datum, &SIZE, LIST, parse_err);  \
       continue;                                                 \
   }
#undef COMPARE
#undef COMPARE_AND_MALLOC
#undef COMPARE_INT
#undef COMPARE_FLOAT
#undef COMPARE_INTERVAL_LIST

void
adjust_base_index_interval_list(intervals, num, first_index)
    interval_array_t intervals;
    int num, first_index;
{
    int i;
    for (i = 0; i < num; i++) intervals[i][0] -= first_index;
}

/*
 * Read a line of any length from stdin.  Return NULL on end of file,
 * otherwise return a pointer to static storage containing the line.  Any
 * trailing newline is stripped off.
 */
char*
read_line(file)
FILE *file;
{
//    static size_t ssz;
//    static char *s = NULL;
    size_t ssz;
    char *s = NULL;

    size_t remaining_size;
    char *p, *n;

    if (NULL == s) {
        ssz = INIT_BUF_SIZE;
        s = pr_safe_malloc(ssz);
    }
    p = s;
    remaining_size = ssz;
    while (1) {
        if (fgets(p, remaining_size, file) == NULL) /* End of file. */
            return p == s ? NULL : s;

        if ((n = strchr(p, '\n')) != NULL) {
            *n = '\0';
            return s;
        }

        /* We did not get the whole line. */

        /*
         * The following assertion is a bit of hack, a at least for 32-bit
         * machines, because we will usually run out of address space first.
         * Really we should treat an over-long line as an input error, but
         * since an over-long line is unlikely and we do want to provide some
         * protection....
         */
        PR_ASSERT(ssz <= INT_MAX);
        if (ssz >= INT_MAX / 2)
            ssz = INT_MAX;
        else {
            ssz *= 2;
        }
        s = pr_safe_realloc(s, ssz);
        p = strchr(s, '\0');
        remaining_size = ssz - (p - s);
    }
}

void
tag_syntax_error(tag_name, datum, err)
    const char *tag_name, *datum;
    pr_append_str *err;
{
    pr_append_new_chunk(err, "Illegal ");
    pr_append(err, tag_name);
    pr_append(err, " value: ");
    pr_append(err, datum);
}

void
parse_align_score(tag_name, datum, out, err)
    const char *datum, *tag_name;
    short *out;
    pr_append_str *err;
{
    double d;

    parse_double(tag_name, datum, &d, err);
    d *= PR_ALIGN_SCORE_PRECISION;
    if (d > SHRT_MAX) {
	pr_append_new_chunk(err, "Value too large at tag ");
	pr_append(err, tag_name);
    } else {
	/* Should we be rounding here? */
	*out = (short)d;
    }
}    

void
parse_double(tag_name, datum, out, err)
    const char *datum, *tag_name;
    double *out;
    pr_append_str *err;
{
    char *nptr;
    *out = strtod(datum, &nptr);
    if (nptr == datum) {
	/* Empty string or complete junk. */
	tag_syntax_error(tag_name, datum, err);
	*out = 0.0;
	return;
    }
    /* Look for trailing junk. */
    while (*nptr != '\n' && *nptr != '\0') {
	if (*nptr != ' ' && *nptr != '\t') {
	    tag_syntax_error(tag_name, datum, err);
	    break;
	}
	nptr++;
    }
}

void
parse_int(tag_name, datum, out, err)
    const char *datum, *tag_name;
    int *out;
    pr_append_str *err;
{
    char *nptr;
    long tlong;
    tlong = strtol(datum, &nptr, 10);
    if (tlong > INT_MAX || tlong < INT_MIN) {
	tag_syntax_error(tag_name, datum, err);
	pr_append(err, " (value too large or too small)");
	return;
    }
    *out = tlong;
    if (nptr == datum) {
	/* Empty string or complete junk. */
	tag_syntax_error(tag_name, datum, err);
	return;
    }
    /* Look for trailing junk. */
    while (*nptr != '\n' && *nptr != '\0') {
	if (*nptr != ' ' && *nptr != '\t') {
	    tag_syntax_error(tag_name, datum, err);
	    break;
	}
	nptr++;
    }
}

/* 
 * For correct input, return a pointer to the first non-tab, non-space
 * character after the second integer, and place the integers in out1 and
 * out2.  On incorrect input, return NULL;
 */
const char *
parse_int_pair(tag_name, datum, sep, out1, out2, err)
    const char    *tag_name, *datum;
    char          sep;          /* The separator, e.g. ',' or '-'. */
    int           *out1, *out2; /* The 2 integers. */
    pr_append_str *err;         /* Error messages. */
{
    char *nptr, *tmp;
    long tlong;
    tlong = strtol(datum, &nptr, 10);
    if (tlong > INT_MAX || tlong < INT_MIN) {
	tag_syntax_error(tag_name, datum, err);
	pr_append(err, " (value too large or too small)");
	return NULL;
    }
    *out1 = tlong;
    if (nptr == datum) {
	tag_syntax_error(tag_name, datum, err);
	return NULL;
    }
    while (' ' == *nptr || '\t' == *nptr) nptr++;
    if (sep != *nptr) {
	tag_syntax_error(tag_name, datum, err);
	return NULL;
    }
    nptr++; /* Advance past separator. */
    while (' ' == *nptr || '\t' == *nptr) nptr++;
    tmp = nptr;
    tlong = strtol(tmp, &nptr, 10);
    if (tlong > INT_MAX || tlong < INT_MIN) {
	tag_syntax_error(tag_name, datum, err);
	pr_append(err, " (value too large or too small)");
	return NULL;
    }
    *out2 = tlong;
    if (nptr == tmp) {
	tag_syntax_error(tag_name, datum, err);
	return NULL;
    }
    while (' ' == *nptr || '\t' == *nptr) nptr++;

    /* A hack to live with the old TARGET syntax. */
    if (',' == *nptr && !strcmp(tag_name, "TARGET")) {
	/* Skip the old-fashioned "description". */
	while(' ' != *nptr && '\t' != *nptr 
	      && '\0' != *nptr && '\n' != *nptr) nptr++;
	/* Advance to non-space, non-tab. */
	while (' ' == *nptr || '\t' == *nptr) nptr++;
    }
    return nptr;
}

void
parse_interval_list(tag_name, datum, count, interval_array, err)
    const char *tag_name;
    const char *datum;
    int *count;
    interval_array_t interval_array;
    pr_append_str *err;
{
    const char *p = datum;
    while (' ' == *p || '\t' == *p) p++;
    while (*p != '\0' && *p != '\n') {
	if (*count >= PR_MAX_INTERVAL_ARRAY) {
	    pr_append_new_chunk(err, "Too many elements for tag ");
	    pr_append(err, tag_name);
	    return;
	}
	p = parse_int_pair(tag_name, p, ',', 
			   &interval_array[*count][0],
			   &interval_array[*count][1],
			   err);
	if (NULL == p) return;
	(*count)++;
    }
}

void
parse_product_size(tag_name, in, pa, err)
    const char *tag_name;
    const char *in;
    primer_args *pa;
    pr_append_str *err;
{
    char *q = NULL;
    const char *s = in;
    const char *p;
    int i;
    /* 
     * Handle possible double quotes around the value.
     * (This handling is needed for backward compatibility with v2.)
     */
    if ('"' == *s)  {
      s++;
      in++;
      q = strchr(s, '"');
      if (NULL == q) {
	pr_append_new_chunk(err, tag_name);
	pr_append(err, " begins but does not end with a quote");
	return;
      }
      /* Ignore the " and everything after it. */
      *q = '\0';
    }
    p = in;
    while (' ' == *p || '\t' == *p) p++;
    i = 0;
    while (*p != '\0' && *p != '\n') {
	if (i >= PR_MAX_INTERVAL_ARRAY) {
	    pr_append_new_chunk(err, "Too many values for ");
	    pr_append(err, tag_name);
	    return;
	}
	p = parse_int_pair(tag_name, p, '-',
			   &pa->pr_min[i], &pa->pr_max[i], err);
	if (NULL == p) return;
	i++;
    }
    pa->num_intervals = i;
}

    /*
 * Reads any file in fasta format and fills in *lib.  Sets lib->error to a
 * non-empty string on error.
 */
void
read_seq_lib(lib, filename, errfrag)
    seq_lib *lib;
    const char *filename;
    const char *errfrag;
{
    char  *p;
    FILE *file;
    int i, m, k;
    size_t j, n;
    char buf[2];
    char offender = '\0', tmp;

    char *pointerToFree = NULL;

    PR_ASSERT(NULL != lib);
    PR_ASSERT(NULL != filename);

    free_seq_lib(lib);

    lib->repeat_file = pr_safe_malloc(strlen(filename) + 1);
    strcpy(lib->repeat_file, filename);

    if((file = fopen(lib->repeat_file,"r")) == NULL) {
        pr_append_new_chunk(&lib->error,
                            "Cannot open ");
        goto ERROR;
    }

    j = INIT_BUF_SIZE;
    n = INIT_LIB_SIZE;
    lib->names = pr_safe_malloc(INIT_LIB_SIZE*sizeof(*lib->names));
    lib->seqs  = pr_safe_malloc(INIT_LIB_SIZE*sizeof(*lib->seqs));
    lib->weight= pr_safe_malloc(INIT_LIB_SIZE*sizeof(*lib->weight));
    lib->seq_num = 0;

    i = -1;  m = 0; k = 0;
    while((p = read_line(file))) {
        pointerToFree = p;
        if(*p == '>'){
            i++;
            if(i >= n) {
                n += INIT_LIB_SIZE;
                lib->names = pr_safe_realloc(lib->names,n*sizeof(*lib->names));
                lib->seqs  = pr_safe_realloc(lib->seqs ,n*sizeof(*lib->seqs));
                lib->weight= pr_safe_realloc(lib->weight,
                                             n*sizeof(*lib->weight));
            }
            p++;
            lib->names[i] = pr_safe_malloc(strlen(p) + 1);
            strcpy(lib->names[i],p);
            lib->weight[i] = parse_seq_name(lib->names[i]);
            lib->seqs[i] = pr_safe_malloc(INIT_BUF_SIZE);
            lib->seqs[i][0] = '\0';
            lib->seq_num = i+1;
            if(lib->weight[i] < 0) {
                pr_append_new_chunk(&lib->error, "Illegal weight in ");
                goto ERROR;
            }
            j = INIT_BUF_SIZE;
            k = 0;
            if(i > 0) {
                /* We are actually testing the previous sequence. */
                if(strlen(lib->seqs[i-1]) == 0) {
                    pr_append_new_chunk(&lib->error, "Empty sequence in ");
                    goto ERROR;
                }
                tmp = upcase_and_check_char(lib->seqs[i-1]);
                m += tmp;
                if (tmp && '\0' == offender) offender = tmp;
            }
            p--;
        }
        else {
            if(i < 0){
                pr_append_new_chunk(&lib->error,
                                    "Missing id line (expected '>') in ");
                goto ERROR;
            } else {
                if(k+strlen(p) > j-2){
                    while(j-2 < k+ strlen(p))j += INIT_BUF_SIZE;
                    lib->seqs[i] = pr_safe_realloc(lib->seqs[i], j);

                }
                strcat(lib->seqs[i], p);
                k += strlen(p);
            }
        }
        free(pointerToFree);
        pointerToFree = NULL;
    }
    if(i < 0) {
        pr_append_new_chunk(&lib->error, "Empty ");
        goto ERROR;
    }
    else if(strlen(lib->seqs[i]) < 3) {
        pr_append_new_chunk(&lib->error, "Sequence length < 3 in ");
        goto ERROR;
    }
    tmp = upcase_and_check_char(lib->seqs[i]);
    m += tmp;
    if (tmp && '\0' == offender) offender = tmp;
    if (offender) {
        pr_append_new_chunk(&lib->warning,
                            "Unrecognized character (");
        buf[0] = offender;
        buf[1] = '\0';
        pr_append(&lib->warning, buf);
        pr_append(&lib->warning, ") in ");
        pr_append(&lib->warning, errfrag);
        pr_append(&lib->warning, " ");
        pr_append(&lib->warning, lib->repeat_file);
    }
    fclose(file);
    reverse_complement_seq_lib(lib);
    return;

 ERROR:
    free(pointerToFree);
    pointerToFree = NULL;

    pr_append(&lib->error, errfrag);
    pr_append(&lib->error, " ");
    pr_append(&lib->error, lib->repeat_file);
    if (file) fclose(file);
}

/* 
 * Free exogenous storage associated with a seq_lib (but not the seq_lib
 * itself).  Silently ignore NULL p.  Set *p to 0 bytes.
 */
void
free_seq_lib(p)
    seq_lib *p;
{
    int i;
    if (NULL == p) return;

    if ( NULL != p->repeat_file) free(p->repeat_file);
    if (NULL != p->seqs) { 
	for(i = 0; i < p->seq_num; i++)
	    if (NULL != p->seqs[i]) free(p->seqs[i]);
	free(p->seqs);
    }
    if (NULL != p->names) {
	for(i = 0; i < p->seq_num; i++)
	    if (NULL != p->names[i]) free(p->names[i]);
	free(p->names);
    }
    if (NULL != p->weight) free(p->weight);
    if (NULL != p->error.data) free(p->error.data);
    if (NULL != p->warning.data) free(p->warning.data);
    if (NULL != p->rev_compl_seqs) free(p->rev_compl_seqs);
    memset(p, 0, sizeof(*p));
}

/* 
 * Removes spaces and "end-of-line" characters
 * from the sequence, replaces all other
 * characters except A, T, G, C and IUB/IUPAC
 * codes with N.  Returns 0 if there were no such
 * replacements and the first non-ACGT IUB
 * character otherwise. 
 */
char
upcase_and_check_char(s)
    char *s;
{
    int i, j, n, m;

    j = 0; m = 0;
    n = strlen(s);
    for(i=0; i<n; i++){
      
	switch(s[i])
	{
	case 'a' : s[i-j] = 'A'; break;
	case 'g' : s[i-j] = 'G'; break;
	case 'c' : s[i-j] = 'C'; break;
	case 't' : s[i-j] = 'T'; break;
	case 'n' : s[i-j] = 'N'; break;
	case 'A' : s[i-j] = 'A'; break;
	case 'G' : s[i-j] = 'G'; break;
	case 'C' : s[i-j] = 'C'; break;
	case 'T' : s[i-j] = 'T'; break;
	case 'N' : s[i-j] = 'N'; break;

        case 'b' : case 'B': 
        case 'd' : case 'D':
        case 'h' : case 'H':
        case 'v' : case 'V':
        case 'r' : case 'R':
        case 'y' : case 'Y':
        case 'k' : case 'K':
        case 'm' : case 'M':
	case 's' : case 'S':
	case 'w' : case 'W':
	  s[i-j] = toupper(s[i]); break;

	case '\n': j++;          break;
	case ' ' : j++;          break;
	case '\t': j++;          break;
	case '\r': j++;          break;
	default  : if (!m) m = s[i]; s[i-j] = 'N'; 
	}
    }
    s[n-j] = '\0';
    return m;
}

double
parse_seq_name(s)
char *s;
{
    char *p, *q;
    double n;

    p = s;
    while( *p != '*' && *p != '\0' ) p++;
    if (*p == '\0' ) return 1;
    else {
	 p++;
	 n = strtod( p, &q );
	 if( q == p ) return -1;
    }
    if(n > PR_MAX_LIBRARY_WT) return -1;

    return n;
}

void
reverse_complement_seq_lib(lib)
seq_lib  *lib;
{
    int i, n, k;
    if((n = lib->seq_num) == 0) return;
    else {
	lib->names = pr_safe_realloc(lib->names, 2*n*sizeof(*lib->names));
	lib->seqs = pr_safe_realloc(lib->seqs, 2*n*sizeof(*lib->seqs));
	lib->weight = pr_safe_realloc(lib->weight, 2*n*sizeof(*lib->weight));
	lib->rev_compl_seqs = pr_safe_malloc(2*n*sizeof(*lib->seqs));

	lib->seq_num *= 2;
	for(i=n; i<lib->seq_num; i++){
	    k = strlen(lib->names[i-n]);
	    lib->names[i] = pr_safe_malloc(k + 9);
	    strcpy(lib->names[i], "reverse ");
	    strcat(lib->names[i], lib->names[i-n]);
	    lib->seqs[i] = pr_safe_malloc(strlen(lib->seqs[i-n]) + 1);
	    _pr_reverse_complement(lib->seqs[i-n], lib->seqs[i]);
	    lib->weight[i] = lib->weight[i-n];
	    lib->rev_compl_seqs[i-n] = lib->seqs[i];
	    lib->rev_compl_seqs[i] = lib->seqs[i-n];
       }
    }
    return;
}

int
parse_seq_quality(s, num)
   char *s;
   int **num;
{
   int k, i=0, *g;
   long t;
   char *p, *q;

   p = q = s;
   k = strlen(s);
   g = *num = pr_safe_malloc(sizeof(int)*k);
   while(*p == ' ' || *p == '\t'){
      p++;
      if(*p == '\0' || *p == '\n') return 0;
   }
   while(*q != '\0' && *q != '\n'){
      t = strtol(p, &q, 10);
      if(q == p) return i;
      p = q;
      *g = t;
      g++;
      i++;
   }
   return i;
}
