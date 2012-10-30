/*
 * thread.c:
 *
 * 	The Thread class and related functions, including some
 * 	miscellaneous thread-related script-evaluation functions.
 *
 * 	See the ``toi.h'' file for more extensive information on
 * 	the various source files and their purposes.
 *
 */

#include "toi.h"


VALUE cur_thr;

VALUE cThread;
static VALUE thr_crit;
static vector_t *thr_stk;


static int
thread_reap_check(void *ptr)
{
	return object_refcount_down(ptr);
}


static void
thread_reap(VALUE thr)
{
	Thread *th;
	vector_t *v;

	th = THREAD(thr);
	v = th->obj_stk;
	vector_foreach(v, thread_reap_check);

	st_free_table(th->env_tbl);
	vector_free(th->obj_stk);
	vector_free(th->env_stk);
	vector_free(th->self_stk);
	vector_free(th->tok_stk);
	vector_free(th->stack);
#ifdef SYMBOL_CACHE
	vector_free(th->modified_syms);
#endif
}


VALUE
thread_new()
{
	VALUE thr;

	thr = NEW_OBJ(Thread);
	OBJ_SETUP(thr, T_THREAD);
	BASIC(thr).klass = cThread;

	vector_push(thr_stk, (void*)thr);

	THREAD(thr)->alive_p = 1;
	if (cur_thr)
		THREAD(thr)->branch = THREAD(cur_thr)->branch;
	if ((thr != main_thread()) && (thr != cur_thr))
		THREAD(thr)->recv = cur_thr;
	THREAD(thr)->up = cur_thr;
	THREAD(thr)->env_tbl = st_init_numtable();
	THREAD(thr)->obj_stk = vector_new();
	THREAD(thr)->self_stk = vector_new();
	THREAD(thr)->env_stk = vector_new();
	THREAD(thr)->tok_stk = vector_new();
	THREAD(thr)->stack = vector_new();
#ifdef SYMBOL_CACHE
	THREAD(thr)->modified_syms = vector_new();
#endif

	return thr;
}


VALUE
push_scope()
{
	VALUE thr;

	thr = thread_new();
	cur_thr = thr;

	return thr;
}


VALUE
pop_scope()
{
	VALUE tmp;

	vector_t *stk = thr_stk;
	VALUE thr;

	thr = (VALUE)vector_pop(stk);
	while ( (tmp = (VALUE)vector_pop(THREAD(thr)->obj_stk)) )
		memdealloc(tmp);

	cur_thr = THREAD(thr)->up;
	return thr;
}


VALUE
thread_s_main()
{
	VALUE thr;

	return (VALUE)vector_first(thr_stk);

	for (thr = cur_thr; TEST(thr); thr = THREAD(thr)->up)
		if (! TEST(THREAD(thr)->up) )
			return thr;
	return thr;
}


VALUE
main_thread()
{
	return thread_s_main();
}


VALUE
thread_s_current()
{
	return (VALUE)cur_thr;
}


VALUE
thread_set_critical(VALUE val)
{
	thr_crit = val;
	return val;
}


VALUE
thread_get_critical()
{
	return thr_crit;
}


static VALUE
thread_deadlock()
{
	fail("thread deadlock");
}


/* FIXME: incompatible with current design of soft-method-calling */
VALUE
thread_schedule()
{
	int i;
	VALUE th;

	/* don't increment round-robin count if in critical section */
	if ( TEST(thr_crit) || (vector_length(thr_stk) <= 1) )
	{
		if (!THREAD(cur_thr)->alive_p)
			thread_deadlock();
		return cur_thr;
	}

	/* find the highest priority thread; lowest round-robin count otherwise */
	for (i = 0; i < vector_length(thr_stk); i++)
	{
		th = (VALUE)vector_aref(thr_stk, i);
		if ( (THREAD(th)->prio > THREAD(cur_thr)->prio) ||
		     (!(THREAD(th)->prio < THREAD(cur_thr)->prio) && (THREAD(th)->cnt < THREAD(cur_thr)->cnt)) )
		{
			if (THREAD(th)->alive_p)
				cur_thr = (VALUE)vector_aref(thr_stk, i);
			else
				THREAD(th)->cnt++;
		}
	}

	THREAD(cur_thr)->cnt++;
	return cur_thr;
}


void
Init_thread()
{
	if (!thr_stk)
		thr_stk = vector_new();
	if (!cur_thr)
		cur_thr = thread_new();

	cThread = define_class(intern("Thread"), cObject);
	OBJ_SETUP(cur_thr, T_THREAD);
	BASIC(cur_thr).klass = cThread;
	define_method(cThread, intern("call"), eval_thread, 1);

	define_singleton_method(cThread, intern("main"), main_thread, 0);
	define_singleton_method(cThread, intern("current"), thread_s_current, 0);
	define_singleton_method(cThread, intern("critical"), thread_get_critical, 0);
	define_singleton_method(cThread, intern("setcritical"), thread_set_critical, 1);

}
