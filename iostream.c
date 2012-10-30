/*
 * iostream.c:
 * 
 * 	Implements the IOStream class, which is the
 * 	superclass that implements most (or all) of
 * 	any I/O stream object's methods (files, etc.).
 *
 */

#include "toi.h"
#include <fcntl.h>

VALUE cIOStream;
VALUE cFile;



VALUE
file_writable_p(VALUE recv)
{
	if ((TYPE(recv) != T_IOSTREAM) && (TYPE(recv) != T_FILE))
		check_type(recv, T_FILE);

	if (IOSTREAM(recv)->fwrite)
		return Qtrue;
	else
		return Qfalse;
}

VALUE
file_readable_p(VALUE recv)
{
	if ((TYPE(recv) != T_IOSTREAM) && (TYPE(recv) != T_FILE))
		check_type(recv, T_FILE);

	if (IOSTREAM(recv)->fread)
		return Qtrue;
	else
		return Qfalse;
}


VALUE
file_flush(VALUE self)
{
	if (!file_writable_p(self))
		return Qtrue;

	fflush(IOSTREAM(self)->fwrite);
	return Qtrue;
}


static void
io_reap(VALUE recv)
{
	file_close(recv);
}


static VALUE
iostream_new()
{
	VALUE io;

	io = NEW_OBJ(IOStream);
	OBJ_SETUP(io, T_IOSTREAM);
	BASIC(io).klass = cIOStream;
	BASIC(io).reap = io_reap;

	return (VALUE)io;
}


static VALUE
iostream_wrap(FILE *in, FILE *out)
{
	VALUE io;

	io = iostream_new();
	IOSTREAM(io)->fread = in;
	IOSTREAM(io)->fwrite = out;
	return (VALUE)io;
}


VALUE
file_open(char *path, char *mode)
{
	VALUE io;
	FILE *f;

	f = fopen(path, mode);
	if (!f)
		fail(strerror(errno));

	io = iostream_new();

	if (strchr(mode, '+'))
	{
		IOSTREAM(io)->fread = f;
		IOSTREAM(io)->fwrite = f;
	}
	else if (strchr(mode, 'w'))
	{
		IOSTREAM(io)->fwrite = f;
	}
	else if (strchr(mode, 'r'))
	{
		IOSTREAM(io)->fread = f;
	}

	IOSTREAM(io)->path = string_new(path);

	return (VALUE)io;
}

VALUE
file_new(char *path, char *mode)
{
	int fd;

	fd = open(path, O_RDONLY | O_CREAT | O_EXCL);
	if (fd >= 0)
		close(fd);

	return file_open(path, mode);
}

static VALUE
file_new_internal(VALUE self, int argc, VALUE *argv, int create)
{
	VALUE path, mode;

	if (argc < 1)
		fail("File.new() requires a filename");
	else if (argc < 2)
		mode = string_new("r+");
	else
		mode = argv[1];
	path = argv[0];

	path = toi_funcall(path, intern("to_s"), 0);
	mode = toi_funcall(mode, intern("to_s"), 0);

	if (create)
		return file_new(str2cstr(path), str2cstr(mode));
	else
		return file_open(str2cstr(path), str2cstr(mode));
}


VALUE
file_new_thunk(VALUE self, int argc, VALUE *argv)
{
	return file_new_internal(self, argc, argv, 1);
}

VALUE
file_open_thunk(VALUE self, int argc, VALUE *argv)
{
	return file_new_internal(self, argc, argv, 0);
}


VALUE
file_close(VALUE recv)
{
	FILE *fr, *fw;

	if ((TYPE(recv) != T_IOSTREAM) && (TYPE(recv) != T_FILE))
		check_type(recv, T_FILE);

	fr = IOSTREAM(recv)->fread;
	fw = IOSTREAM(recv)->fwrite;

	if (fr)
		fclose(fr);
	if (fw && (fw != fr))
		fclose(fw);

	IOSTREAM(recv)->fread = NULL;
	IOSTREAM(recv)->fwrite = NULL;

	return (VALUE)recv;
}


VALUE
file_read(VALUE recv)
{
	char buf[1024] = { '\0' };
	int len = sizeof(buf);
	FILE *f;
	VALUE ret;

	if (!TEST(file_readable_p(recv)))
		fail("iostream is not readable");

	f = IOSTREAM(recv)->fread;
	ret = string_new("");

	while (!feof(f))
	{
		fread(buf, 1, len-1, f);
		buf[len-1] = '\0';
		dstrcat(STRING(ret)->ptr, buf);
	}

	return (VALUE)ret;
}


VALUE
file_write(VALUE recv, VALUE arg)
{
	FILE *f;
	char *cstr;
	int len;
	VALUE strobj;

	if (!TEST(file_writable_p(recv)))
		fail("iostream is not writable");

	f = IOSTREAM(recv)->fwrite;
	strobj = toi_funcall(arg, intern("to_s"), 0);
	cstr = str2cstr(strobj);
	len = strlen(cstr);

	return integer_new(fwrite(cstr, 1, len, f));
}


VALUE
file_puts(VALUE recv, VALUE arg)
{
	VALUE bytes;
	VALUE strobj;

	strobj = toi_funcall(arg, intern("to_s"), 0);
	bytes = file_write(recv, strobj);
	if (string_c_last_char(strobj) != '\n')
		bytes = toi_funcall(bytes, intern("+"), 1,
		                    file_write(recv, string_new("\n")));
	return bytes;
}


VALUE
file_unlink(VALUE recv, VALUE path)
{
	check_type(path, T_STRING);
	if (unlink(str2cstr(path)) == -1)
		fail(strerror(errno));

	return Qtrue;
}


VALUE
file_gets(VALUE recv)
{
	char buf[65536];

	fgets(buf, sizeof(buf), IOSTREAM(recv)->fread);
	return string_new(buf);
}


VALUE
file_puts_thunk(VALUE self, int argc, VALUE *argv)
{
	int i = 0;
	VALUE ret = Qnil;

	if (!argc)
		ret = file_puts(self, string_new(""));
	while (argc--)
		ret = file_puts(self, argv[i++]);

	return ret;
}


VALUE
file_path(VALUE self)
{
	if ( (TYPE(self) != T_IOSTREAM) &&
	     (TYPE(self) != T_FILE) )
		check_type(self, T_FILE);

	return IOSTREAM(self)->path;
}


void
Init_iostream()
{
	cIOStream = define_class(intern("IOStream"), cObject);
	cFile = define_class(intern("File"), cIOStream);

	define_singleton_method(cFile, intern("new"), file_new_thunk, -1);
	define_singleton_method(cFile, intern("open"), file_new_thunk, -1);
	define_singleton_method(cFile, intern("unlink"), file_unlink, 1);
	define_singleton_method(cFile, intern("delete"), file_unlink, 1);

	define_method(cIOStream, intern("close"), file_close, 0);
	define_method(cIOStream, intern("read"), file_read, 0);
	define_method(cIOStream, intern("write"), file_write, 1);
	define_method(cIOStream, intern("puts"), file_puts_thunk, -1);
	define_method(cIOStream, intern("gets"), file_gets, 0);
	define_method(cIOStream, intern("flush"), file_flush, 0);
	define_method(cIOStream, intern("path"), file_path, 0);

	define_global_method(intern("puts"), file_puts, 1);
	define_global_method(intern("write"), file_write, 1);

	define_method(cIOStream, intern("readable?"), file_readable_p, 0);
	define_method(cIOStream, intern("writable?"), file_writable_p, 0);

	define_global_variable(intern("stdin"), iostream_wrap(stdin, NULL));
	define_global_variable(intern("stdout"), iostream_wrap(NULL, stdout));
	define_global_variable(intern("stderr"), iostream_wrap(NULL, stderr));
}
