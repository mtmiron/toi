#include "toi.h"


VALUE cArray;


static int
ary_reap_check(void *ptr)
{
	return object_refcount_down(ptr);
}


void
ary_reap(VALUE ary)
{
	check_type(ary, T_ARRAY);

	vector_foreach(ARRAY(ary)->ptr, ary_reap_check);
	vector_free(ARRAY(ary)->ptr);
}


VALUE
ary_new()
{
	VALUE ary;

	ary = NEW_OBJ(Array);
	OBJ_SETUP(ary, T_ARRAY);
	BASIC(ary).klass = cArray;
	BASIC(ary).reap = ary_reap;
	ARRAY(ary)->ptr = vector_new();

	return (VALUE)ary;
}


VALUE
ary_s_new(VALUE recv)
{
	return ary_new();
}


VALUE
ary_length(VALUE recv)
{
	return integer_new(vector_length(ARRAY(recv)->ptr));
}


VALUE
ary_push(VALUE recv, VALUE arg)
{
	BASIC(arg).refcount++;
	vector_push(ARRAY(recv)->ptr, (void*)arg);
	return recv;
}


VALUE
ary_pop(VALUE recv)
{
	VALUE ret;
	
	ret = (VALUE)vector_pop(ARRAY(recv)->ptr);
	if (!TEST(ret))
		return Qnil;

	BASIC(ret).refcount--;
	return ret;
}


VALUE
ary_shift(VALUE recv)
{
	VALUE ret;

	ret = (VALUE)vector_shift(ARRAY(recv)->ptr);
	if (!TEST(ret))
		return Qnil;

	BASIC(ret).refcount--;
	return ret;
}


VALUE
ary_unshift(VALUE recv, VALUE arg)
{
	BASIC(arg).refcount++;
	vector_unshift(ARRAY(recv)->ptr, (void*)arg);
	return recv;
}


VALUE
ary_aref(VALUE recv, VALUE arg)
{
	VALUE ret;
	int idx;

	if (TYPE(arg) != T_INTEGER)
		idx = arg;
	else
		idx = INT(arg)->num;

	ret = (VALUE)vector_aref(ARRAY(recv)->ptr, INT(arg)->num);
	if (!TEST(ret))
		return Qnil;
	
	return ret;
}


VALUE
ary_first(VALUE recv)
{
	return ary_aref(recv, integer_new(0));
}


VALUE
ary_last(VALUE recv)
{
	return ary_aref(recv, integer_new(-1));
}


VALUE
ary_to_s(VALUE ary)
{
	VALUE strobj = string_new("[");
	vector_t *v = ARRAY(ary)->ptr;
	dynstring_t *s = STRING(strobj)->ptr;
	int i, len;

	i = 0;
	len = vector_length(v);
	while (len)
	{
		dstrcat(s, str2cstr( toi_funcall((VALUE)vector_aref(v, i), intern("inspect"), 0) ));
		if (++i >= len)
			break;
		dstrcat(s, ", ");
	}
	dstrcat(s, "]");

	return strobj;
}


void
Init_array()
{
	cArray = define_class(intern("Array"), cObject);

	define_singleton_method(cArray, intern("new"), ary_s_new, 0);

	define_method(cArray, intern("new"), ary_s_new, 0);
	define_method(cArray, intern("to_s"), ary_to_s, 0);
	define_method(cArray, intern("length"), ary_length, 0);
	define_method(cArray, intern("push"), ary_push, 1);
	define_method(cArray, intern("pop"), ary_pop, 0);
	define_method(cArray, intern("shift"), ary_shift, 0);
	define_method(cArray, intern("unshift"), ary_unshift, 1);
	define_method(cArray, intern("last"), ary_last, 0);
	define_method(cArray, intern("first"), ary_first, 0);
	define_method(cArray, intern("[]"), ary_aref, 1);
}
