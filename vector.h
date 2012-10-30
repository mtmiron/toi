/* `vector' is my own code.  It's a simple implementation in ANSI C of dynamically-adjusting
   arrays I got sick of constantly hand-tweaking sizes, so I threw it together. */
#ifndef VECTOR_INCLUDED
#define VECTOR_INCLUDED

#ifdef __cplusplus
#	include <cstdlib>
#endif

#ifndef VECTOR_DATA_T
#	define VECTOR_DATA_T unsigned long
#endif

#ifndef ANYARGS
#	ifdef __cplusplus
#		define ANYARGS ...
#	else
#		define ANYARGS
#	endif
#endif


typedef struct {
	VECTOR_DATA_T **data;
	int num;
	int cap;
} vector_t;

enum {
	VECTOR_DELETE = 1,
	VECTOR_CONTINUE,
	VECTOR_STOP,
};


vector_t *vector_new();
vector_t *vector_new_with_size(int);

void vector_foreach(volatile vector_t *, int (*)(ANYARGS));

int vector_optimize(vector_t *);
int vector_shrink(vector_t *);
int vector_resize(vector_t *, int);

void vector_free(vector_t *);
void vector_free_unsafe(vector_t *);

vector_t *vector_push(vector_t *, void *);
vector_t *vector_unshift(vector_t *, void *);
void *vector_pop(vector_t *);
void *vector_shift(vector_t *);

void *vector_delete_at(vector_t *, int);
void *vector_delete(vector_t *, void *);
void vector_clear(vector_t *);

void vector_aset(vector_t *, int, void *);
void *vector_aref(vector_t *, int);

void *vector_last(vector_t *);
void *vector_first(vector_t *);

vector_t *vector_range(vector_t *, int, int);
vector_t *vector_reverse(vector_t *);
int vector_length(vector_t *);

#endif
