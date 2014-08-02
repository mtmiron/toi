#ifndef TOI_H_INCLUDED
#define TOI_H_INCLUDED

#include "vector.h"
#include "dynstring.h"
#include "st.h"
#include <ctype.h>
#include <setjmp.h>
#ifndef __MINGW32__
#	include <ucontext.h>
#endif
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <math.h>



/*
 * Note that a TOI object is represented by the C data
 * type "VALUE," which is defined as an unsigned long here.
 *
 * These are actually pointers to objects allocated on the
 * heap, and the RAM address -- after being cast to an
 * unsigned long int -- is passed throughout the C functions
 * and treated like an object ID or as the data itself rather
 * than a pointer to the object's data.  This basic casting is
 * the foundation upon which all of TOI is built.
 *
 */
#ifndef VALUE
#	define VALUE unsigned long int
#endif


#ifndef GC_NEVER_FREE
#	define GC_NEVER_FREE -32767
#endif


/* Basic macros for object allocation/manipulation */
#define ALLOC(type) (type*)memalloc(sizeof(type))
#define NEW_OBJ(type) (VALUE)ALLOC(type)

#define OBJ_SETUP(obj,objtype) do {\
	if (TEST(obj) && (obj != Qtrue))\
	{\
		if (objtype != T_THREAD)\
			vector_push(THREAD(cur_thr)->obj_stk, (void*)obj);\
		BASIC(obj).type = (int)objtype;\
	}\
} while (0)


/* Test a TOI object for its boolean-logic value */
#define TEST(obj) ((obj) && ((obj) != Qnil))

/* Macros for casting TOI objects to their C-type in order to
   access (or assign to, or otherwise use) their C struct values.
	Note the simple, basic CAST macro, which casts the long int
	back to a struct pointer (and nothing more). */
#define CAST(obj,type) ((type*)obj)
#define BASIC(obj) (CAST(obj,struct goon)->b)
#define ARRAY(obj) CAST(obj,Array)
#define HASH(obj) CAST(obj,Hash)
#define STRING(obj) CAST(obj,String)
#define INTEGER(obj) CAST(obj,Integer)
#define INT(obj) INTEGER(obj)
#define FLOAT(obj) CAST(obj,Float)
#define KLASS(obj) CAST(obj,Klass)
#define CLASS(obj) KLASS(obj)
#define THREAD(thr) CAST(thr,Thread)
#define IOSTREAM(obj) CAST(obj,IOStream)
#define SYMBOL(sym) CAST(sym,Symbol)
#define MCALL(mc) CAST(mc,MCall)
#define CREF(ref) CAST(ref,Cref)
#define BRANCH(obj) CAST(obj,Branch)
#define EXCEPTION(obj) CAST(obj,TOIException)
#define ASSIGN(obj) CAST(obj,Assign)

/* These expand to the type of an object (int, or `enum type') */
#define TYPE(obj) (BASIC(obj).type)
#define TYPE_OF(obj) TYPE(obj)

/* Throw an exception unless the type of an object matches */
#define Check_Type(o,t) check_type(o,t)

/* These expand to the `struct klass' object that represents an
   object's class/superclass (which store all defined methods) */
#define CLASS_OF(object) BASIC(object).klass
#define SUPER_CLASS(obj) ((TYPE(obj) == T_CLASS) ? KLASS(obj)->super : cObject)


/* Expands to a char pointer corresponding to the object's type */
#define type2cstring(obj) typestr[TYPE(obj)]

#define print_error(str) fprintf(stderr, "%s\n", str)
#define fail(str) do {\
	if ( TEST(THREAD(cur_thr)->excobj) )\
	{\
		throw_exc(cur_thr, 1, (VALUE*)string_new(str));\
		abort();\
	}\
	else\
	{\
		print_error(str);\
		abort();\
	}\
} while (0)

#ifdef sizeofptr
#	undef sizeofptr
#endif

#define sizeofptr(ptr) (int)*((int*)(ptr - sizeof(int)))


/* global vars (primarily classes that should be available to any
   source file) */
extern VALUE cur_thr;
extern VALUE cur_class;
extern VALUE cself;

extern st_table *gv_tbl;
extern st_table *scache_tbl;

extern VALUE cKernel;
extern VALUE cObject;
extern VALUE cClass;
extern VALUE cMethod;
extern VALUE cArray;
extern VALUE cHash;
extern VALUE cNumeric;
extern VALUE cFloat;
extern VALUE cInteger;
extern VALUE cThread;
extern VALUE cSymbol;
extern VALUE cString;
extern VALUE cIOStream;
extern VALUE cFile;
extern VALUE cGC;
extern VALUE cException;

extern VALUE Qtrue;
extern VALUE Qnil;
extern VALUE Qretry;
extern VALUE Qnewline;
extern VALUE Qrecv;
extern VALUE Qdot;
extern VALUE Qbreak;
#define Qfalse Qnil
#define MCall Mcall


/* prototypes for any funcs used by most (or all) files */
VALUE intern(char *);
VALUE sym2ref(VALUE);

VALUE define_class(VALUE, VALUE);
VALUE define_method(VALUE, VALUE, VALUE(*)(), int);
VALUE define_soft_method(VALUE,VALUE,VALUE);
VALUE define_global_method(VALUE, VALUE(*)(), int);
VALUE define_global_function(VALUE, VALUE(*)(), int);
VALUE define_singleton_method(VALUE, VALUE, VALUE (*)(), int);

VALUE obj_iv_set(VALUE,VALUE,VALUE);
VALUE obj_iv_get(VALUE,VALUE);

VALUE var_in_scope(VALUE, VALUE);
VALUE mcall_call(VALUE);

VALUE float_new(float);

VALUE integer_new(int);

VALUE string_new(char *);
char string_c_last_char(VALUE);

VALUE symbol_to_s(VALUE);

VALUE ary_new();
VALUE ary_push(VALUE,VALUE);
VALUE ary_pop(VALUE);
VALUE ary_aref(VALUE,VALUE);
VALUE ary_shift(VALUE);
VALUE ary_unshift(VALUE,VALUE);
VALUE ary_length(VALUE);

VALUE hash_new();
VALUE hash_aset(VALUE,VALUE,VALUE);
VALUE hash_aref(VALUE,VALUE);

VALUE file_new(char *, char *);
VALUE file_write(VALUE, VALUE);
VALUE file_close(VALUE);

VALUE mcall_new(VALUE, VALUE, int, ...);

VALUE get_method(VALUE,VALUE);
VALUE get_singleton_method(VALUE,VALUE);


char *str2cstr(VALUE);

VALUE inspect(VALUE);

VALUE define_global_variable(VALUE, VALUE);
VALUE define_global_function(VALUE, VALUE (*)(), int);

VALUE define_variable(VALUE, VALUE);
VALUE set_variable(VALUE, VALUE);
VALUE get_variable(VALUE);

VALUE eval_thread(VALUE,VALUE);
VALUE eval_stack(vector_t*);
VALUE eval_cell(VALUE);

VALUE push_self(VALUE);
VALUE pop_self();

VALUE push_cell(VALUE);

VALUE push_scope();
VALUE pop_scope();
VALUE set_cur_env(st_table *);

VALUE main_thread();
VALUE thread_new();

VALUE check_type(VALUE,int);
VALUE kind_of_p(VALUE,VALUE);

VALUE toi_funcall(VALUE, VALUE, int, ...);
VALUE funcall(VALUE, char *, int, ...);

VALUE softwrap_new(VALUE);
VALUE softwrap_call(VALUE, VALUE);

VALUE throw_exc(VALUE, int, VALUE *);


/*
 * This MUST be called for any object that
 * the garbage collector shouldn't free()
 *
 */
VALUE gc_never_free(VALUE);


void Init_thread();
void Init_symbol();
void Init_string();
void Init_class();
void Init_kernel();
void Init_numeric();
void Init_integer();
void Init_float();
void Init_array();
void Init_hash();
void Init_iostream();
void Init_gc();

void *memalloc(int);
void memdealloc(VALUE);

int object_refcount_down(void *);
int object_refcount_up(void *);
int object_refcount_check(void *);

void toi_init(int, char**);



/* Enumerate all possible TOI object types */
enum obj_type {
	T_NIL = 0,
	T_TRUE = 1,
	T_CREF = 2,
	T_MCALL = 3,
	T_THREAD = 4,
	T_CLASS = 5,
	T_SYMBOL = 6,
	T_STRING = 7,
	T_INTEGER = 8,
	T_FLOAT = 9,
	T_ARRAY = 10,
	T_HASH = 11,
	T_IOSTREAM = 12,
	T_ASSIGN = 13,
	T_BRANCH = 14,
	T_BREAK = 15,
	T_FILE = 16,
	T_BEGIN = 17,
	T_RESCUE = 18,
	T_EXCEPTION = 19,
	T_RETRY = 20,

	/* TODO: the following are currently unimplemented */
	T_TRY,
	T_CATCH,
	T_LABEL,
	T_JUMP,
	T_RETURN,
	T_NEWLINE,
	T_DOT,
};

/* string-descriptions of the types (type val == idx: order-sensitive) */
static const char *typestr[] = {
	"Nil",
	"True",
	"Cref",
	"Mcall",
	"Thread",
	"Class",
	"Symbol",
	"String",
	"Integer",
	"Float",
	"Array",
	"Hash",
	"IOStream",
	"Assignment",
	"Branch",
	"Break",
	"File",
	"Begin",
	"Rescue",
	"Exception",
	"Retry",

	/* TODO: unimplemented */
	"Try",
	"Catch",
	"Label",
	"Jump",
	"Return",
	"Newline",
	"Dot",
};


/*
 * All objects that can be referenced from within a TOI script
 * have a structure member called "b" of type "struct basic," as
 * seen below.  It absolutely MUST be the first member element of
 * any TOI object, and the compiler absolutely MUST respect the
 * order in which struct elements are defined in the source.
 *
 * To check an object's type, we just check the first element of
 * its struct: if that isn't guaranteed to be the basic info, TOI
 * will either segfault or just behave totally unpredictably.
 *
 *
 * If the reader is unclear as to how this works: the compiler
 * makes the first ``sizeof(struct basic)'' bytes of all TOI
 * objects the same, regardless of their higher-order data type.
 *
 * When we reference these first bytes by casting the data to a
 * ``(struct goon*)'' and accessing the struct member ``b'', the
 * compiler is producing machine code that accesses the first bytes
 * at whatever RAM address the C identifier is referencing.
 *
 * Due to this behavior, so long as the compiler uniformly organizes
 * all of these structs based on the source code definitions, we can
 * ignore any data beyond those first bytes and use them to check any
 * object's ``struct basic'' data to know how we should deal with the
 * rest of the data.
 *
 */
typedef struct basic {
	int type;
	int refcount; /* references to this object (used by the GC) */
	VALUE klass;
	void (*reap)(VALUE); /* func to free object's RAM; if defined, called as `reap(VALUE self)' */
	st_table *iv_tbl; /* any instance variables tied to this object */
} Basic;

/* Used as a universal cast for checking object types */
struct goon {
	Basic b;
};

typedef struct string {
	Basic b;
	dynstring_t *ptr;
} String;

typedef struct array {
	Basic b;
	vector_t *ptr;
} Array;

typedef struct hash {
	Basic b;
	st_table *ptr;
} Hash;

typedef struct integer {
	Basic b;
	long int num;
} Integer;

typedef struct phloat {
	Basic b;
	float num;
} Float;

/* Symbols represent st_table entries (they're ultimately
   hash keys for the method/variable tables) */
typedef struct symbol {
	Basic b;
	char *ptr;
} Symbol;

/* IOStream objects encapsulate files, sockets, etc. */
typedef struct iostream {
	Basic b;
	VALUE path;
	int lineno;
	FILE *fread;
	FILE *fwrite;
} IOStream;

typedef struct toi_exception {
	Basic b;
	VALUE strerror;
	VALUE thr;
	VALUE body_thr;
	VALUE rescue_thr;
	VALUE catch_type;
	unsigned char retry;
} TOIException;

/* Classes hold instance methods (im_tbl) and singleton
   methods (sm_tbl) (both C funcs and TOI meths) */
typedef struct klass {
	Basic b;
	VALUE sym;
	VALUE super;
	st_table *im_tbl;
	st_table *sm_tbl;
} Klass;

/* A Cref is a wrapper for a C function pointer */
typedef struct cref {
	Basic b;
	VALUE (*ptr)(ANYARGS);
	VALUE self;
	VALUE parmsym;
	int argc;
} Cref;

/* Mcall objects are similar to Crefs, but they only
   represent TOI-defined methods (no compiled funcs) */
typedef struct mcall {
	Basic b;
	VALUE thr;
	VALUE sym;
	VALUE self;
	int argc;
	VALUE argv[8];
} Mcall;

/* Wrapper for dynamic (runtime) variable assignment */
typedef struct assign {
	Basic b;
	VALUE lhs;
	VALUE rhs;
} Assign;

/* For holding if/while data */
typedef struct branch {
	Basic b;
	int loop;	/* loop flag (is this a `while' or an `if') */
	VALUE brk;
	VALUE cond;
	VALUE cond_true;
	VALUE cond_false;
} Branch;

typedef struct toi_jmpbuf {
	Basic b;
	VALUE thr; /* the thread this jmpbuf is local to */
	int ip; /* instruction pointer (destination) */
	int ret; /* return pointer (position to return to) */
} Jump;

/* A TOI "Thread" object is a very broad, general data
   structure.  They hold various information about a
   given thread of execution (environment, stack, and
	other necessary values).  They serve not only as
	thread containers, but also as scope structures. */
typedef struct thr {
	Basic b;
	VALUE up; /* one-level-up scope */
	VALUE recv; /* current receiving object for funcalls */
	VALUE last;

	int ip;
	int prio; /* thread's priority */
	int cnt; /* round-robin count (for scheduler) */
	unsigned char alive_p; /* `eligible for scheduling' flag */
	unsigned char soft_def; /* is this a soft (in-script-defined) method? */

	VALUE branch; /* is this a branch? (for parser) */
	VALUE excobj; /* the most immediate BEGIN object (for exception handling) */

	VALUE callsym; /* the symbol this soft-def'd method will be assigned to */
	VALUE parmsym; /* symbol that parameter passed will be referred to as */
	VALUE arg; /* the actual parameter passed */

	st_table *env_tbl; /* local environment table */
	vector_t *obj_stk; /* locally-defined objects (free()'d when thread is deallocated) */
	vector_t *self_stk; /* stack of receiving objects */
#ifdef SYMBOL_CACHE
	vector_t *modified_syms;
#endif

	vector_t *env_stk;
	vector_t *tok_stk;
	vector_t *stack;
} Thread;



#endif
