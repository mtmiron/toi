#include "toi.h"


VALUE cNumeric;


void
Init_numeric()
{
	cNumeric = define_class(intern("Numeric"), cObject);
}
