/*
 * gc.c:
 * 	Memory management functions and any operations
 * 	that fall under the "garbage collection class"
 * 	category.
 *
 */

#include "toi.h"


VALUE cGC;


int
object_refcount_down(void *ptr)
{
	VALUE obj = (VALUE)ptr;

	if (BASIC(obj).refcount == GC_NEVER_FREE)
		return VECTOR_CONTINUE;
	else if (THREAD(cur_thr)->last == obj)
		return VECTOR_CONTINUE;
	else if (--(BASIC(obj).refcount) > 0)
		return VECTOR_CONTINUE;

	memdealloc(obj);
	vector_delete(THREAD(cur_thr)->obj_stk, (void*)obj);
	return VECTOR_DELETE;
}


int
object_refcount_up(void *ptr)
{
	VALUE obj = (VALUE)ptr;

	if (BASIC(obj).refcount == GC_NEVER_FREE)
		return VECTOR_CONTINUE;

	BASIC(obj).refcount++;
	return VECTOR_CONTINUE;
}


int
object_refcount_check(void *ptr)
{
	VALUE obj = (VALUE)ptr;

	if ( (BASIC(obj).refcount == GC_NEVER_FREE) ||
	     (BASIC(obj).refcount > 0) )
		return VECTOR_CONTINUE;

	memdealloc(obj);
	return VECTOR_DELETE;
}


void *
memalloc(int size)
{
	void *ptr;

	ptr = calloc(1, size);
	if (!ptr)
		fail("calloc");

	return ptr;
}


void
memdealloc(VALUE obj)
{
#ifdef GCLOG
	static int fd = 0;
	char buf[65536];
#endif

	if (!TEST(obj))
		return;

	if (BASIC(obj).refcount == GC_NEVER_FREE)
		return;

	switch (TYPE(obj))
	{
		case T_CLASS:
		case T_SYMBOL:
		case T_MCALL:
		case T_CREF:
		case T_ASSIGN:
		case T_BRANCH:
		case T_BREAK:
		case T_TRUE:
		case T_NIL:
		case T_LABEL:
		case T_TRY:
		case T_CATCH:
		case T_BEGIN:
		case T_RESCUE:
		/* should never be garbage collected */
			return;
		
		case T_THREAD:
			if (main_thread() == obj)
				return;
		default:
			break;
	}

#ifdef GCLOG
	if (!fd)
	{
		fd = open("gc.log", O_WRONLY | O_CREAT | O_TRUNC);
		if (!fd)
		{
			perror("open");
			fail("error during logging of GC activity");
		}
	}
	snprintf(buf, sizeof(buf), "gc deallocating %s:%p\n", type2cstring(obj), (VALUE*)obj);
	write(fd, buf, strlen(buf));
#endif

	if (BASIC(obj).reap)
		BASIC(obj).reap(obj);

	free((VALUE*)obj);
}


VALUE
gc_s_run(VALUE self)
{
	VALUE sav_cur, thr = cur_thr;

	sav_cur = cur_thr;
	while (thr)
	{
		vector_foreach(THREAD(thr)->obj_stk, object_refcount_check);
		thr = cur_thr = THREAD(thr)->up;
	}
	cur_thr = sav_cur;

	return Qnil;
}


static int
gc_never_free0(void *ptr)
{
	VALUE obj = (VALUE)ptr;

	gc_never_free(obj);
	return VECTOR_CONTINUE;
}


static int
gc_never_free_hash0(st_data_t key, st_data_t entry, st_data_t arg)
{
	gc_never_free((VALUE)entry);
	return ST_CONTINUE;
}


VALUE
gc_never_free(VALUE obj)
{
	if (!obj)
		return Qnil;
	BASIC(obj).refcount = GC_NEVER_FREE;

	if (TYPE(obj) == T_ARRAY)
		vector_foreach(ARRAY(obj)->ptr, gc_never_free0);
	else if (TYPE(obj) == T_HASH)
		st_foreach(HASH(obj)->ptr, gc_never_free_hash0, 0);

	return obj;
}


void
Init_gc()
{
	cGC = define_class(intern("GC"), cObject);

	define_singleton_method(cGC, intern("run"), gc_s_run, 0);

	gc_never_free(get_variable(intern("ARGV")));
	gc_never_free(get_variable(intern("stdin")));
	gc_never_free(get_variable(intern("stdout")));
	gc_never_free(get_variable(intern("stderr")));
}
