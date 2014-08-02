/*
 * class.c:
 *
 * 	Implements classes and their related functions
 * 	(class/method defining, basic object funcs, etc.)
 *
 */

#include "toi.h"


VALUE cObject;
VALUE cClass;
VALUE cMethod;
VALUE cur_class;



VALUE
obj_iv_set(VALUE recv, VALUE sym, VALUE val)
{
	st_table *m_tbl;

	m_tbl = BASIC(recv).iv_tbl;
	st_add_direct(m_tbl, (st_data_t)sym, (st_data_t)val);

	return val;
}


VALUE
obj_iv_get(VALUE recv, VALUE sym)
{
	st_table *m_tbl;
	VALUE ret = Qnil;

	m_tbl = BASIC(recv).iv_tbl;
	st_lookup(m_tbl, (st_data_t)sym, (st_data_t*)&ret);

	return ret;
}


VALUE
kind_of_p(VALUE sub, VALUE sup)
{
	VALUE buf;

	if (TYPE(sub) != T_CLASS)
		buf = CLASS_OF(sub);
	if (TYPE(sup) != T_CLASS)
		sup = CLASS_OF(sup);

	while (TEST(buf))
	{
		if (buf == sup)
			return buf;
		else
			buf = CLASS(buf)->super;
	}

	return Qnil;
}


VALUE
define_global_variable(VALUE sym, VALUE val)
{
	st_table *m_tbl;

	m_tbl = gv_tbl;
	st_add_direct(m_tbl, (st_data_t)sym, (st_data_t)val);

	return (VALUE)val;
}


VALUE
define_global_method(VALUE sym, VALUE (*func)(), int argc)
{
	if (!cKernel)
		cKernel = define_class(intern("Kernel"), cObject);

	return define_singleton_method(cKernel, sym, func, argc);
}


VALUE
define_variable(VALUE sym, VALUE val)
{
	st_table *m_tbl;

	m_tbl = THREAD(cur_thr)->env_tbl;
	st_add_direct(m_tbl, (st_data_t)sym, (st_data_t)val);

	return (VALUE)val;
}


VALUE
define_class(VALUE sym, VALUE super)
{
	VALUE k;

	if ( TEST((k = var_in_scope(sym, cur_thr))) )
		if ( TYPE( (k = check_variable(sym, k)) ) == T_CLASS )
			return k;

	k = NEW_OBJ(Klass);
	OBJ_SETUP(k, T_CLASS);
	BASIC(k).refcount = GC_NEVER_FREE;
	KLASS(k)->sym = sym;
	KLASS(k)->super = super;
	KLASS(k)->im_tbl = st_init_numtable();
	KLASS(k)->sm_tbl = st_init_numtable();
	
	define_variable(sym, k);
	return k;
}


VALUE
mcall_new(VALUE self, VALUE sym, int argc, ...)
{
	int i;
	va_list ap;
	VALUE fwrapper;

	fwrapper = NEW_OBJ(Mcall);
	OBJ_SETUP(fwrapper, T_MCALL);
	BASIC(fwrapper).klass = cMethod;

	va_start(ap, argc);
	for (i = 0; i < argc; i++)
		MCALL(fwrapper)->argv[i] = va_arg(ap, VALUE);
	va_end(ap);

	MCALL(fwrapper)->argc = argc;
	MCALL(fwrapper)->self = self;
	MCALL(fwrapper)->thr = cur_thr;
	MCALL(fwrapper)->sym = sym;

	return fwrapper;
}


/*
 * The majority of this inelegant code is due to
 * the small method cache that's kept from call
 * to call; it results in a significant increase
 * in the interpreter's speed.
 *
 */
VALUE
mcall_call(VALUE meth)
{
	VALUE save_self, recv, krecv, sym, func = Qnil, ret = Qnil;
	int i, argc;
	int curi = 0;
#ifdef csize
	#undef csize
#endif
#define csize 3
	static struct { VALUE k; VALUE s; VALUE f; } cache[csize];

	sym = MCALL(meth)->sym;
	recv = MCALL(meth)->self;
	if (TYPE(recv) == T_MCALL)
		recv = mcall_call(recv);
	else if (TYPE(recv) == T_SYMBOL)
		recv = sym2ref(recv);

	save_self = THREAD(cur_thr)->recv;

	/* fix for back-to-back singleton-method-calls */
	if (TYPE(recv) == T_CLASS)
		krecv = recv;
	else
		krecv = CLASS_OF(recv);

	for (i = 0; i <= csize; i++)
	{
		if ((cache[i].k == krecv) && (cache[i].s == sym))
		{
			func = cache[i].f;
			break;
		}
	}

	if (!TEST(func))
	{
		func = get_method(recv, sym);
		if (!TEST(func))
			func = get_singleton_method(recv, sym);
		if (!TEST(func))
		{
			func = get_singleton_method(cKernel, sym);
			if (!TEST(func))
			{
				func = get_method(cself, sym);
				if (!TEST(func))
				{
					func = get_singleton_method(cself, sym);
					if (!TEST(func))
					{
						func = get_method(cObject, sym);
						if (!TEST(func))
						{
							funcall(get_variable(intern("stderr")), "write", 1, funcall(recv, "to_s", 0));
							funcall(get_variable(intern("stderr")), "write", 1, string_new("."));
							toi_funcall(get_variable(intern("stderr")), intern("write"), 1, symbol_to_s(sym));
							fail(": mcall error, symbol is undefined in scope of receiving object");
						}
					}
				}
			}
		}
		cache[curi].k = krecv;
		cache[curi].s = sym;
		cache[curi].f = func;
		if (curi < csize)
			curi++;
		else
			curi = 0;
	}

	THREAD(cur_thr)->recv = THREAD(cur_thr)->last;

	if (TYPE(func) == T_CREF)
		ret = toi_funcall0(recv,
			func, MCALL(meth)->argc,
			MCALL(meth)->argv);
	else if (TYPE(func) == T_MCALL)
	{
		argc = MCALL(meth)->argc;
		for (i = 0; i < argc; i++)
			MCALL(func)->argv[i] = MCALL(meth)->argv[i];
		ret = mcall_call(func);
	}
	else
		fail("mcall error: unrecognized structure type");

	THREAD(cur_thr)->recv = THREAD(cur_thr)->last = save_self;
	return ret;
}


static VALUE
cref_new(VALUE (*fnc)())
{
	VALUE cref;

	cref = NEW_OBJ(Cref);
	OBJ_SETUP(cref, T_CREF);
	CREF(cref)->ptr = fnc;

	return cref;
}


VALUE
define_method(VALUE klass, VALUE sym, VALUE (*fnc)(ANYARGS), int argc)
{
	VALUE cref;
	st_table *m_tbl;

	m_tbl = KLASS(klass)->im_tbl;
	cref = cref_new(fnc);
	CREF(cref)->argc = argc;

	st_add_direct(m_tbl, (st_data_t)sym, (st_data_t)cref);
	return cref;
}


VALUE
define_soft_method(VALUE klass, VALUE sym, VALUE thr)
{
	VALUE meth;
	st_table *m_tbl;

	meth = mcall_new(thr, intern("call"), 1, THREAD(thr)->parmsym);
	m_tbl = KLASS(klass)->im_tbl;
	st_add_direct(m_tbl, (st_data_t)sym, (st_data_t)meth);

	return meth;
}


VALUE
define_singleton_method(VALUE klass, VALUE sym, VALUE (*fnc)(ANYARGS), int argc)
{
	VALUE cref;
	st_table *m_tbl;

	m_tbl = KLASS(klass)->sm_tbl;
	cref = cref_new(fnc);
	CREF(cref)->argc = argc;

	st_add_direct(m_tbl, (st_data_t)sym, (st_data_t)cref);
	return cref;
}


VALUE
get_method(VALUE k, VALUE sym)
{
	VALUE ret = Qnil;

	if (!k)
		k = cObject;
	if (TYPE(k) != T_CLASS)
		k = CLASS_OF(k);

	while (TEST(k))
	{
		if (st_lookup(KLASS(k)->im_tbl, (st_data_t)sym, (st_data_t*)&ret))
			return ret;
		else if (st_lookup(KLASS(k)->im_tbl, (st_data_t)intern("method_missing"), (st_data_t*)&ret))
			return ret;
		k = SUPER_CLASS(k);
	}

	if (!TEST(ret))
		st_lookup(KLASS(cKernel)->im_tbl, (st_data_t)sym, (st_data_t*)&ret);
	if (!TEST(ret))
		st_lookup(KLASS(cKernel)->sm_tbl, (st_data_t)sym, (st_data_t*)&ret);

	return (VALUE)ret;
}


VALUE
get_singleton_method(VALUE k, VALUE sym)
{
	VALUE ret = Qnil;

	if (TYPE(k) != T_CLASS)
		k = CLASS_OF(k);
	if (!k)
		k = cObject;

	while (TEST(k))
	{
		if (st_lookup(KLASS(k)->sm_tbl, (st_data_t)sym, (st_data_t*)&ret))
			return ret;
		else if (st_lookup(KLASS(k)->sm_tbl, (st_data_t)intern("method_missing"), (st_data_t*)&ret))
			return ret;
		k = SUPER_CLASS(k);
	}

	return (VALUE)ret;
}


static char *
class_to_cstr(VALUE obj)
{
	VALUE k;

	if (TYPE(obj) != T_CLASS)
		k = CLASS_OF(obj);
	else
		k = obj;

	if (!k)
		return "(no class)";
	return SYMBOL(CLASS(k)->sym)->ptr;
}


int
defined_p(VALUE recv, char *ptr)
{
	st_table *tbl;
	VALUE k;

	if (TYPE(recv) == T_CLASS)
		k = recv;
	else
		k = CLASS_OF(recv);

	if (!k || !TEST(k))
		return 0;

	tbl = CLASS(k)->im_tbl;
	return st_lookup(tbl, (st_data_t)intern(ptr), (st_data_t*)NULL);
}


VALUE
inspect(VALUE recv)
{
	char cstr[256];

	if (CLASS_OF(recv) != cObject && defined_p(recv, "inspect"))
		return toi_funcall(recv, intern("inspect"), 0);

#ifdef __cplusplus
	sprintf(cstr, "%s:%p", class_to_cstr(recv), (VALUE*)recv);
#else
	snprintf(cstr,sizeof(cstr)*sizeof(char),"%s:%p",
	         class_to_cstr(recv), (VALUE*)recv);
#endif
	return (VALUE)string_new(cstr);
}


VALUE
mcall_inspect(VALUE mobj)
{
	char buf[4096] = { '\0' };
	int argc, i;
	VALUE sym, parms = ary_new();

	argc = MCALL(mobj)->argc;
	sym = MCALL(mobj)->sym;
	for (i = 0; i < argc; i++)
		ary_push(parms, MCALL(mobj)->argv[i]);

	snprintf(buf, sizeof(buf), "Method:{sym=%s,argc=%d,argv=%s}",
	         str2cstr(funcall(sym,"to_s",0)), argc,
	         str2cstr(funcall(parms,"to_s",0)));

	return string_new(buf);
}


static int
methods_callback(st_data_t key, st_data_t entry, st_data_t ary)
{
	ary_push((VALUE)ary, funcall((VALUE)key, "to_s", 0));
	return ST_CHECK;
}


VALUE
methods(VALUE self)
{
	VALUE ary, klass;
	st_table *m_tbl;

	ary = ary_new();
	if (TYPE(self) != T_CLASS)
		klass = CLASS_OF(self);
	else
		klass = self;

	if (!TEST(klass))
		fail("bug: class unknown");
	m_tbl = KLASS(klass)->im_tbl;

	st_foreach(m_tbl, methods_callback, (st_data_t)ary);
	return ary;
}


void
Init_class()
{
	if (cObject)
		return;

	cObject = define_class(intern("Object"), Qnil);
	cClass = define_class(intern("Class"), cObject);
	cMethod = define_class(intern("Method"), cObject);

	cur_class = cObject;
	define_method(cObject, intern("to_s"), inspect, 0);
	define_method(cObject, intern("inspect"), inspect, 0);
	define_method(cObject, intern("methods"), methods, 0);

	define_method(cMethod, intern("call"), mcall_call, 0);
	define_method(cMethod, intern("inspect"), mcall_inspect, 0);
}
