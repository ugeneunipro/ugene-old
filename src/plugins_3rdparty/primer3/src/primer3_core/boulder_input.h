/*
Copyright (c) 1996,1997,1998,1999,2000,2001,2004,2006
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

#ifndef BOULDER_INPUT_H
#define BOULDER_INPUT_H 1
#include "primer3.h"

typedef struct program_args {
    char format_output;
    char twox_compat;
    char strict_tags;
} program_args;

int read_record(const program_args *, primer_args *, /*primer_args *,*/
		seq_args *);

#ifdef __cplusplus
extern "C" { /* } */
#endif
/* functions. */
void   read_seq_lib(seq_lib *lib, const char *filename, const char *errfrag);
void   free_seq_lib( seq_lib *);
void   adjust_base_index_interval_list(interval_array_t, int, int);
void   parse_align_score(const char *, const char *, short *,
				pr_append_str *);
void   parse_double(const char *, const char *, double *,
			   pr_append_str *);
void   parse_int(const char *, const char *, int *, pr_append_str *);
const char *parse_int_pair(const char *, const char *, char, int *, int *,
			    pr_append_str *);
void   parse_interval_list(const char *, const char *, int*,
				  interval_array_t, pr_append_str *);
void   parse_product_size(const char *, const char *, primer_args *,
				 pr_append_str *);
void   tag_syntax_error(const char *, const char *,  pr_append_str *);
void   read_seq_lib(seq_lib *, const char *, const char *);
char   upcase_and_check_char(char *);
char*  read_line(FILE *);
double parse_seq_name(char *);
void   reverse_complement_seq_lib(seq_lib *);
int    parse_seq_quality(char *, int **);
#ifdef __cplusplus
    /* { */ }
#endif

#endif



