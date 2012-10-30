/*
 * init.c:
 *
 * 	Contains the core initialization functions that must be executed
 * 	both in order and prior to any of the other interpreter's code.
 *
 * 	Also responsible for calling the ``Init_{classname}'' functions that
 * 	by convention bootstrap the various class/method info implemented by
 * 	the interpreter's various source files.
 *
 */

#include "toi.h"


/* global variable symbol table */
st_table *gv_tbl;
static int sigint_flag;


void
toi_set_argv(int argc, char **argv)
{
	int i;
	VALUE ary = ary_new();

	for (i = 0; i < argc; i++)
		ary_push(ary, string_new(argv[i]));
	
	define_global_variable(intern("ARGV"), ary);
}


static void
handle_sigint()
{
	if (sigint_flag)
	{
		signal(SIGINT, SIG_DFL);
		raise(SIGINT);
	}
	else
	{
		fprintf(stderr, "\n%s\n", "caught SIGINT; send again to interrupt execution");
		sigint_flag = 1;
	}
}


void
toi_init(int argc, char **argv)
{
	gv_tbl = st_init_numtable();

	Init_thread();
	THREAD(cur_thr)->recv = main_thread();
	if (THREAD(main_thread())->env_tbl)
		st_free_table(THREAD(main_thread())->env_tbl);
	THREAD(main_thread())->env_tbl = gv_tbl;

	Init_symbol();
	Init_class();

	/* have to call Init_thread() again */
	Init_thread();

	Init_kernel();
	cself = cKernel;
	Init_numeric();
	Init_float();
	Init_integer();
	Init_array();
	Init_hash();
	Init_string();
	Init_iostream();
	Init_exception();

	toi_set_argv(argc, argv);

	Init_gc();

	signal(SIGINT, handle_sigint);
}
