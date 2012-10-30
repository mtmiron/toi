#include "dynstring.h"


/*
 * TODO: sprintf()
 *
 */



char
dstr_last_char(DYNSTRING dstr)
{
	return dstr_aref(dstr, -1);
}


char
dstr_aref(DYNSTRING dstr, int idx)
{
	if (idx < 0)
		idx = dstr->len + idx;
	if ((idx >= dstr->len) || (idx < 0))
		return (char)0;

	return dstr->ptr[idx];
}


DYNSTRING
dstr_aset(DYNSTRING dstr, int idx, char *strarg)
{
	int len;

	if (idx < 0)
		idx = dstr->len + idx;
	if (idx < 0)
		return dstr;

	len = strlen(strarg);
	while ((dstr->capa - idx) <= len)
		dstr_grow(dstr);

	strcpy(&dstr->ptr[idx], strarg);

	if (idx + len > dstr->len)
		dstr->len = idx + len;
	return dstr;
}


/*
 * Allocate and return a new dynstring object,
 *  optionally initializing data pointer to initstr
 *
 */
DYNSTRING
dstr_new(char *initstr)
{
	DYNSTRING str;
	int mincap;

	str = malloc(sizeof(DYNSTRING));
	if (initstr)
	{
		for ( mincap = strlen(initstr),
	      	str->capa = DYNSTRING_DEFAULT_SIZE;
		      str->capa <= mincap;
		      str->capa *= 2 )
			/* null */
		;
		str->ptr = malloc(sizeof(char) * str->capa);
		if (!str->ptr)
			return (DYNSTRING)NULL; /* should throw error */
		str->len = mincap;
		strcpy(str->ptr, initstr);
	}
	else
	{
		str->capa = DYNSTRING_DEFAULT_SIZE;
		str->ptr = malloc(sizeof(char) * str->capa);
		if (!str->ptr)
			return (DYNSTRING)NULL;
		str->len = 0;
	}

	return (DYNSTRING)str;
}


/*
 * Allocate with explicit initial capacity
 *
 */
DYNSTRING
dstr_new_with_size(int size)
{
	DYNSTRING str;

	str = malloc(sizeof(DYNSTRING));
	str->ptr = malloc(sizeof(char) * size);
	str->capa = size;
	str->len = 0;
	return str;
}


/*
 * Double the space allocated for data pointer
 *
 */
DYNSTRING
dstr_grow(DYNSTRING str)
{
	char *buf;

	if (!str->capa)
	{
		str->capa = DYNSTRING_DEFAULT_SIZE;
		str->ptr = malloc(str->capa * sizeof(char));
	}

	str->capa *= 2;
	buf = realloc(str->ptr, sizeof(char) * str->capa);
	if (!buf)
		str->capa /= 2;
	else
		str->ptr = buf;

	return str;
}


/*
 * Increase allocated data pointer size by specified length
 *
 */
DYNSTRING
dstr_grow_by(DYNSTRING dstr, int addlen)
{
	int newcap, mincap;
	char *buf;

	if (!dstr->capa)
	{
		dstr->capa = DYNSTRING_DEFAULT_SIZE;
		dstr->ptr = malloc(dstr->capa * sizeof(char));
	}

	newcap = dstr_capa(dstr);
	mincap = (addlen + dstrlen(dstr));

	for (newcap = dstr->capa; newcap <= mincap; newcap *= 2) /* null */;

	buf = realloc(dstr->ptr, sizeof(char) * newcap);
	if (!buf)
		return (DYNSTRING)NULL;

	dstr->ptr = buf;
	dstr->capa = newcap;
	dstr->len = mincap;

	return dstr;
}


/*
 * Free the mem pointed to by dynstring->ptr
 *  w/o freeing dynstring itself
 */
DYNSTRING
dstr_clear(DYNSTRING dstr)
{
	if (dstr->ptr && dstr->capa)
		free(dstr->ptr);
	dstr->ptr = NULL;
	dstr->capa = dstr->len = 0;

	return dstr;
}


/*
 * Free both data pointer and
 *  dynstring object
 *
 */
void
dstr_free(DYNSTRING str)
{
	dstr_clear(str);
	if ( str )
		free(str);

	str = NULL;
}


/*
 * Concatenate char* on to end of
 *  dynstring data ptr
 *
 */
DYNSTRING
dstrcat(DYNSTRING dstr, char *strptr)
{
	int dlen, slen, cmblen;
	char *dptr, *sta, *end;

	if (!dstr_capa(dstr))
		dstr_grow(dstr);

	dlen = dstrlen(dstr);
	slen = strlen(strptr);
	cmblen = dlen + slen;

	if ( cmblen >= dstr_capa(dstr) )
		dstr_grow_by(dstr, slen);
	dptr = dstr_str(dstr);

	sta = dptr;
	end = sta + strlen(dptr) - 1;

/*	if (strptr >= sta && strptr <= end)
		memmove((dptr + sizeof(char) * strlen(dptr)), strptr, strlen(strptr));
	else
*/
	strncat(dptr, strptr, dstr_capa(dstr));

	dptr[cmblen] = '\0'; /* ensure a terminating NULL byte is present */
	dstr->len = strlen(dptr);

	return dstr;
}


/*
 * Overwrite dynstring data ptr w/
 *  specified char string
 *
 */
DYNSTRING
dstrcpy(DYNSTRING dstr, char *strptr)
{
	int dlen, slen;
	char *dptr, *sta, *end;

	dlen = dstrlen(dstr);
	slen = strlen(strptr);

	while ( slen >= dstr_capa(dstr) )
		dstr_grow(dstr);
	dptr = dstr_str(dstr);

	sta = dptr;
	end = sta + strlen(dptr) - 1;

/*	TODO: fix segfault for overlapping mem addresses
	if (strptr >= sta && strptr <= end)
		memmove((dptr + sizeof(char) * strlen(strptr)), strptr, sizeof(char) * strlen(strptr));
	else
*/
	strncpy(dptr, strptr, dstr_capa(dstr));

	dstr->len = strlen(dptr);
	return dstr;
}


/*
 * free() any superfluous memory currently
 *  allocated by dynstring's char ptr
 *
 */
DYNSTRING
dstr_shrink(DYNSTRING dstr)
{
	int len = dstrlen(dstr), newcap = dstr->capa;
	char *buf;

	if (!dstrval(dstr))
		return dstr;

	while (newcap >= len && newcap > 1)
		newcap /= 2;

	buf = realloc(dstr->ptr, sizeof(char) * (newcap *= 2));
	if (!buf)
		return dstr;

	dstr->ptr = buf;
	dstr->capa = newcap;
	return dstr;
}
