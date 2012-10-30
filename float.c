#include "toi.h"
#include <math.h>


VALUE cFloat;


VALUE
float_new(float num)
{
	VALUE obj;

	obj = NEW_OBJ(Float);
	OBJ_SETUP(obj, T_FLOAT);
	BASIC(obj).klass = cFloat;
	FLOAT(obj)->num = num;

	return (VALUE)obj;
}


VALUE
float_s_new(VALUE recv, float num)
{
	return float_new(num);
}


VALUE
float_to_s(VALUE recv)
{
	char cstr[256] = { '\0' };

#ifdef __cplusplus
	sprintf(cstr, "%#f", FLOAT(recv)->num);
#else
	snprintf(cstr, sizeof(char)*sizeof(cstr),
	         "%#f", FLOAT(recv)->num);
#endif
	return (VALUE)string_new(cstr);
}


static float
num2float(VALUE arg)
{
	if (TYPE(arg) == T_FLOAT)
		return FLOAT(arg)->num;
	else
		return (float)INTEGER(arg)->num;
}

VALUE
float_multiply(VALUE recv, VALUE arg)
{
	return float_new(FLOAT(recv)->num * num2float(arg));
}

VALUE
float_subtract(VALUE recv, VALUE arg)
{
	return float_new(FLOAT(recv)->num - num2float(arg));
}

VALUE
float_addition(VALUE recv, VALUE arg)
{
	return float_new(FLOAT(recv)->num + num2float(arg));
}

VALUE
float_divide(VALUE recv, VALUE arg)
{
	return float_new(FLOAT(recv)->num / num2float(arg));
}

VALUE
float_pow(VALUE recv, VALUE arg)
{
	return float_new(powf(FLOAT(recv)->num, num2float(arg)));
}

VALUE
float_to_i(VALUE recv)
{
	return integer_new((int)FLOAT(recv)->num);
}

VALUE
float_to_f(VALUE recv)
{
	return recv;
}


void
Init_float()
{
	cFloat = define_class(intern("Float"), cNumeric);

	define_singleton_method(cFloat, intern("new"), (void*)float_s_new, 1);

	define_method(cFloat, intern("*"), float_multiply, 1);
	define_method(cFloat, intern("+"), float_addition, 1);
	define_method(cFloat, intern("-"), float_subtract, 1);
	define_method(cFloat, intern("/"), float_divide, 1);
	define_method(cFloat, intern("**"), float_pow, 1);

	define_method(cFloat, intern("to_f"), float_to_f, 0);
	define_method(cFloat, intern("to_i"), float_to_i, 0);
	define_method(cFloat, intern("to_s"), float_to_s, 0);
}
