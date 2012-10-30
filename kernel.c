/*
 * kernel.c:
 *
 * 	The `Kernel' class, which contains global methods/functions
 * 	and in other ways acts as the `global namespace', so to speak.
 *
 */

#include "toi.h"

VALUE file_puts_thunk(VALUE, int, VALUE*);

VALUE cKernel;


VALUE
kernel_puts(VALUE recv, VALUE arg)
{
	VALUE out;

	out = get_variable(intern("stdout"));

	return toi_funcall(out, intern("puts"), 1, arg);
}


VALUE
kernel_puts_thunk(VALUE recv, int argc, VALUE *argv)
{
	VALUE self = get_variable(intern("stdout"));

	return file_puts_thunk(self, argc, argv);
}


VALUE
kernel_write(VALUE recv, VALUE arg)
{
	VALUE out;

	out = get_variable(intern("stdout"));

	return toi_funcall(out, intern("write"), 1, arg);
}


VALUE
kernel_exit(VALUE recv, int argc, VALUE *argv)
{
	int exval;

	if ((argc > 0) && TEST(argv[0]))
		exval = 1;
	else
		exval = 0;

	exit(exval);
}


VALUE
define_global_function(VALUE sym, VALUE (*func)(), int argc)
{
	return define_method(cKernel, sym, func, argc);
}


VALUE
kernel_not(VALUE self, VALUE arg)
{
	if (TEST(arg))
		return Qnil;
	else
		return Qtrue;
}


VALUE
kernel_sleep(VALUE self, VALUE arg)
{
	int secs;

	if (TYPE(arg) != T_INTEGER)
		arg = funcall(arg, "to_i", 0);
		
	secs = INT(arg)->num;
	while (secs > 0)
		secs = sleep(secs);

	return Qnil;
}


VALUE
kernel_s_self(VALUE self)
{
	return THREAD(cur_thr)->recv;
}


void
Init_kernel()
{
	if (!cObject)
		cObject = define_class(intern("Object"), Qnil);
	if (!cKernel)
		cKernel = define_class(intern("Kernel"), cObject);

	define_global_function(intern("!"), kernel_not, 1);
	define_global_function(intern("puts"), kernel_puts_thunk, -1);
	define_global_function(intern("write"), kernel_write, 1);
	define_global_function(intern("print"), kernel_write, 1);
	define_global_function(intern("exit"), kernel_exit, -1);
	define_global_function(intern("sleep"), kernel_sleep, 1);

	define_global_function(intern("self"), kernel_s_self, 0);
}
