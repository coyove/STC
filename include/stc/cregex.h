/*
This is a Unix port of the Plan 9 regular expression library, by Rob Pike.

Copyright © 2021 Plan 9 Foundation
Copyright © 2022 Tyge Løvset, for additions made in 2022.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef CREGEX_H_INCLUDED
#define CREGEX_H_INCLUDED
/*
 * cregex.h
 * 
 * This is a extended version of regexp9, supporting UTF8 input, common 
 * shorthand character classes, ++.
 */
#include <stdbool.h>
#include <string.h>
#include "forward.h" // csview 

enum {
    CREG_DEFAULT = 0,
    /* compile-flags */
    CREG_C_DOTALL = 1<<0,    /* dot matches newline too */
    CREG_C_ICASE = 1<<1,     /* ignore case */
    /* match-flags */
    CREG_M_FULLMATCH = 1<<2, /* like start-, end-of-line anchors were in pattern: "^ ... $" */
    CREG_M_NEXT = 1<<3,      /* use end of previous match[0] as start of input */
    CREG_M_STARTEND = 1<<4,  /* use match[0] as start+end of input */
    /* replace-flags */
    CREG_R_STRIP = 1<<5,     /* only keep the matched strings, strip rest */
    /* limits */
    CREG_MAX_CLASSES = 16,
    CREG_MAX_CAPTURES = 32,
};

typedef enum {
    CREG_OK = 0,
    CREG_SUCCESS = 0, /* [deprecated] */
    CREG_NOMATCH = -1,
    CREG_MATCHERROR = -2,
    CREG_OUTOFMEMORY = -3,
    CREG_UNMATCHEDLEFTPARENTHESIS = -4,
    CREG_UNMATCHEDRIGHTPARENTHESIS = -5,
    CREG_TOOMANYSUBEXPRESSIONS = -6,
    CREG_TOOMANYCHARACTERCLASSES = -7,
    CREG_MALFORMEDCHARACTERCLASS = -8,
    CREG_MISSINGOPERAND = -9,
    CREG_UNKNOWNOPERATOR = -10,
    CREG_OPERANDSTACKOVERFLOW = -11,
    CREG_OPERATORSTACKOVERFLOW = -12,
    CREG_OPERATORSTACKUNDERFLOW = -13,
} cregex_result;

typedef struct {
    struct _Reprog* prog;
    int error;
} cregex;

typedef struct {
    const cregex* re;
    const char* input;
    csview match[CREG_MAX_CAPTURES];
} cregex_iter;

#define c_FORMATCH(it, Re, Input) \
    for (cregex_iter it = {Re, Input}; \
         cregex_find(it.re, it.input, it.match, CREG_M_NEXT) == CREG_OK; )

static inline
cregex cregex_init(void) {
    cregex re = {0};
    return re;
}

/* return CREG_OK, or negative error code on failure. */
int cregex_compile(cregex *self, const char* pattern, int cflags);

static inline
cregex cregex_from(const char* pattern, int cflags) {
    cregex re = {0};
    cregex_compile(&re, pattern, cflags);
    return re;
}

/* number of capture groups in a regex pattern, 0 if regex is invalid */
unsigned cregex_captures(const cregex* self);

/* return CREG_OK, CREG_NOMATCH or CREG_MATCHERROR. */
int cregex_find(const cregex* re, const char* input,
                csview match[], int mflags);
static inline
int cregex_find_sv(const cregex* re, csview input, csview match[]) {
    csview *mp = NULL;
    if (match) { match[0] = input; mp = match; }
    return cregex_find(re, input.str, mp, CREG_M_STARTEND);
}

/* match + compile RE pattern */
int cregex_find_pattern(const char* pattern, const char* input,
                        csview match[], int cmflags);

static inline
bool cregex_is_match(const cregex* re, const char* input)
    { return cregex_find(re, input, NULL, CREG_DEFAULT) == CREG_OK; }

/* replace regular expression */ 
cstr cregex_replace_sv(const cregex* re, csview input, const char* replace, unsigned count,
                       bool (*mfun)(int i, csview match, cstr* mstr), int rflags);

static inline
cstr cregex_replace(const cregex* re, const char* input, const char* replace) { 
    csview sv = {input, strlen(input)};
    return cregex_replace_sv(re, sv, replace, ~0U, NULL, CREG_DEFAULT);
}

/* replace + compile RE pattern, and extra arguments */
cstr cregex_replace_pattern_ex(const char* pattern, const char* input, const char* replace, unsigned count,
                               bool (*mfun)(int i, csview match, cstr* mstr), int crflags);
static inline
cstr cregex_replace_pattern(const char* pattern, const char* input, const char* replace)
    { return cregex_replace_pattern_ex(pattern, input, replace, ~0U, NULL, CREG_DEFAULT); }

/* destroy regex */
void cregex_drop(cregex* self);

#endif // CREGEX_H_INCLUDED
#if defined i_extern || defined STC_EXTERN
#  include "../../src/cregex.c"
#  include "../../src/utf8code.c"
#endif
#undef i_opt
#undef i_header
#undef i_static
#undef i_implement
#undef i_extern
