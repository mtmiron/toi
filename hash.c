/*
 * hash.c:
 *
 * 	Implements TOI Hash objects (also known as dictionaries,
 * 	associative-arrays, or sometimes key-value pairs).
 *
 */

#include "toi.h"



VALUE cHash;



void
hash_reap(VALUE hash)
{
	check_type(hash, T_HASH);

	st_free_table(HASH(hash)->ptr);
}


VALUE
hash_new()
{
	VALUE hash;

	hash = NEW_OBJ(Hash);
	OBJ_SETUP(hash, T_HASH);
	BASIC(hash).klass = cHash;
	BASIC(hash).reap = hash_reap;
	HASH(hash)->ptr = st_init_numtable();

	return (VALUE)hash;
}


VALUE
hash_aset(VALUE hash, VALUE key, VALUE val)
{
	st_table *tbl;

	check_type(hash, T_HASH);

	tbl = HASH(hash)->ptr;
	st_insert(tbl, (st_data_t)key, (st_data_t)val);
	
	return hash;
}


VALUE
hash_aref(VALUE hash, VALUE key)
{
	st_table *tbl;
	VALUE ret = Qnil;

	check_type(hash, T_HASH);

	tbl = HASH(hash)->ptr;
	st_lookup(tbl, (st_data_t)key, (st_data_t*)&ret);

	return ret;
}


void
Init_hash()
{
	cHash = define_class(intern("Hash"), cObject);
}
