#include "toi.h"


VALUE cInteger;


VALUE
integer_new(int num)
{
	VALUE i;

	i = NEW_OBJ(Integer);
	OBJ_SETUP(i, T_INTEGER);
	BASIC(i).klass = cInteger;
	INTEGER(i)->num = num;

	return (VALUE)i;
}


VALUE
integer_s_new(VALUE recv, int num)
{
	return integer_new(num);
}


VALUE
integer_to_s(VALUE recv)
{
	char cstr[16] = { '\0' };
#ifdef __cplusplus
	sprintf(cstr, "%d", INTEGER(recv)->num);
#else
	snprintf(cstr, sizeof(cstr) * sizeof(char), "%d", INTEGER(recv)->num);
#endif
	return string_new(cstr);
}

VALUE
integer_multiply(VALUE recv, VALUE arg)
{
	if (TYPE(arg) == T_STRING)
		arg = toi_funcall(arg, intern("to_num"), 0);

	if (TYPE(arg) == T_FLOAT)
		return float_new(INTEGER(recv)->num * FLOAT(arg)->num);
	else if (TYPE(arg) == T_INTEGER)
		return integer_new(INTEGER(recv)->num * INTEGER(arg)->num);
	else
		return integer_multiply(recv, funcall(arg, "to_i", 0));
}

VALUE
integer_divide(VALUE recv, VALUE arg)
{
	if (TYPE(arg) == T_STRING)
		arg = toi_funcall(arg, intern("to_num"), 0);

	if (TYPE(arg) == T_FLOAT)
		return float_new(INTEGER(recv)->num / FLOAT(arg)->num);
	else
		return integer_new(INTEGER(recv)->num / INTEGER(arg)->num);
}

VALUE
integer_addition(VALUE recv, VALUE arg)
{
	if (TYPE(arg) == T_STRING)
	{
		recv = toi_funcall(recv, intern("to_s"), 0);
		return toi_funcall(recv, intern("+"), 1, arg);
	}

	if (TYPE(arg) == T_FLOAT)
		return float_new(INTEGER(recv)->num + FLOAT(arg)->num);
	else
		return integer_new(INTEGER(recv)->num + INTEGER(arg)->num);
}

VALUE
integer_subtract(VALUE recv, VALUE arg)
{
	if (TYPE(arg) == T_STRING)
		arg = toi_funcall(arg, intern("to_num"), 0);

	if (TYPE(arg) == T_FLOAT)
		return float_new(INTEGER(recv)->num - FLOAT(arg)->num);
	else
		return integer_new(INTEGER(recv)->num - INTEGER(arg)->num);
}

VALUE
integer_pow(VALUE recv, VALUE arg)
{
	if (TYPE(arg) == T_STRING)
		arg = toi_funcall(arg, intern("to_num"), 0);

	if (TYPE(arg) == T_FLOAT)
		return float_new(powf((float)INTEGER(recv)->num, FLOAT(arg)->num));
	else
		return integer_new((int)(powf((float)INTEGER(recv)->num, (float)INTEGER(arg)->num)));
}

VALUE
integer_to_i(VALUE self)
{
	return self;
}

VALUE
integer_to_f(VALUE self)
{
	return float_new((float)INT(self)->num);
}

VALUE
integer_less_than(VALUE self, VALUE arg)
{
	if (TYPE(arg) == T_FLOAT)
	{
		if (INT(self)->num < FLOAT(arg)->num)
			return Qtrue;
		else
			return Qfalse;
	}

	check_type(arg, T_INTEGER);
	if (INT(self)->num < INT(arg)->num)
		return Qtrue;
	else
		return Qfalse;
}

VALUE
integer_greater_than(VALUE self, VALUE arg)
{
	if (TYPE(arg) == T_FLOAT)
	{
		if (INT(self)->num > FLOAT(arg)->num)
			return Qtrue;
		else
			return Qfalse;
	}

	check_type(arg, T_INTEGER);
	if (INT(self)->num > INT(arg)->num)
		return Qtrue;
	else
		return Qfalse;
}

VALUE
integer_less_or_equal(VALUE self, VALUE arg)
{
	if (TYPE(arg) == T_FLOAT)
	{
		if (INT(self)->num <= FLOAT(arg)->num)
			return Qtrue;
		else
			return Qfalse;
	}

	check_type(arg, T_INTEGER);
	if (INT(self)->num <= INT(arg)->num)
		return Qtrue;
	else
		return Qfalse;
}

VALUE
integer_greater_or_equal(VALUE self, VALUE arg)
{
	if (TYPE(arg) == T_FLOAT)
	{
		if (INT(self)->num >= FLOAT(arg)->num)
			return Qtrue;
		else
			return Qfalse;
	}

	check_type(arg, T_INTEGER);
	if (INT(self)->num >= INT(arg)->num)
		return Qtrue;
	else
		return Qfalse;
}

VALUE
integer_equal(VALUE self, VALUE arg)
{
	if (TYPE(arg) == T_FLOAT)
	{
		if (INT(self)->num == FLOAT(arg)->num)
			return Qtrue;
		else
			return Qfalse;
	}

	check_type(arg, T_INTEGER);
	if (INT(self)->num == INT(arg)->num)
		return Qtrue;
	else
		return Qfalse;
}

VALUE
integer_times(VALUE self, VALUE arg)
{
	return Qnil;
}

VALUE
integer_increment(VALUE self)
{
	INT(self)->num++;
	return self;
}

VALUE
integer_decrement(VALUE self)
{
	INT(self)->num--;
	return self;
}

void
Init_integer()
{
	cInteger = define_class(intern("Integer"), cNumeric);

	define_singleton_method(cInteger, intern("new"), integer_s_new, 1);

	define_method(cInteger, intern("*"), integer_multiply, 1);
	define_method(cInteger, intern("/"), integer_divide, 1);
	define_method(cInteger, intern("+"), integer_addition, 1);
	define_method(cInteger, intern("-"), integer_subtract, 1);
	define_method(cInteger, intern("**"), integer_pow, 1);

	define_method(cInteger, intern("++"), integer_increment, 0);
	define_method(cInteger, intern("--"), integer_decrement, 0);

	define_method(cInteger, intern("<"), integer_less_than, 1);
	define_method(cInteger, intern(">"), integer_greater_than, 1);
	define_method(cInteger, intern("<="), integer_less_or_equal, 1);
	define_method(cInteger, intern(">="), integer_greater_or_equal, 1);
	define_method(cInteger, intern("=="), integer_equal, 1);

	define_method(cInteger, intern("to_s"), integer_to_s, 0);
	define_method(cInteger, intern("to_i"), integer_to_i, 0);
	define_method(cInteger, intern("to_f"), integer_to_f, 0);

	define_method(cInteger, intern("times"), integer_times, 1);
}
