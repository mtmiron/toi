/*
 *
 * Generic lib for manipulating strings
 * that dynamically-resize at run-time.
 *
 * Copyright (C) 2007-2008 Murray Miron
 *
*/

#ifndef DYNSTRING_H_INCLUDED
#define DYNSTRING_H_INCLUDED

#ifndef DYNSTRING_DEFAULT_SIZE
#	define DYNSTRING_DEFAULT_SIZE 16
#endif

#include <stdlib.h>
#include <string.h>

typedef struct {
	char *ptr;
	int len;
	int capa;
} dynstring_t;

typedef dynstring_t* DYNSTRING;

#ifndef DYNSTRING
#	define DYNSTRING dynstring_t*
#endif

#define dstrlen(str) (((DYNSTRING)(str))->len)
#define dstrsize(str) dstrlen(str)
#define dstr_capa(str) (((DYNSTRING)(str))->capa)

#define dstr_str(str) ((char*)(((DYNSTRING)(str))->ptr))
#define dstrval(str) dstr_str(str)

DYNSTRING dstrcat(DYNSTRING, char *);
DYNSTRING dstrcpy(DYNSTRING, char *);

char dstr_last_char(DYNSTRING);
char dstr_aref(DYNSTRING, int);
DYNSTRING dstr_aset(DYNSTRING, int, char *);

DYNSTRING dstr_shrink(DYNSTRING);
DYNSTRING dstr_grow(DYNSTRING);
DYNSTRING dstr_grow_by(DYNSTRING, int);

DYNSTRING dstr_new();
DYNSTRING dstr_new_with_size(int);


void dstr_free(DYNSTRING);
DYNSTRING dstr_clear(DYNSTRING);

#endif
