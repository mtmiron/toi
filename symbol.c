/*
 * symbol.c:
 *
 * 	Implements the Symbol class and related methods/functions.  These
 * 	objects are ultimately used as hash keys for retrieving method and
 * 	identifier values from a given context's ``st_table'' hashes.
 *
 * 	See ``st.h'' and ``st.c'' for further information on the public-domain
 * 	symbol table library TOI uses (i.e., dictionaries/associative-arrays).
 *
 */

#include "toi.h"
#include <string.h>


VALUE Qnil;
VALUE Qtrue;
VALUE Qnewline;
VALUE Qbreak;
VALUE Qrecv;
VALUE Qdot;
VALUE cSymbol;

st_table *scache_tbl;
static st_table *sym_tbl;




#ifdef NO_SYMBOL_PERSISTANCE
/*	NOTE: Symbols are, by design, persistent.  Should never be called
		(this is left in as a reminder that symbols allocate heap space) */
void
symbol_reap(VALUE sym)
{
	check_type(sym, T_SYMBOL);

	if (SYMBOL(sym)->ptr)
		free(SYMBOL(sym)->ptr);
}

#else

void
symbol_reap(VALUE sym)
{
	return;
}

#endif


/*
 * A static cache of the latest symbol-to-reference
 * lookups is kept; this significantly increases speed.
 *
 */
VALUE
sym2ref(VALUE sym)
{
#ifdef csize
	#undef csize
#endif
#define csize 4
	static struct { VALUE s; VALUE v; VALUE t; } cache[csize + 1];
	static int curi = 0;
	int i;
	VALUE ret = sym;
	VALUE key, save_recv = THREAD(cur_thr)->recv;

	if (TYPE(sym) != T_SYMBOL)
	{
		fprintf(stderr, "%s:\n", str2cstr(inspect(sym)));
		fail("object is not a symbol");
	}

	if (st_lookup(THREAD(cur_thr)->env_tbl, (st_data_t)sym, (st_data_t*)&ret))
	{
		THREAD(cur_thr)->last = ret;
		return ret;
	}

#ifdef SYMBOL_CACHE
	for (i = 0; i < csize; i++)
		if ((cache[i].s == sym) && (cache[i].t == cur_thr) && !st_delete(scache_tbl, (st_data_t*)&sym, (st_data_t*)&key))
			return (VALUE)cache[i].v;
#endif

	if (TEST((ret = get_variable(sym))))
		;
	else if (TEST((ret = get_method(THREAD(cur_thr)->recv, sym))))
		;
	else if (TEST((ret = get_singleton_method(THREAD(cur_thr)->recv, sym))))
		;
	else if (TEST((ret = get_singleton_method(cKernel, sym))))
		;
	else
		return sym;

#ifdef SYMBOL_CACHE
	cache[curi].s = sym;
	cache[curi].v = ret;
	cache[curi].t = cur_thr;
	if (curi == csize)
		curi = 0;
	else
		curi++;
#endif

	THREAD(cur_thr)->last = ret;
	return (VALUE)ret;
}


static VALUE
symbol_new(char *str)
{
	VALUE sym;

	sym = NEW_OBJ(Symbol);
	OBJ_SETUP(sym, T_SYMBOL);
	BASIC(sym).klass = cSymbol;
/*	BASIC(sym).reap = symbol_reap; */
	BASIC(sym).refcount = GC_NEVER_FREE;
	SYMBOL(sym)->ptr = strdup(str);

	st_add_direct(sym_tbl, (st_data_t)str, (st_data_t)sym);
	return sym;
}


VALUE
intern(char *str)
{
	VALUE sym;

	if (!sym_tbl)
		sym_tbl = st_init_strtable();
	if (!st_lookup(sym_tbl, (st_data_t)str, (st_data_t*)&sym))
		sym = symbol_new(str);

	return sym;
}


VALUE
check_variable(VALUE sym, VALUE thr)
{
	st_table *tbl;
	VALUE ret = (VALUE)0;

	tbl = THREAD(thr)->env_tbl;

	st_lookup(tbl, (st_data_t)sym, (st_data_t*)&ret);
	return ret;
}


VALUE
get_variable(VALUE sym)
{
	VALUE ret = 0;
	VALUE thr = cur_thr;

	while (TEST(thr))
	{
		ret = check_variable(sym, thr);
		if (!ret)
			thr = THREAD(thr)->up;
		else
			break;
	}

	if (!ret)
	{
		ret = check_variable(sym, main_thread());
		if (TEST(ret))
			return ret;
		else
			return Qnil;
	}
	else
		return (VALUE)ret;
}


VALUE
var_in_scope(VALUE sym, VALUE thr)
{
	while (thr && TEST(thr))
	{
		if (st_lookup(THREAD(thr)->env_tbl, (st_data_t)sym, NULL))
			return thr;
		else
			thr = THREAD(thr)->up;
	}

	if (st_lookup(THREAD(main_thread())->env_tbl, (st_data_t)sym, NULL))
		return main_thread();

	return (VALUE)0;
}


VALUE
nil_to_s()
{
	return string_new("nil");
}

VALUE
true_to_s()
{
	return string_new("true");
}


VALUE
symbol_to_s(VALUE recv)
{
	return string_new(SYMBOL(recv)->ptr);
}


void
Init_symbol()
{
#ifdef SYMBOL_CACHE
	scache_tbl = st_init_numtable();
#endif

	if (!sym_tbl)
		sym_tbl = st_init_strtable();

	Init_class();
	if (!cSymbol)
		cSymbol = define_class(intern("Symbol"), cObject);

	define_method(cSymbol, intern("to_s"), symbol_to_s, 0);

	Qnil = NEW_OBJ(struct goon);
	OBJ_SETUP(Qnil, T_NIL);
	CLASS_OF(Qnil) = define_class(intern("NilClass"), cObject);

	Qtrue = NEW_OBJ(struct goon);
	OBJ_SETUP(Qtrue, T_TRUE);
	CLASS_OF(Qtrue) = define_class(intern("TrueClass"), cObject);
/*
	Qnewline = NEW_OBJ(struct goon);
	OBJ_SETUP(Qnewline, T_NEWLINE);
	CLASS_OF(Qnewline) = define_class(intern("NewlineClass"), cObject);

	Qdot = NEW_OBJ(struct goon);
	OBJ_SETUP(Qdot, T_DOT);
	CLASS_OF(Qdot) = define_class(intern("DotClass"), cObject);
*/
	Qbreak = NEW_OBJ(struct goon);
	OBJ_SETUP(Qbreak, T_BREAK);
	CLASS_OF(Qbreak) = define_class(intern("BreakClass"), cObject);

	define_method(CLASS_OF(Qnil), intern("to_s"), nil_to_s, 0);
	define_method(CLASS_OF(Qtrue), intern("to_s"), true_to_s, 0);

	define_global_variable(intern("nil"), Qnil);
	define_global_variable(intern("false"), Qnil);
	gc_never_free(Qnil);
	define_global_variable(intern("true"), Qtrue);
	gc_never_free(Qtrue);
	define_global_variable(intern("break"), Qbreak);
	gc_never_free(Qbreak);
}
