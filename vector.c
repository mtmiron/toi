#include <stdlib.h>
#include <string.h>
#include "vector.h"

#ifndef VECTOR_DEFAULT_SIZE
#	define VECTOR_DEFAULT_SIZE 8
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* If there isn't room for another element, call realloc() */
static int
update_capacity(vector_t *v)
{
	VECTOR_DATA_T **buf;

	if (v->cap > v->num)
		return 0;

	buf = realloc(v->data, sizeof(VECTOR_DATA_T *) * v->cap * 2);
	if (!buf)
		return -1;

	v->data = buf;
	v->cap *= 2;
	return 0;
}

/* Explicitly realloc() a vector */
int
vector_resize(vector_t *v, int n)
{
	VECTOR_DATA_T **buf;

	if (n == 0)
		n = v->num;
	else if (n < 0)
		n = v->num + n;

	buf = realloc(v->data, sizeof(VECTOR_DATA_T *) * n);
	if (!buf)
		return -1;

	v->data = buf;
	v->cap = n;
	if (v->cap < v->num)
		v->num = v->cap;

	return v->cap;
}

/* Call the given function for each element */
void
vector_foreach(volatile vector_t *v, int (*func)(ANYARGS))
{
	int i, ret;

	for (i = 0; i < v->num; i++)
	{
		ret = func(v->data[i]);
		if (ret == VECTOR_DELETE)
			vector_delete_at((vector_t *)v, i--);
		else if (ret == VECTOR_STOP)
			break;
	}
}

/*
 * Note that vector_shrink() doesn't play nice if multiple
 * threads are utilizing the same vector concurrently.
 *
 */

/* realloc() vector, balancing RAM usage and speed of insertions */
int
vector_shrink(vector_t *v)
{
	if (v->cap / 2 < v->num || v->cap <= VECTOR_DEFAULT_SIZE)
		return v->cap;
      
	while ( (v->cap /= 2) > v->num )
		;

	v->cap *= 2;
	v->data = realloc(v->data, sizeof(VECTOR_DATA_T *) * v->cap);
	return v->cap;
}

/* This is the same as vector_shrink(), but optimizes properly
	regardless of initial size -- slower & unnecessary unless
	vector_new_with_size() was used to initialize the vector */
int
vector_optimize(vector_t *v)
{
	unsigned int i;
	VECTOR_DATA_T **buf;

	if (v->num < 1024)
		i = VECTOR_DEFAULT_SIZE;
	else
		i = 2048;

	for (; v->num > i; i *= 2);

	buf = realloc(v->data, sizeof(VECTOR_DATA_T *) * i);
	if (!buf)
		return -1;

	v->cap = i;
	v->data = buf;
	return i;
}

/* Initialize a vector, explicitly stating the initial capacity */
vector_t *
vector_new_with_size(int n)
{
	vector_t *ptr;

	ptr = malloc(sizeof(vector_t));
	if (!ptr)
		return NULL;

	if (n <= 0)
		n = 2;
	ptr->data = malloc(sizeof(VECTOR_DATA_T *) * n);
	if (!ptr->data)
	{
		free(ptr);
		return NULL;
	}
	ptr->cap = n;
	ptr->num = 0;
	return ptr;
}

/* Initialize a vector with the default capacity */
vector_t *
vector_new()
{
	return vector_new_with_size(VECTOR_DEFAULT_SIZE);
}

/* free() a vector */
void
vector_free(vector_t *v)
{
	free(v->data);
	v->data = NULL;
	free(v);
	v = NULL;
}

/* free() a vector and all the pointers it contains -- can segfault */
void
vector_free_unsafe(vector_t *v)
{
	int i;

	for (i = 0; i < v->num; i++)
	{
		if (v->data[i])
		{
			free(v->data[i]);
			v->data[i] = NULL;
		}
	}
	vector_free(v);
}

/* Push a value on to the vector */
vector_t *
vector_push(vector_t *v, void *ptr)
{
	if (update_capacity(v))
		return NULL;

	v->data[v->num++] = (VECTOR_DATA_T *)ptr;
	return v;
}

/* Insert a value at the beginning of a vector -- much slower
   than vector_push() */
vector_t *
vector_unshift(vector_t *v, void *ptr)
{
	if (update_capacity(v))
		return NULL;
	
	memmove(v->data + 1, v->data, sizeof(VECTOR_DATA_T *) * v->num++);
	v->data[0] = (VECTOR_DATA_T *)ptr;
	return v;
}

/* As below, but a bit faster... a ``safe'' alternative is provided
   incase this function doesn't perform properly */
void *
vector_shift(vector_t *v)
{
	VECTOR_DATA_T *ptr;
	register int i;
	int len;

	if (!v->num)
		return NULL;

	ptr = v->data[0];
	len = --v->num;
	for (i = 0; i <= len; v->data[i] = v->data[++i])
		;

	return (void *)ptr;
}

/* Delete the first value in a vector, shifting all elements
   down accordingly -- much slower than vector_pop() */
void *
vector_shift_safe(vector_t *v)
{
	VECTOR_DATA_T *ptr;

	if (!v->num)
		return NULL;

	ptr = v->data[0];
	memmove(v->data, v->data+1, sizeof(VECTOR_DATA_T *) * --v->num);
	return (void *)ptr;
}

void
vector_clear(vector_t *v)
{
	VECTOR_DATA_T **buf;

	buf = realloc(v->data, sizeof(VECTOR_DATA_T*)*VECTOR_DEFAULT_SIZE);
	if (!buf)
		return;

	v->data = buf;
	v->num = 0;
	v->cap = VECTOR_DEFAULT_SIZE;
}

/* Delete the last element */
void *
vector_pop(vector_t *v)
{
	VECTOR_DATA_T *ptr;

	if (!v->num)
		return NULL;

	ptr = v->data[--v->num];
	return (void *)ptr;
}

/* Reference a vector element by array-style index */
void *
vector_aref(vector_t *v, int idx)
{
	if (idx < 0)
		idx = v->num + idx;
	if (idx >= v->num || idx < 0)
		return NULL;

	return (void *)v->data[idx];
}

/* Set a vector element by array-style index */
void
vector_aset(vector_t *v, int idx, void *ptr)
{
	if (idx < 0)
		idx = v->num + idx;
	if (idx >= v->num || idx < 0)
		return;

	v->data[idx] = ptr;
}

/* Delete a vector element by array-style index */
void *
vector_delete_at(vector_t *v, int idx)
{
	void *ptr;

	if (idx < 0)
		idx = v->num + idx;
	if (idx >= v->num || idx < 0)
		return NULL;

	ptr = (void *)v->data[idx];
	memmove(v->data + idx, v->data + 1 + idx, sizeof(VECTOR_DATA_T *) * (--v->num - idx));
	return ptr;
}

void *
vector_last(vector_t *v)
{
	return vector_aref(v, v->num - 1);
}

void *
vector_first(vector_t *v)
{
	return vector_aref(v, 0);
}

/* Delete any elements that match a value or memory address */
void *
vector_delete(vector_t *v, void *ptr)
{
	int i;
	void *ret = NULL;

	for (i = 0; i < v->num; i++)
		if (v->data[i] == ptr)
			ret = vector_delete_at(v, i);

	return ret;
}

/* Create and return a new vector consisting of a range of values
   in the passed vector */
vector_t *
vector_range(vector_t *v, int lo, int hi)
{
	int i;
	vector_t *v2;

	if (!v->num)
		return NULL;
	if (lo < 0)
		lo = v->num + lo;
	if (hi < 0)
		hi = v->num + hi;
	if (lo > hi)
	{
		i = hi;
		hi = lo;
		lo = i;
	}
	if (hi > v->num)
		hi = v->num;
	if (lo > v->num)
		lo = v->num;

	v2 = vector_new();
	for (i = lo; i <= hi; i++)
		vector_push(v2, v->data[i]);

	return v2;
}

/* Create and return a vector consisting of the current vector's elements
   in reverse order */
vector_t *
vector_reverse(vector_t *v)
{
	vector_t *vr;
	int i;

	i = vector_shrink(v);
	vr = vector_new_with_size(i);

	for (i = 0; i < v->num; i++)
		vector_push(vr, (void *)v->data[i]);

	return vr;
}

/* Return the number of elements in the given vector */
int
vector_length(vector_t *v)
{
	return v->num;
}

#ifdef __cplusplus
}
#endif
