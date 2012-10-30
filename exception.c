/*
 * exception.c:
 *
 * 	Functions related to exceptions and
 * 	handling thereof.
 *
 */




#include "toi.h"



VALUE cException;
VALUE Qretry;



static VALUE
get_current_exception_block()
{
	VALUE ret = Qnil, thr = cur_thr;

	while (TEST(thr))
	{
		//ret = (VALUE)vector_aref(THREAD(thr)->stack, THREAD(thr)->ip-1);
		ret = THREAD(thr)->excobj;
		if (TEST(ret) && (TYPE(ret) == T_EXCEPTION))
			return ret;
		else
			thr = THREAD(thr)->up;
	}

	return Qnil;
}


VALUE
exception_retry(VALUE self)
{
	VALUE exc;

	exc = get_current_exception_block();
	if (!TEST(exc))
		fail("TOI bug in exception_retry()");

	EXCEPTION(exc)->retry = (unsigned char)1;
	return Qbreak;
}


VALUE
throw_exc(VALUE self, int argc, VALUE *argv)
{
	/* argv[0] == VALUE string descrip; argv[1] == optional
	   exception class being thrown (default `Exception') */
	int i, len;
	VALUE buf, thr, ret, exc_type;
	VALUE exc;

	thr = cur_thr;
	exc = THREAD(thr)->excobj;

	while (!TEST(exc) && TEST(thr))
	{
		thr = THREAD(thr)->up;
		if (TEST(thr))
			exc = THREAD(thr)->excobj;
		else
			exc = Qnil;
	}

	if (!TEST(exc))
	{
		if ( (argc > 0) && argv[0] && (TYPE(argv[0]) == T_STRING) )
			fail(str2cstr(argv[0]));
		else
			fail("unhandled exception");
	}

	if ((argc > 0) && (argv[0]) && TEST(argv[0]))
		EXCEPTION(exc)->strerror = argv[0];
	else
		EXCEPTION(exc)->strerror = string_new("(details of exception unspecified)");

	if (argc > 1)
		exc_type = argv[1];
	else
		exc_type = cException;

	if (!TEST(exc) || !kind_of_p(exc, exc_type))
		fail("uncaught exception");

	if (TEST(EXCEPTION(exc)->rescue_thr))
	{
		thr = cur_thr;
		cur_thr = EXCEPTION(exc)->rescue_thr;
		eval_stack( THREAD(EXCEPTION(exc)->rescue_thr)->stack );
		ret = Qbreak;
		cur_thr = thr;
	}
	else
	{
		ret = THREAD(cur_thr)->last;
		cur_thr = EXCEPTION(exc)->thr;
	}

	return ret;
}

	
void
Init_exception()
{
	cException = define_class(intern("Exception"), cObject);

	Qretry = NEW_OBJ(struct goon);
	OBJ_SETUP(Qretry, T_RETRY);
	BASIC(Qretry).klass = cException;
	gc_never_free(Qretry);

	define_global_function(intern("fail"), throw_exc, -1);
	define_global_function(intern("throw"), throw_exc, -1);
	define_global_function(intern("retry"), exception_retry, 0);
}
