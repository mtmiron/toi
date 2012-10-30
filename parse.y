%{
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
static int oplist[] = { '\n',
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

static struct { char *str; int code; } keywords[] = {
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

%}


%token tCONSTANT tSYMBOL tINTEGER tFLOAT tSTRING
			tBREAK tELSE tEND tLOOP

%right ' ' '\t' tIF tTHEN tELSE tEND tWHILE tBREAK tNOT tCLASS tDEFINE
       tSYMBOL tBEGIN tRESCUE tFAIL
%left tAND
%left tOR
%left '+' '-'
%left '*' '/'
%left ')' ']' '<' '>' '}'
%right '=' '(' '[' '{'
%right '!' '^' '.'

%%


program:
	/* empty */
	| program stmt	{
		if ($2)
				push_cell($2);
		$$ = $1;
}
;

stmt:
	'\n'				{ $$ = 0; }
	| ';'				{ $$ = 0; }
	| expr '\n'		{ $$ = $1; }//$$ = (TYPE($1)==T_SYMBOL?mcall_new(POP_SELF(), $1, 0):$1); }
	| expr '\r' '\n'		{ $$ = $1;}//$$ = (TYPE($1)==T_SYMBOL?mcall_new(POP_SELF(), $1, 0):$1); }
	| expr ';'		{ $$ = $1;}//$$ = (TYPE($1)==T_SYMBOL?mcall_new(POP_SELF(), $1, 0):$1); }
;

expr:
	tCONSTANT	{ $$ = $1; }
	| tSYMBOL	{ $$ = $1; }
	| tINTEGER	{ $$ = $1; }
	| tFLOAT		{ $$ = $1; }
	| tSTRING	{ $$ = $1; }
	| tBEGIN 	{ parse_begin(); return; }
	| tIF expr	{ parse_if($2); return; }
	| tWHILE expr	{ parse_while($2); return; }
	| tLOOP		{ parse_loop(); return; }
	| tELSE		{ parse_else(); return; }
	| tEND		{ parse_end(); return; }
	| tCLASS expr	{ parse_class($2); return; }
//	| tFAIL 		{ push_cell(mcall_new(POP_SELF(), intern("fail"), 2, string_new(NULL), cException)); return; }
	| expr '=' expr		{ $$ = assign($1, $3); }
	| '\t'		{ $$ = 0; }
	| ' '			{ $$ = 0; }
	| expr ' '	{ $$ = $1; }
	| ' ' expr	{ $$ = $2; }
	| '\t' expr	{ $$ = $2; }
	| tRESCUE 	{ parse_rescue(cException); return; }
	| tRESCUE ' ' expr { parse_rescue($2); return; }
	| expr '(' ')' 	{ $$ = mcall_new(POP_SELF(), $1, 0); }
	| expr ' ' expr	{ $$ = mcall_new(POP_SELF(), $1, 1, $3); }
	| expr '(' expr ')'	{ $$ = mcall_new(POP_SELF(), $1, 1, $3); }
//	| expr tOR expr
//	| expr tAND expr
	| '!' expr			{ $$ = mcall_new(cself, intern("!"), 1, $2); }
	| expr '+' '+'		{ $$ = mcall_new($1, intern("++"), 0); }
	| expr '-' '-'		{ $$ = mcall_new($1, intern("--"), 0); }
	| expr '+' expr	{ $$ = mcall_new($1, intern("+"), 1, $3); }
	| expr '-' expr	{ $$ = mcall_new($1, intern("-"), 1, $3); }
	| expr '/' expr	{ $$ = mcall_new($1, intern("/"), 1, $3); }
	| expr '*' expr	{ $$ = mcall_new($1, intern("*"), 1, $3); }
	| expr '*' '*' expr	{ $$ = mcall_new($1, intern("**"), 1, $4); }
	| expr '+' '=' expr	{ $$ = assign($1, mcall_new($1, intern("+"), 1, $4)); }
	| expr '-' '=' expr	{ $$ = assign($1, mcall_new($1, intern("-"), 1, $4)); }
	| expr '*' '=' expr	{ $$ = assign($1, mcall_new($1, intern("*"), 1, $4)); }
	| expr '/' '=' expr	{ $$ = assign($1, mcall_new($1, intern("/"), 1, $4)); }
	| expr '<' expr		{ $$ = mcall_new($1, intern("<"), 1, $3); }
	| expr '>' expr		{ $$ = mcall_new($1, intern(">"), 1, $3); }
	| expr '<' '=' expr		{ $$ = mcall_new($1, intern("<="), 1, $4); }
	| expr '>' '=' expr		{ $$ = mcall_new($1, intern(">="), 1, $4); }
	| expr '=' '=' expr		{ $$ = mcall_new($1, intern("=="), 1, $4); }
	| expr '.' expr		{ $$ = $3; PUSH_SELF($1); }
	| expr '[' expr ']'	{ $$ = mcall_new($1, intern("[]"), 1, $3); }
	| '(' expr ')'		{ $$ = $2; }
	| tDEFINE tSYMBOL { parse_define($2, 0); return; }
	| tDEFINE tSYMBOL '(' expr ')' { parse_define($2, $4); return; }
;

%%


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
	if (argc > 1)
		istream = fopen(argv[1], "r");
	else
		istream = stdin;

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
