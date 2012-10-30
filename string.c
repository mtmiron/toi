/*
 * string.c:
 *
 * 	Implements the String class and its methods.
 *
 */

#include "toi.h"


VALUE cString;




void
string_reap(VALUE str)
{
	check_type(str, T_STRING);

	dstr_free(STRING(str)->ptr);
}


char
string_c_last_char(VALUE str)
{
	return dstr_last_char(STRING(str)->ptr);
}


VALUE
string_new(char *cstring)
{
	VALUE str;
	dynstring_t *dstr;

	str = NEW_OBJ(String);
	OBJ_SETUP(str, T_STRING);
	BASIC(str).klass = cString;
	BASIC(str).reap = string_reap;

	if (!cstring)
		dstr = dstr_new("");
	else
		dstr = dstr_new(cstring);

	if (!dstr)
		return Qnil;

	STRING(str)->ptr = dstr;
	return (VALUE)str;
}


VALUE
string_new_with_size(char *str, int len)
{
	char buf[65536];
	int i;

	for (i = 0; (i < sizeof(buf)) && (i < len); i++)
		buf[i] = str[i];

	buf[i] = '\0';
	return string_new(buf);
}


VALUE
string_to_s(VALUE recv)
{
	return recv;
}


char *
str2cstr(VALUE tstr)
{
	char *cstr;
	dynstring_t *dstr;

	check_type(tstr, T_STRING);

	dstr = STRING(tstr)->ptr;
	cstr = dstrval(dstr);

	return cstr;
}


VALUE
string_length(VALUE recv)
{
	int len;

	len = dstrlen(STRING(recv)->ptr);
	return (VALUE)integer_new(len);
}


VALUE
string_to_i(VALUE self)
{
	char *ptr;

	ptr = str2cstr(self);
	return integer_new(atoi(ptr));
}


VALUE
string_to_f(VALUE self)
{
	char *ptr;

	ptr = str2cstr(self);
	return float_new(atof(ptr));
}


VALUE
string_to_num(VALUE self)
{
	int i, len;
	char *ptr;

	ptr = str2cstr(self);
	len = string_length(self);

	if ((ptr[0] == '+') || (ptr[0] == '-'))
		i = 1;
	else
		i = 0;

	for (; i < len; i++)
	{
		if (ptr[i] == '.')
			return string_to_f(self);
		else if (!isdigit(ptr[i]))
			return string_to_i(self);
	}

	return string_to_i(self);
}


VALUE
string_inspect(VALUE self)
{
	char buf[65536];

	if (TYPE(self) == T_CLASS)
		snprintf(buf, sizeof(buf), "StringClass:%p", self);
	else if (TYPE(self) != T_STRING)
		check_type(self, T_STRING);
	else
		snprintf(buf, sizeof(buf), "%c%s%c", '"', dstrval(STRING(self)->ptr), '"');

	return string_new(buf);
}

VALUE
string_plus(VALUE self, VALUE arg)
{
	char buf[65536], *lstr, *rstr;

	lstr = str2cstr(self);
	rstr = str2cstr(toi_funcall(arg, intern("to_s"), 0));

	snprintf(buf, sizeof(buf), "%s%s", lstr, rstr);
	return string_new(buf);
}


VALUE
string_chomp(VALUE self)
{
	dynstring_t *dstr;
	char *str;
	VALUE ret;

	dstr = STRING(self)->ptr;
	str = dstrval(dstr);

	if (str[strlen(str)-1] != '\n')
		return self;

	str = strdup(str);
	while ((str[strlen(str)-1] == '\n') || (str[strlen(str)-1] == '\r'))
		str[strlen(str)-1] = '\0';

	ret = string_new(str);
	free(str);
	return ret;
}


VALUE
string_minus(VALUE self, VALUE subobj)
{
	char *buf, *ptr, *sub;
	int sublen;
	VALUE ret;

	buf = strdup(str2cstr(self));
	sub = str2cstr(subobj);
	sublen = strlen(sub);

	while ( (ptr = strstr(buf, sub)) )
	{
		while ( (*ptr = ptr[sublen]) )
			ptr[sublen] = '\0', ptr++;
	}

	ret = string_new(buf);
	free(buf);
	return ret;
}


VALUE
string_multiply(VALUE self, VALUE num)
{
	int i, mult;
	char *buf, *src;
	VALUE ret;

	mult = funcall(num, "to_i", 0);
	mult = INT(mult)->num;
	src = str2cstr(self);

	buf = malloc(1 + strlen(src) * mult);
	*buf = '\0';
	for (i = 0; i < mult; i++)
		strcat(buf, src);

	ret = string_new(buf);
	free(buf);
	return ret;
}


VALUE
string_new_thunk(VALUE self)
{
	return string_new(NULL);
}


VALUE
string_capitalize(VALUE self)
{
	char *sptr;
	int i;
	VALUE ret;

	check_type(self, T_STRING);
	sptr = malloc( strlen( dstrval( STRING(self)->ptr) ) );
	if (!sptr)
		fail("failed to allocate memory");

	if (strlen(dstrval(STRING(self)->ptr)) == 0)
		return string_new(NULL);

	sptr[0] = toupper(dstrval(STRING(self)->ptr)[0]);
	for (i = 1; i < strlen(dstrval(STRING(self)->ptr)); i++)
		sptr[i] = tolower(dstrval(STRING(self)->ptr)[i]);

	ret = string_new(sptr);
	free(sptr);
	return ret;
}


VALUE
string_downcase(VALUE self)
{
	char *newptr, *oldptr;
	int i, len;
	VALUE ret;

	check_type(self, T_STRING);
	oldptr = dstrval(STRING(self)->ptr);
	len = dstrlen(STRING(self)->ptr);
	if (len == 0)
		return string_new(NULL);

	newptr = malloc(len);
	if (!newptr)
		fail("failed to allocate memory");

	for (i = 0; i < len; i++)
		newptr[i] = tolower(oldptr[i]);

	ret = string_new(newptr);
	free(newptr);
	return ret;
}


VALUE
string_upcase(VALUE self)
{
	char *newptr, *oldptr;
	int i, len;
	VALUE ret;

	check_type(self, T_STRING);
	oldptr = dstrval(STRING(self)->ptr);
	len = dstrlen(STRING(self)->ptr);
	if (len == 0)
		return string_new(NULL);

	newptr = malloc(len);
	if (!newptr)
		fail("failed to allocate memory");

	for (i = 0; i < len; i++)
		newptr[i] = toupper(oldptr[i]);

	ret = string_new(newptr);
	free(newptr);
	return ret;
}


static VALUE
string_split_substr(VALUE self, char *substr)
{
	char *ptr, *str, *tok, *ram;
	int i, len = strlen(substr);
	VALUE ary = ary_new();

	check_type(self, T_STRING);
	str = ram = ptr = tok = strdup(str2cstr(self));
	if (!str)
		fail("failed to allocate memory");

	while ( (ptr = strstr(str, substr)) )
	{
		for (i = 0; i < len; i++)
			ptr[i] = '\0';
		ary_push(ary, string_new(tok));
		tok = &ptr[i];
	}
		
	ary_push(ary, string_new(tok));
	free(ram);
	return ary;
}


VALUE
string_split(VALUE self, int argc, VALUE *argv)
{
	char *sav, *ptr, *tok, delim;
	VALUE ary = ary_new();

	check_type(self, T_STRING);
	if (argc <= 0)
		delim = ' ';
	else
		if (dstrlen(STRING(argv[0])->ptr) > 1)
			return string_split_substr(self, str2cstr(argv[0]));
		else
			delim = *str2cstr(argv[0]);

	sav = tok = ptr = strdup(str2cstr(self));
	if (!ptr)
		fail("failed to allocate memory");
	if (strlen(ptr) == 0)
		return ary;

	if (delim == '\0')
	{
		while (*ptr)
			ary_push(ary, string_new_with_size(ptr++, 1));
		free(sav);
		return ary;
	}

	while (*ptr)
	{
		if (*ptr == delim)
		{
			*ptr = '\0';
			ary_push(ary, string_new(tok));
			tok = (ptr + 1);
		}
		ptr++;
	}

	ary_push(ary, string_new(tok));
	free(sav);
	return ary;
}


VALUE
string_equal(VALUE self, VALUE parm)
{
	char *str1, *str2;

	if (TYPE(parm) != T_STRING)
		return Qfalse;

	str1 = dstrval(STRING(self)->ptr);
	str2 = dstrval(STRING(parm)->ptr);

	if (strcmp(str1, str2) == 0)
		return Qtrue;
	else
		return Qfalse;
}


void
Init_string()
{
	cString = define_class(intern("String"), cObject);

	define_singleton_method(cString, intern("new"), string_new_thunk, 0);

	define_method(cString, intern("new"), string_new_thunk, 0);
	define_method(cString, intern("to_s"), string_to_s, 0);
	define_method(cString, intern("inspect"), string_inspect, 0);
	define_method(cString, intern("to_i"), string_to_i, 0);
	define_method(cString, intern("to_f"), string_to_f, 0);
	define_method(cString, intern("to_num"), string_to_num, 0);
	define_method(cString, intern("+"), string_plus, 1);
	define_method(cString, intern("-"), string_minus, 1);
	define_method(cString, intern("*"), string_multiply, 1);
	define_method(cString, intern("=="), string_equal, 1);

	define_method(cString, intern("length"), string_length, 0);
	define_method(cString, intern("size"), string_length, 0);
	define_method(cString, intern("chomp"), string_chomp, 0);
	define_method(cString, intern("capitalize"), string_capitalize, 0);
	define_method(cString, intern("downcase"), string_downcase, 0);
	define_method(cString, intern("upcase"), string_upcase, 0);
	define_method(cString, intern("split"), string_split, -1);
}
