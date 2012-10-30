#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#define NODE_OBJECT(nd) ((struct node*)nd)->self

#include "toi.h"


enum node_type {
	NODE_IF,
	NODE_WHILE,
	NODE_BREAK,
	NODE_RESCUE,
	NODE_FUNCALL,
	NODE_ASSIGN,
	NODE_THROW,
	NODE_DEFINE,
};

typedef struct node {
	int type;	/* type flag */
	int negate;	/* negate boolean value? */

	VALUE arglist;	/* array of arguments to be passed */
	VALUE fsym;	/* symbol to resolve to a function pointer */
	VALUE self;	/* what the value of SELF (``this'' in C++) is to be set to for call
		            ... or, the symbol this node represents/holds */

	VALUE lhs;	/* left-hand symbol (assignation) */
	struct node *nd_rhs;	/* right-hand value in assignation statement */

	struct node *nd_if;	/* `if {this}'  */

	struct node *nd_head;	/* `if true then {this}', `try() {this}', etc. */
	struct node *nd_body;	/* `else then {this}', `catch() {this}', etc. */
	struct node *nd_tail;	/* `endif', end of try/catch, `ensure {this}', etc. */

	struct node *nd_and;	/* logical AND condition */
	struct node *nd_or;	/* logical OR condition */
} NODE;



#endif
