/*
 * eval.c:
 *
 * 	Contains most of the core evaluation/interpretation functions
 * 	used on an already-tokenized (i.e. parsed) script file.
 *
 */

#include "toi.h"


#ifdef __cplusplus
extern "C" {
#endif


VALUE
toi_funcall0(VALUE recv, VALUE fptr, int argc, VALUE parms[])
{
	check_type(fptr, T_CREF);
	VALUE argv[8];
	int i;

	for (i = 0; i < argc; i++)
		argv[i] = eval_cell(parms[i]);

	/* for variadic functions, let them sort out the argument array */
	if (CREF(fptr)->argc == -1)
		return (VALUE)CREF(fptr)->ptr(recv, argc, argv);

	if (CREF(fptr)->argc > argc)
		fail("too few arguments to function call");
	else if (CREF(fptr)->argc < argc)
		fail("too many arguments to function call");

	switch (argc)
	{
		case 0: return (VALUE)CREF(fptr)->ptr(recv);
		case 1: return (VALUE)CREF(fptr)->ptr(recv, argv[0]);
		case 2: return (VALUE)CREF(fptr)->ptr(recv, argv[0], argv[1]);
		case 3: return (VALUE)CREF(fptr)->ptr(recv, argv[0], argv[1], argv[2]);
		case 4: return (VALUE)CREF(fptr)->ptr(recv, argv[0], argv[1], argv[2], argv[3]);
		case 5: return (VALUE)CREF(fptr)->ptr(recv, argv[0], argv[1], argv[2], argv[3], argv[4]);
		case 6: return (VALUE)CREF(fptr)->ptr(recv, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
		case 7: return (VALUE)CREF(fptr)->ptr(recv, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
		default: fail("invalid number of arguments given to toi_funcall0() (must be <= 7)");
	}

	return (VALUE)Qnil;
}


VALUE
toi_funcall(VALUE recv, VALUE sym, int argc, ...)
{
	va_list ap;
	int i;
	VALUE argv[argc];
	VALUE m = Qnil;
	VALUE ret = Qnil;

	va_start(ap, argc);
	for (i = 0; i < argc; i++)
		argv[i] = va_arg(ap, VALUE);
	va_end(ap);

	if (TYPE(recv) != T_CLASS)
		m = get_method(CLASS_OF(recv), sym);
	else
		m = get_method(recv, sym);

	if (TYPE(m) != T_CREF)
	{
		if (TYPE(m) != T_MCALL)
		{
			if (!TEST(m))
				fail("bug: meth is nil");
			funcall(get_variable(intern("stderr")), "write", 1, string_new("bug: type of meth is `"));
			funcall(get_variable(intern("stderr")), "write", 1, string_new((char*)type2cstring(TYPE(m))));
			funcall(get_variable(intern("stderr")), "write", 1, string_new("'. "));
			fail("method is neither cref nor mcall");
		}
		return mcall_call(m);
	}

	ret = (VALUE)toi_funcall0(recv, m, argc, argv);

	return ret;
}


VALUE
funcall(VALUE recv, char *str, int argc, ...)
{
	va_list ap;
	int i;
	VALUE argv[argc];

	va_start(ap, argc);
	for (i = 0; i < argc; i++)
		argv[i] = va_arg(ap, VALUE);
	va_end(ap);

	switch (argc)
	{
		case 0: return toi_funcall(recv, intern(str), argc);
		case 1: return toi_funcall(recv, intern(str), argc, argv[0]);
		case 2: return toi_funcall(recv, intern(str), argc, argv[0], argv[1]);
		case 3: return toi_funcall(recv, intern(str), argc, argv[0], argv[1], argv[2]);
		case 4: return toi_funcall(recv, intern(str), argc, argv[0], argv[1], argv[2], argv[3]);
		case 5: return toi_funcall(recv, intern(str), argc, argv[0], argv[1], argv[2], argv[3], argv[4]);
		case 6: return toi_funcall(recv, intern(str), argc, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
		case 7: return toi_funcall(recv, intern(str), argc, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
		default: fail("parameter error in funcall(): bad number of arguments");
	}

	return Qnil;
}


static VALUE
assign0(VALUE wrapper)
{
	VALUE self, arg, val, oldval, oldlast, thr;
	st_table *tbl;

	check_type(wrapper, T_ASSIGN);
	self = ASSIGN(wrapper)->lhs;
	arg = ASSIGN(wrapper)->rhs;

	if (TYPE(self) != T_SYMBOL)
		return toi_funcall(self, intern("="), 1, arg);

	thr = var_in_scope(self, cur_thr);
	if (!TEST(thr))
		thr = cur_thr;

	tbl = THREAD(thr)->env_tbl;
	oldlast = THREAD(thr)->last;
	val = eval_cell(arg);

	THREAD(thr)->last = oldlast;
	if ( st_lookup(tbl, (st_data_t)self, (st_data_t*)&oldval) )
		object_refcount_down((void*)oldval);

	THREAD(thr)->last = val;
	object_refcount_up((void*)val);
	st_add_direct(tbl, (st_data_t)self, (st_data_t)val);
#ifdef SYMBOL_CACHE
	st_add_direct(scache_tbl, (st_data_t)self, (st_data_t)val);
#endif
	return val;
}


VALUE
assign(VALUE self, VALUE arg)
{
	VALUE obj;

	obj = NEW_OBJ(Assign);
	OBJ_SETUP(obj, T_ASSIGN);
	BASIC(obj).klass = cObject;

	ASSIGN(obj)->lhs = self;
	ASSIGN(obj)->rhs = arg;

	return obj;
}


VALUE
check_type(VALUE obj, int type)
{
	VALUE serr, sout;

	if (TYPE(obj) == type)
		return Qnil;

	serr = check_variable(intern("stderr"), main_thread());
	if (!TEST(serr))
		fail("fatal bug: error reporting an error");

	toi_funcall(serr, intern("write"), 1, string_new("type mismatch: `"));
	toi_funcall(serr, intern("write"), 1, inspect(obj));
	toi_funcall(serr, intern("write"), 1, string_new("' != `"));
	toi_funcall(serr, intern("write"), 1, string_new((char*)typestr[type]));
	toi_funcall(serr, intern("puts"), 1, string_new("'"));

	abort();
}

VALUE
eval_thread(VALUE thr, VALUE parm)
{
	VALUE sav_thr = cur_thr, sav_self = cself, ret = Qnil;
	st_table *sav_env = NULL;

	/* handle recursive calls by initializing a new environment */
	if (cur_thr == thr)
	{
		sav_env = THREAD(thr)->env_tbl;
		THREAD(thr)->env_tbl = st_init_numtable();
	}

	if (THREAD(thr)->parmsym)
	{
		st_add_direct(THREAD(thr)->env_tbl,
		          (st_data_t)THREAD(thr)->parmsym,
		          (st_data_t)eval_cell(parm));
	}
	st_add_direct(THREAD(thr)->env_tbl,
	          (st_data_t)intern("self"),
	          (st_data_t)THREAD(cur_thr)->recv);

	cur_thr = thr;
	cself = THREAD(thr)->recv;
	ret = eval_stack(THREAD(thr)->stack);
	if (sav_env)
	{
		st_free_table(THREAD(thr)->env_tbl);
		THREAD(sav_thr)->env_tbl = sav_env;
	}
	cur_thr = sav_thr;
	cself = sav_self;
	return ret;
}


VALUE
eval_stack(vector_t *stk)
{
	VALUE cell, val;
	int i = 0, *thr_ip = &THREAD(cur_thr)->ip;

	while ( (cell = (VALUE)vector_aref(stk, i++)) )
	{
		*thr_ip = i;
		val = eval_cell(cell);
		if (val == Qbreak)
			break;
		THREAD(cur_thr)->last = val;
	}

	vector_foreach(THREAD(cur_thr)->obj_stk, object_refcount_check);
	return val;
}


VALUE
eval_cell(VALUE cell)
{
	VALUE ret = Qnil;
	VALUE save_thr;
	VALUE loop_cond;
	vector_t *loop_stk;

	if (!TEST(cell))
		return Qnil;

//	THREAD(cur_thr)->last = cell;
	switch (TYPE(cell))
	{
		case T_NIL:
		case T_TRUE:
		case T_CLASS:
		case T_THREAD:
		case T_ARRAY:
		case T_HASH:
		case T_STRING:
		case T_INTEGER:
		case T_FLOAT:
		case T_FILE:
		case T_BREAK:
			/* fall through */
		case T_IOSTREAM:
			return cell;

		case T_SYMBOL:
			ret = sym2ref(cell);
			if (ret != cell)
				return eval_cell(ret);
			else
				return Qnil;

		case T_BEGIN:
			fail("TOI bug: current cell on the exec stack is of type T_BEGIN");
			break;

		case T_EXCEPTION:
			do {
				cur_thr = EXCEPTION(cell)->body_thr;
				THREAD(cur_thr)->ip = 0;
				EXCEPTION(cell)->retry = (unsigned char)0;
				ret = eval_stack(THREAD(cur_thr)->stack);
			} while ( EXCEPTION(cell)->retry );

			cur_thr = EXCEPTION(cell)->thr;
			return ret;

		case T_CREF:
			return (VALUE)toi_funcall0(cself, cell, 0, NULL);

		case T_MCALL:
			return (VALUE)mcall_call(cell);

		case T_ASSIGN:
			return (VALUE)assign0(cell);

		case T_BRANCH:
			save_thr = cur_thr;
			if ( TEST(eval_cell(BRANCH(cell)->cond)) )
			{
				cur_thr = cself = BRANCH(cell)->cond_true;
				if ( BRANCH(cell)->loop )
				{
					loop_cond = BRANCH(cell)->cond;
					loop_stk = THREAD(BRANCH(cell)->cond_true)->stack;
					do {
						ret = eval_stack(loop_stk);
					} while ( (ret != Qbreak) && TEST(eval_cell(loop_cond)) );
				}
				else
				{
					ret = eval_stack( THREAD(BRANCH(cell)->cond_true)->stack );
				}
			}
			else if (BRANCH(cell)->cond_false)
			{
				cur_thr = cself = BRANCH(cell)->cond_false;
				if ( BRANCH(cell)->loop )
				{
					loop_cond = BRANCH(cell)->cond;
					loop_stk = THREAD(BRANCH(cell)->cond_false)->stack;
					do {
						ret = eval_stack( loop_stk );
					} while ( (ret != Qbreak) && !TEST(eval_cell(loop_cond)) );
				}
				else
				{
					ret = eval_stack( THREAD(BRANCH(cell)->cond_false)->stack );
				}
			}
			cur_thr = cself = save_thr;

			if (!ret)
			{
				return Qnil;
			}
			else if (ret == Qbreak)
			{
				/* handle BREAK statements inside IF clauses
				   (if the current BRANCH is not a loop,
					propagate the BREAK up the stack) */
				if ( BRANCH(cell)->loop )
					ret = Qnil;
			}

			return ret;

		default:
			fprintf(stderr, "\n%d:", TYPE(cell));
			fail("type flag unrecognized during eval of cell");
	}

	if (!ret)
		ret = Qnil;

	return (VALUE)ret;
}


#ifdef __cplusplus
}
#endif
