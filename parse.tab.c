/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 1 "parse.y"

#include "toi.h"
#include "node.h"

/* The C data type of tokens (for Bison) */
#define YYSTYPE VALUE

/* Size of the parsing buffer (a token is limited to this length) */
#define LEXBUFSIZE 65536

#define PUSH_SELF(s) do { vector_push(THREAD(cur_thr)->self_stk,(void*)s); THREAD(cur_thr)->recv = cself = s; } while (0)
#define PUSH_NEST(obj) do { vector_push(end_vector, (void*)obj); } while (0)
#define POP_NEST() (vector_pop(end_vector))
#define POP_SELF() pop_self0()
#define NUM_SELF() (vector_length(THREAD(cur_thr)->self_stk))

FILE *istream;
VALUE cself, thr_buf;
static VALUE current_class, pop_self0();
static vector_t *end_vector;

/* Legal "operator" characters (recognized as token delimiters) */
static char oplist[] = {
	';',
	'+',
	'-',
	'*',
	'/',
	'^',
	'(',
	')',
	'=',
	'!',
	'.',
	'[',
	']',
	'=',
	'<',
	'>',
	'{',
	'}',
	'\n',
	'\r',
	' ',
};

#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     tCONSTANT = 258,
     tSYMBOL = 259,
     tINTEGER = 260,
     tFLOAT = 261,
     tSTRING = 262,
     tBREAK = 263,
     tELSE = 264,
     tEND = 265,
     tLOOP = 266,
     tFAIL = 267,
     tRESCUE = 268,
     tBEGIN = 269,
     tDEFINE = 270,
     tCLASS = 271,
     tNOT = 272,
     tWHILE = 273,
     tTHEN = 274,
     tIF = 275,
     tAND = 276,
     tOR = 277
   };
#endif

struct { char *str; VALUE code; } keywords[] = {
	{ "class", tCLASS },
	{ "def", tDEFINE },
	{ "begin", tBEGIN },
	{ "rescue", tRESCUE },
//	{ "fail", tFAIL },
//	{ "throw", tFAIL }, /* synonym for fail */
	{ "if", tIF },
	{ "then", tTHEN },
	{ "else", tELSE },
	{ "end", tEND },
	{ "while", tWHILE },
	{ "loop", tLOOP },
	{ "not", tNOT },
	{ "and", tAND },
	{ "or", tOR },
	{ "!", tNOT },
	{ "&&", tAND },
	{ "||", tOR },
	{ NULL, 0 },
};

static VALUE
branch_new()
{
	VALUE obj;

	obj = NEW_OBJ(Branch);
	OBJ_SETUP(obj, T_BRANCH);
	BASIC(obj).klass = cKernel;

	return obj;
}

static VALUE
begin_new()
{
	VALUE obj;

	obj = NEW_OBJ(TOIException);
	OBJ_SETUP(obj, T_EXCEPTION);
	BASIC(obj).klass = cException;

	EXCEPTION(obj)->thr = cur_thr;
	THREAD(cur_thr)->excobj = obj;

	return obj;
}

static void
end_begin()
{
	VALUE obj;

	obj = THREAD(THREAD(cur_thr)->up)->excobj;
	cur_thr = THREAD(cur_thr)->up;

	THREAD(cur_thr)->excobj = (VALUE)0;
	THREAD(EXCEPTION(obj)->body_thr)->excobj = obj;

	push_cell(obj);
}

static void
parse_rescue(VALUE type)
{
	VALUE exc;

	exc = THREAD(cur_thr)->excobj;
	if (!TEST(type))
		type = cException;

	/* hack to avoid thread_new() setting up-level-thr wrong */
	cur_thr = EXCEPTION(exc)->thr;
	cur_thr = EXCEPTION(exc)->rescue_thr = thread_new();
	THREAD(cur_thr)->excobj = exc;
	EXCEPTION(exc)->catch_type = type;
}


static void
parse_begin()
{
	VALUE exc;

	exc = begin_new();
	EXCEPTION(exc)->thr = cur_thr;

	EXCEPTION(exc)->body_thr = cur_thr = thread_new();
	THREAD(cur_thr)->excobj = exc;
	PUSH_NEST(end_begin);
}

static void
end_if()
{
	if (!THREAD(cur_thr)->branch)
		yyerror("end without opening control statement");
	cur_thr = THREAD(cur_thr)->up;
}

static void
end_class()
{
	current_class = SUPER_CLASS(current_class);
	if (!TEST(current_class))
		current_class = cKernel;
}

static void
end_define()
{
	VALUE new_meth, klass, callsym;

	new_meth = cur_thr;
	cur_thr = THREAD(cur_thr)->up;
	THREAD(new_meth)->up = (VALUE)0;

	klass = current_class;
	if (!klass)
		klass = cKernel;

	callsym = THREAD(new_meth)->callsym;
	if (!callsym)
		fail("fatal bug during parsing: soft-defined method has no symbol");

	define_soft_method(klass, callsym, new_meth);
}

static void
parse_class(VALUE sym)
{
	current_class = define_class(sym, current_class);
	PUSH_NEST(end_class);
}

static void
parse_define(VALUE callsym, VALUE parmsym)
{
	VALUE meth;

	cur_thr = thread_new();
	THREAD(cur_thr)->soft_def = 1;
	THREAD(cur_thr)->callsym = callsym;
	THREAD(cur_thr)->parmsym = parmsym;

	PUSH_NEST(end_define);
}

static VALUE
parse_if(VALUE cond)
{
	VALUE branch;

	branch = branch_new();
	push_cell(branch);
	BRANCH(branch)->cond = cond;
	BRANCH(branch)->cond_true = cur_thr = thread_new();
	THREAD(cur_thr)->branch = branch;
	st_free_table(THREAD(cur_thr)->env_tbl);
	THREAD(cur_thr)->env_tbl = THREAD(THREAD(cur_thr)->up)->env_tbl;

	PUSH_NEST(end_if);
	return branch;
}

static void
parse_while(VALUE cond)
{
	VALUE branch;

	branch = parse_if(cond);
	BRANCH(branch)->loop = Qtrue;
}

static void
parse_loop()
{
	parse_while(Qtrue);
}

static void
parse_else()
{
	if (!THREAD(cur_thr)->branch)
		yyerror("else without opening control statement");
	thr_buf = cur_thr;
	cur_thr = THREAD(cur_thr)->up; /* b/c thread_new() uses cur_thr to set higher-level thr */
	cur_thr = BRANCH(THREAD(thr_buf)->branch)->cond_false = thread_new();
	st_free_table(THREAD(cur_thr)->env_tbl);
	THREAD(cur_thr)->env_tbl = THREAD(thr_buf)->env_tbl;
	THREAD(cur_thr)->branch = THREAD(thr_buf)->branch;
}

static void
parse_end()
{
	void (*f)();

	f = POP_NEST();
	if (!f)
		yyerror("end without opening control statement");

	f();
}


/* Line 371 of yacc.c  */
#line 350 "parse.tab.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     tCONSTANT = 258,
     tSYMBOL = 259,
     tINTEGER = 260,
     tFLOAT = 261,
     tSTRING = 262,
     tBREAK = 263,
     tELSE = 264,
     tEND = 265,
     tLOOP = 266,
     tFAIL = 267,
     tRESCUE = 268,
     tBEGIN = 269,
     tDEFINE = 270,
     tCLASS = 271,
     tNOT = 272,
     tWHILE = 273,
     tTHEN = 274,
     tIF = 275,
     tAND = 276,
     tOR = 277
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 435 "parse.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   529

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  44
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  4
/* YYNRULES -- Number of rules.  */
#define YYNRULES  53
/* YYNRULES -- Number of states.  */
#define YYNSTATES  86

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   277

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,    13,
      41,     2,     2,    43,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,    12,    38,     2,     2,     2,     2,     2,     2,
      35,    29,    27,    25,     2,    26,    40,    28,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    42,
      31,    34,    32,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    36,     2,    30,    39,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    37,     2,    33,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    14,    18,    21,
      23,    25,    27,    29,    31,    33,    36,    39,    41,    43,
      45,    48,    52,    54,    56,    59,    62,    65,    67,    71,
      75,    79,    84,    87,    91,    95,    99,   103,   107,   111,
     116,   121,   126,   131,   136,   140,   144,   149,   154,   159,
     163,   168,   172,   175
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      45,     0,    -1,    -1,    45,    46,    -1,    41,    -1,    42,
      -1,    47,    41,    -1,    47,    43,    41,    -1,    47,    42,
      -1,     3,    -1,     4,    -1,     5,    -1,     6,    -1,     7,
      -1,    16,    -1,    22,    47,    -1,    20,    47,    -1,    11,
      -1,     9,    -1,    10,    -1,    18,    47,    -1,    47,    34,
      47,    -1,    13,    -1,    12,    -1,    47,    12,    -1,    12,
      47,    -1,    13,    47,    -1,    15,    -1,    15,    12,    47,
      -1,    47,    35,    29,    -1,    47,    12,    47,    -1,    47,
      35,    47,    29,    -1,    38,    47,    -1,    47,    25,    25,
      -1,    47,    26,    26,    -1,    47,    25,    47,    -1,    47,
      26,    47,    -1,    47,    28,    47,    -1,    47,    27,    47,
      -1,    47,    27,    27,    47,    -1,    47,    25,    34,    47,
      -1,    47,    26,    34,    47,    -1,    47,    27,    34,    47,
      -1,    47,    28,    34,    47,    -1,    47,    31,    47,    -1,
      47,    32,    47,    -1,    47,    31,    34,    47,    -1,    47,
      32,    34,    47,    -1,    47,    34,    34,    47,    -1,    47,
      40,    47,    -1,    47,    36,    47,    30,    -1,    35,    47,
      29,    -1,    17,     4,    -1,    17,     4,    35,    47,    29,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   300,   300,   302,   310,   311,   312,   313,   314,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     340,   341,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "tCONSTANT", "tSYMBOL", "tINTEGER",
  "tFLOAT", "tSTRING", "tBREAK", "tELSE", "tEND", "tLOOP", "' '", "'\\t'",
  "tFAIL", "tRESCUE", "tBEGIN", "tDEFINE", "tCLASS", "tNOT", "tWHILE",
  "tTHEN", "tIF", "tAND", "tOR", "'+'", "'-'", "'*'", "'/'", "')'", "']'",
  "'<'", "'>'", "'}'", "'='", "'('", "'['", "'{'", "'!'", "'^'", "'.'",
  "'\\n'", "';'", "'\\r'", "$accept", "program", "stmt", "expr", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,    32,     9,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,    43,    45,    42,    47,    41,
      93,    60,    62,   125,    61,    40,    91,   123,    33,    94,
      46,    10,    59,    13
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    44,    45,    45,    46,    46,    46,    46,    46,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     2,     3,     2,     1,
       1,     1,     1,     1,     1,     2,     2,     1,     1,     1,
       2,     3,     1,     1,     2,     2,     2,     1,     3,     3,
       3,     4,     2,     3,     3,     3,     3,     3,     3,     4,
       4,     4,     4,     4,     3,     3,     4,     4,     4,     3,
       4,     3,     2,     5
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     9,    10,    11,    12,    13,    18,    19,
      17,    23,    22,    27,    14,     0,     0,     0,     0,     0,
       0,     4,     5,     3,     0,    25,    26,     0,    52,    20,
      16,    15,     0,    32,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     6,     8,     0,    28,     0,
      51,    30,    33,     0,    35,    34,     0,    36,     0,     0,
      38,     0,    37,     0,    44,     0,    45,     0,    21,    29,
       0,     0,    49,     7,     0,    40,    41,    39,    42,    43,
      46,    47,    48,    31,    50,    53
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    23,    24
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -32
static const yytype_int16 yypact[] =
{
     -32,    55,   -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,
     -32,   383,   383,   -10,   -32,    -1,   383,   383,   383,   383,
     383,   -32,   -32,   -32,   397,   489,   489,   383,   -31,   489,
     489,   489,   418,   -28,   383,    95,   131,   167,   203,   239,
     275,   311,   347,   383,   383,   -32,   -32,   -24,   489,   383,
     -32,   489,   -32,   383,    51,   -32,   383,    51,   383,   383,
     -21,   383,   -21,   383,     0,   383,     0,   383,     0,   -32,
     436,   454,   -28,   -32,   471,     0,     0,   -21,     0,     0,
       0,     0,     0,   -32,   -32,   -32
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -32,   -32,   -32,   -11
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      25,    26,    27,    28,    49,    29,    30,    31,    32,    33,
      39,    40,    44,    41,    42,    43,    48,    73,     0,    44,
       0,     0,     0,    51,    54,    57,    60,    62,    64,    66,
      68,    70,    71,    72,    41,    42,    43,     0,    74,     0,
      44,     0,    75,     0,     0,    76,     0,    77,    78,     0,
      79,     0,    80,     0,    81,     2,    82,     0,     3,     4,
       5,     6,     7,     0,     8,     9,    10,    11,    12,     0,
      13,    14,    15,    16,     0,    17,     0,    18,    37,    38,
       0,     0,    39,    40,     0,    41,    42,    43,     0,     0,
      19,    44,     0,    20,     0,     0,    21,    22,     3,     4,
       5,     6,     7,     0,     8,     9,    10,    11,    12,     0,
      13,    14,    15,    16,     0,    17,     0,    18,     0,     0,
      52,     0,     0,     0,     0,     0,     0,     0,     0,    53,
      19,     0,     0,    20,     3,     4,     5,     6,     7,     0,
       8,     9,    10,    11,    12,     0,    13,    14,    15,    16,
       0,    17,     0,    18,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,    56,    19,     0,     0,    20,
       3,     4,     5,     6,     7,     0,     8,     9,    10,    11,
      12,     0,    13,    14,    15,    16,     0,    17,     0,    18,
       0,     0,     0,     0,    58,     0,     0,     0,     0,     0,
       0,    59,    19,     0,     0,    20,     3,     4,     5,     6,
       7,     0,     8,     9,    10,    11,    12,     0,    13,    14,
      15,    16,     0,    17,     0,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    61,    19,     0,
       0,    20,     3,     4,     5,     6,     7,     0,     8,     9,
      10,    11,    12,     0,    13,    14,    15,    16,     0,    17,
       0,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,    19,     0,     0,    20,     3,     4,
       5,     6,     7,     0,     8,     9,    10,    11,    12,     0,
      13,    14,    15,    16,     0,    17,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    65,
      19,     0,     0,    20,     3,     4,     5,     6,     7,     0,
       8,     9,    10,    11,    12,     0,    13,    14,    15,    16,
       0,    17,     0,    18,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    67,    19,     0,     0,    20,
       3,     4,     5,     6,     7,     0,     8,     9,    10,    11,
      12,     0,    13,    14,    15,    16,     0,    17,     0,    18,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,     0,    19,     0,     0,    20,     3,     4,     5,     6,
       7,     0,     8,     9,    10,    11,    12,     0,    13,    14,
      15,    16,     0,    17,     0,    18,     0,     0,     0,    34,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,    20,    35,    36,    37,    38,     0,     0,    39,    40,
      34,    41,    42,    43,     0,     0,     0,    44,    45,    46,
      47,     0,     0,    35,    36,    37,    38,    50,    34,    39,
      40,     0,    41,    42,    43,     0,     0,     0,    44,     0,
       0,    35,    36,    37,    38,    83,    34,    39,    40,     0,
      41,    42,    43,     0,     0,     0,    44,     0,     0,    35,
      36,    37,    38,    34,    84,    39,    40,     0,    41,    42,
      43,     0,     0,     0,    44,     0,    35,    36,    37,    38,
      85,    34,    39,    40,     0,    41,    42,    43,     0,     0,
       0,    44,     0,     0,    35,    36,    37,    38,     0,     0,
      39,    40,     0,    41,    42,    43,     0,     0,     0,    44
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-32)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int8 yycheck[] =
{
      11,    12,    12,     4,    35,    16,    17,    18,    19,    20,
      31,    32,    40,    34,    35,    36,    27,    41,    -1,    40,
      -1,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    34,    35,    36,    -1,    49,    -1,
      40,    -1,    53,    -1,    -1,    56,    -1,    58,    59,    -1,
      61,    -1,    63,    -1,    65,     0,    67,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      15,    16,    17,    18,    -1,    20,    -1,    22,    27,    28,
      -1,    -1,    31,    32,    -1,    34,    35,    36,    -1,    -1,
      35,    40,    -1,    38,    -1,    -1,    41,    42,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      15,    16,    17,    18,    -1,    20,    -1,    22,    -1,    -1,
      25,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,
      35,    -1,    -1,    38,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    18,
      -1,    20,    -1,    22,    -1,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    34,    35,    -1,    -1,    38,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    15,    16,    17,    18,    -1,    20,    -1,    22,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    34,    35,    -1,    -1,    38,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    15,    16,
      17,    18,    -1,    20,    -1,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    35,    -1,
      -1,    38,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    15,    16,    17,    18,    -1,    20,
      -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    34,    35,    -1,    -1,    38,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      15,    16,    17,    18,    -1,    20,    -1,    22,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,
      35,    -1,    -1,    38,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    18,
      -1,    20,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    34,    35,    -1,    -1,    38,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    15,    16,    17,    18,    -1,    20,    -1,    22,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    38,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    15,    16,
      17,    18,    -1,    20,    -1,    22,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    38,    25,    26,    27,    28,    -1,    -1,    31,    32,
      12,    34,    35,    36,    -1,    -1,    -1,    40,    41,    42,
      43,    -1,    -1,    25,    26,    27,    28,    29,    12,    31,
      32,    -1,    34,    35,    36,    -1,    -1,    -1,    40,    -1,
      -1,    25,    26,    27,    28,    29,    12,    31,    32,    -1,
      34,    35,    36,    -1,    -1,    -1,    40,    -1,    -1,    25,
      26,    27,    28,    12,    30,    31,    32,    -1,    34,    35,
      36,    -1,    -1,    -1,    40,    -1,    25,    26,    27,    28,
      29,    12,    31,    32,    -1,    34,    35,    36,    -1,    -1,
      -1,    40,    -1,    -1,    25,    26,    27,    28,    -1,    -1,
      31,    32,    -1,    34,    35,    36,    -1,    -1,    -1,    40
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    45,     0,     3,     4,     5,     6,     7,     9,    10,
      11,    12,    13,    15,    16,    17,    18,    20,    22,    35,
      38,    41,    42,    46,    47,    47,    47,    12,     4,    47,
      47,    47,    47,    47,    12,    25,    26,    27,    28,    31,
      32,    34,    35,    36,    40,    41,    42,    43,    47,    35,
      29,    47,    25,    34,    47,    26,    34,    47,    27,    34,
      47,    34,    47,    34,    47,    34,    47,    34,    47,    29,
      47,    47,    47,    41,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    29,    30,    29
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
/* Line 1792 of yacc.c  */
#line 302 "parse.y"
    {
		if ((yyvsp[(2) - (2)]))
				push_cell((yyvsp[(2) - (2)]));
		(yyval) = (yyvsp[(1) - (2)]);
}
    break;

  case 4:
/* Line 1792 of yacc.c  */
#line 310 "parse.y"
    { (yyval) = 0; }
    break;

  case 5:
/* Line 1792 of yacc.c  */
#line 311 "parse.y"
    { (yyval) = 0; }
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 312 "parse.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 7:
/* Line 1792 of yacc.c  */
#line 313 "parse.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 314 "parse.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 9:
/* Line 1792 of yacc.c  */
#line 318 "parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 10:
/* Line 1792 of yacc.c  */
#line 319 "parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 320 "parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 321 "parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 322 "parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 323 "parse.y"
    { parse_begin(); return; }
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 324 "parse.y"
    { parse_if((yyvsp[(2) - (2)])); return; }
    break;

  case 16:
/* Line 1792 of yacc.c  */
#line 325 "parse.y"
    { parse_while((yyvsp[(2) - (2)])); return; }
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 326 "parse.y"
    { parse_loop(); return; }
    break;

  case 18:
/* Line 1792 of yacc.c  */
#line 327 "parse.y"
    { parse_else(); return; }
    break;

  case 19:
/* Line 1792 of yacc.c  */
#line 328 "parse.y"
    { parse_end(); return; }
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 329 "parse.y"
    { parse_class((yyvsp[(2) - (2)])); return; }
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 331 "parse.y"
    { (yyval) = assign((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 332 "parse.y"
    { (yyval) = 0; }
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 333 "parse.y"
    { (yyval) = 0; }
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 334 "parse.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 335 "parse.y"
    { (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 26:
/* Line 1792 of yacc.c  */
#line 336 "parse.y"
    { (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 337 "parse.y"
    { parse_rescue(cException); return; }
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 338 "parse.y"
    { parse_rescue((yyvsp[(2) - (3)])); return; }
    break;

  case 29:
/* Line 1792 of yacc.c  */
#line 339 "parse.y"
    { (yyval) = mcall_new(POP_SELF(), (yyvsp[(1) - (3)]), 0); }
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 340 "parse.y"
    { (yyval) = mcall_new(POP_SELF(), (yyvsp[(1) - (3)]), 1, (yyvsp[(3) - (3)])); }
    break;

  case 31:
/* Line 1792 of yacc.c  */
#line 341 "parse.y"
    { (yyval) = mcall_new(POP_SELF(), (yyvsp[(1) - (4)]), 1, (yyvsp[(3) - (4)])); }
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 344 "parse.y"
    { (yyval) = mcall_new(cself, intern("!"), 1, (yyvsp[(2) - (2)])); }
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 345 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (3)]), intern("++"), 0); }
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 346 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (3)]), intern("--"), 0); }
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 347 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (3)]), intern("+"), 1, (yyvsp[(3) - (3)])); }
    break;

  case 36:
/* Line 1792 of yacc.c  */
#line 348 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (3)]), intern("-"), 1, (yyvsp[(3) - (3)])); }
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 349 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (3)]), intern("/"), 1, (yyvsp[(3) - (3)])); }
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 350 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (3)]), intern("*"), 1, (yyvsp[(3) - (3)])); }
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 351 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (4)]), intern("**"), 1, (yyvsp[(4) - (4)])); }
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 352 "parse.y"
    { (yyval) = assign((yyvsp[(1) - (4)]), mcall_new((yyvsp[(1) - (4)]), intern("+"), 1, (yyvsp[(4) - (4)]))); }
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 353 "parse.y"
    { (yyval) = assign((yyvsp[(1) - (4)]), mcall_new((yyvsp[(1) - (4)]), intern("-"), 1, (yyvsp[(4) - (4)]))); }
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 354 "parse.y"
    { (yyval) = assign((yyvsp[(1) - (4)]), mcall_new((yyvsp[(1) - (4)]), intern("*"), 1, (yyvsp[(4) - (4)]))); }
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 355 "parse.y"
    { (yyval) = assign((yyvsp[(1) - (4)]), mcall_new((yyvsp[(1) - (4)]), intern("/"), 1, (yyvsp[(4) - (4)]))); }
    break;

  case 44:
/* Line 1792 of yacc.c  */
#line 356 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (3)]), intern("<"), 1, (yyvsp[(3) - (3)])); }
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 357 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (3)]), intern(">"), 1, (yyvsp[(3) - (3)])); }
    break;

  case 46:
/* Line 1792 of yacc.c  */
#line 358 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (4)]), intern("<="), 1, (yyvsp[(4) - (4)])); }
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 359 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (4)]), intern(">="), 1, (yyvsp[(4) - (4)])); }
    break;

  case 48:
/* Line 1792 of yacc.c  */
#line 360 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (4)]), intern("=="), 1, (yyvsp[(4) - (4)])); }
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 361 "parse.y"
    { (yyval) = (yyvsp[(3) - (3)]); PUSH_SELF((yyvsp[(1) - (3)])); }
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 362 "parse.y"
    { (yyval) = mcall_new((yyvsp[(1) - (4)]), intern("[]"), 1, (yyvsp[(3) - (4)])); }
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 363 "parse.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 364 "parse.y"
    { parse_define((yyvsp[(2) - (2)]), 0); return; }
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 365 "parse.y"
    { parse_define((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); return; }
    break;


/* Line 1792 of yacc.c  */
#line 2096 "parse.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


/* Line 2055 of yacc.c  */
#line 368 "parse.y"



int
yyerror(char *s)
{
	char buf[LEXBUFSIZE];
	int ln, slen, err, pos, endpos;

	/* print the error message */
	fprintf(stderr, "%s:", s);

	/* store the position in question and rewind the stream */
	err = ftell(istream) - 1;
	rewind(istream);

	/* count lines until we reach the error line */
	for (pos = 0, ln = 1; pos < err; pos++)
	{
		if (getc(istream) == '\n')
		{
			ln++;
			/* save this position (beginning of error line) */
			endpos = ftell(istream);
		}
	}
	/* seek to the error line & read it in */
	fseek(istream, endpos, SEEK_SET);
	fgets(buf, sizeof(buf), istream);

	/* chomp off the newline(s) */
	slen = strlen(buf) - 1;
	while ((buf[slen] == '\n') || (buf[slen] == '\r'))
		buf[slen--] = '\0';

	/* print the line #, copy of the script line, and a pointer to the error */
	fprintf(stderr, " on line %d:\n%s\n %*s\n", ln, buf, pos - endpos, "^");

	exit(1);
}


VALUE
push_cell(VALUE cell)
{
	VALUE thr;
	vector_t *stk;

	thr = cur_thr;
	stk = THREAD(thr)->stack;

	vector_push(stk, (void*)cell);
#ifdef TOIDEBUG
	fprintf(stderr, "cell: %s\n", str2cstr(funcall(cell,"inspect",0)));
#endif
	return thr;
}


static VALUE
pop_self0()
{
	if (NUM_SELF() == 0)
		THREAD(cur_thr)->recv = cself = (VALUE)THREAD(THREAD(cur_thr)->up)->recv;
	else
		THREAD(cur_thr)->recv = cself = (vector_length(THREAD(cur_thr)->self_stk) > 1 ? (VALUE)vector_pop(THREAD(cur_thr)->self_stk) : (VALUE)vector_first(THREAD(cur_thr)->self_stk));
	return cself;
}


/* if this char is an operator, return it; otherwise return 0 */
static char
toi_token_op(char c)
{
	int i;
	static int len = sizeof(oplist);

	for (i = 0; i < len; i++)
	{
		if (c == oplist[i])
			return c;
	}

	return (char)0;
}


/* check what kind of token this string represents */
static int
toi_token_type(char *str)
{
	int i = 0, len = strlen(str);

	if (!len) /* empty string */
		return tSTRING;
	if (toi_token_op(str[0]) && (str[0] != '-'))
		return (int)str[0];
	for (i = 0; keywords[i].str != NULL; i++)
	{
		if (strcmp(str, keywords[i].str) == 0)
			return keywords[i].code;
	}

	if (isdigit(str[0]) || (str[0] == '-'))
	{
		if (strchr(str, '.'))
		{
			return tFLOAT;
		}
		else
		{
			return tINTEGER;
		}
	}
	else if (!isalpha(str[0]))
	{
		if (str[0] == '"')
		{
			return tSTRING;
		}
		else
		{
			return tSYMBOL;
		}
	}
	else
	{
		return tSYMBOL;
	}
}

static VALUE
parse_string(char *buf, int i, char c)
{
	int strerr, strc;

	/* save opening char and read till we find its matching close */
	strc = c;
	strerr = ftell(istream); /* incase we hit EOF, save start pos */
	while ((c = getc(istream)) != EOF)
	{
		if (c == strc)
		{
			yylval = string_new(buf);
			BASIC(yylval).refcount = GC_NEVER_FREE;
			return tSTRING;
		}

		/* handle embedded escape character */
		if (c == '\\')
		{
			c = getc(istream);
			switch (c)
			{
				case 't': c = '\t'; break;
				case 'n': c = '\n'; break;
				case 'r': c = '\r'; break;
				case 'b': c = '\b'; break;
				case 's': c = ' '; break;
				default: break;
			}
		}

		buf[i++] = c;
		if (i >= LEXBUFSIZE)
		{
			yyerror("maximum allowed token size exceeded: imminent buffer overrun in yylex()");
			abort();
		}
	}
	fseek(istream, strerr, SEEK_SET); /* seek to string's start pos for yyerror() */
	yyerror("parse error: unterminated string meets end of file");

	return Qnil; /* not reached */
}


/*
 * yylex():
 *
 * 	Repeatedly called while parsing to obtain the next token.
 *
 */
int
yylex()
{
	char buf[LEXBUFSIZE] = { '\0' };
	char c, strc;
	int type, strerr;
	register int i = 0;

	while ((c = getc(istream)) != EOF)
	{
		/* string */
		if ((c == '"') || (c == '\''))
		{
			return parse_string(buf, i, c);
		}

		/* alphanumeric character, or '_', or '?' (legal symbol chars) */
		else if (isalnum(c) || (c == '_') || (c == '?'))
			buf[i++] = c;

		/* comment */
		else if (c == '#')
		{
			fgets(&buf[i], LEXBUFSIZE - i, istream);
			buf[i] = '\0';
			ungetc('\n', istream);
			continue;
		}

		/* it's a delimiter (space or an operator) */
		else if (isspace(c) || toi_token_op(c))
		{
			if (i == 0)
			{
				/* unary minus check */
				if (c == '-')
				{
					buf[i++] = c;
					c = getc(istream);
					if (isdigit(c))
					{
						buf[i++] = c;
						continue;
					}
					else
					{
						ungetc(c,istream);
						return '-';
					}
				}
				else
				{
					return c;
				}
			}

			switch (toi_token_type(buf))
			{
				case tSTRING:
					yylval = string_new(&buf[1]);
					BASIC(yylval).refcount = GC_NEVER_FREE;
					type = tSTRING;
					break;
				case tSYMBOL:
					yylval = intern(buf);
					BASIC(yylval).refcount = GC_NEVER_FREE;
					type = tSYMBOL;
					break;
				case tFLOAT:
					yylval = float_new(atof(buf));
					BASIC(yylval).refcount = GC_NEVER_FREE;
					type = tFLOAT;
					break;
				case tINTEGER:
					/* check the next char (could be a method call or a float) */
					if (c == '.')
					{
						c = getc(istream);
						if (isdigit(c))
						{
							ungetc(c,istream);
							buf[i++] = '.';
							continue;
						}
					}
					yylval = integer_new(atoi(buf));
					BASIC(yylval).refcount = GC_NEVER_FREE;
					type = tINTEGER;
					break;
				case tIF:
				case tELSE:
				case tEND:
				case tWHILE:
				case tLOOP:
				case tDEFINE:
				case tCLASS:
				case tBEGIN:
				case tRESCUE:
				case tFAIL:
					/* fall through */
					return toi_token_type(buf);
				default:
					type = (int)c;
					fprintf(stderr, "warning: unrecognized token type (%d)\n", type);
					break;
			}

			ungetc(c,istream);
			return type;
		}

		if (i >= LEXBUFSIZE)
			fail("yylex(): single-token-string too large to parse (about to overflow local buffer)");
	}

	if (c == EOF)
		push_cell(mcall_new(main_thread(), intern("exit"), 0));
	return -1;
}


static void
parser_dealloc()
{
	vector_free(end_vector);
}


/*
 * main():
 *
 * 	Kick everything off.
 *
 */
int
main(int argc, char **argv)
{
	int ip, ret;
	VALUE cell;
	vector_t *stk;

	/* were we run with a file parameter? */
	if (argc > 1 && *argv[1] == '-') {
        switch (*(argv[1]+1)) {
        case 'h':
            printf("Usage: %s [file]\n", argv[0]);
        default:
            return 1;
        }
    }
    else if (argc > 1) {
		istream = fopen(argv[1], "r");
    }
	else {
		istream = stdin;
    }

	/* report & abort if we have no I/O stream */
	if (!istream)
		fail(strerror(errno));

	/* init the interpreter */
	toi_init(argc, argv);

	/* define top-level self */
	PUSH_SELF(cself);
	end_vector = vector_new();

	/* keep parsing the input until we reach EOF */
	do {
		ret = yyparse();
	} while (ret);

	/* deallocate parser mem */
	parser_dealloc();
	if (istream != stdin)
		fclose(istream);

	/* evaluate the main instruction stack, one cell at a time */
	while (1)
	{
		ip = THREAD(cur_thr)->ip; /* instruction pointer */
		stk = THREAD(cur_thr)->stack; /* stack */
		cell = (VALUE)vector_aref(stk, ip); /* current cell (instruction) */

		/* incase this thread has reached the end of its stack */
		if (!cell)
		{
			cur_thr = THREAD(cur_thr)->up;
			if (!cur_thr) /* has to be the main thread */
				break;
			continue;
		}
		
		eval_cell(cell);
		THREAD(cur_thr)->ip++;
	}

	return ret;
}
