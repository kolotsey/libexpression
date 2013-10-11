/*
 * Copyright 2011 Sergey Kolotsey.
 *
 * This file is part of libexpression library.
 *
 * libexpression is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libexpression is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public Licenise
 * along with libexpression. If not, see <http://www.gnu.org/licenses/>.
 *
 * =====================================================================
 *
 * Private libexpression types and structures.
 */

#ifndef LIBEXPRESSION_PRIVATE_H_
#define LIBEXPRESSION_PRIVATE_H_

#include "libexpression-config.h"

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !defined STDC_HEADERS && defined HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#include <stdio.h>
#include <errno.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <ctype.h>

#include <math.h>

#include "libexpression.h"


#define IS_SPACE(c) ((c)==' ' || (c)=='\t')
#define IS_BLANK(c) ((c)==' ' || (c)=='\t' ||(c)=='\r' ||(c)=='\n')
#define IS_NUMERIC(c) ((c)>='0' && (c)<='9')
#define IS_ALPHA(c) (((c)>='a' && (c)<='z') ||((c)>='A' && (c)<='Z') || (c)=='_')
#define IS_ALPHANUMERIC(c) (IS_NUMERIC(c) || IS_ALPHA(c))
#define IS_HEXADEMICAL(c) ({\
		int ret;\
		switch(c){\
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': ret=(c)-'0'; break;\
			case 'a': case 'A': ret=10; break;\
			case 'b': case 'B': ret=11; break;\
			case 'c': case 'C': ret=12; break;\
			case 'd': case 'D': ret=13; break;\
			case 'e': case 'E': ret=14; break;\
			case 'f': case 'F': ret=15; break;\
			default: ret=-1; break;\
		}\
		ret;\
	})
#define IS_OCTAL(c) ({\
		int ret;\
		if( (c)>='0' && (c)<='7'){\
			ret=(c)-'0';\
		}else{\
			ret=-1;\
		}\
		ret;\
	})
#define IS_BINARY(c) ({\
		int ret;\
		if( (c)=='0'){\
			ret=0;\
		}else if( (c)=='1'){\
			ret=1;\
		}else{\
			ret=-1;\
		}\
		ret;\
	})



/*
 * Type definitions
 */

typedef enum {
	T_NONE=0,
	T_LPAREN,
	T_RPAREN,
	T_COMMA,
	T_OPERATOR,
	T_INTEGER,
	T_REAL,
	T_BOOLEAN,
	T_STRING,
	T_PARAMETER,
	T_FUNCTION,

	T_IFCONDITION,
	T_IFSTATEMENT,
} token_type_t;


typedef enum {
	O_NONE      = 0,
    O_PLUS      = '+',
    O_MINUS     = '-',
    O_UMINUS    = 'a',  // unary minus (-)
    O_UPLUS     = 'b',  // unary plus (+)
    O_DIV       = '/',
    O_MOD       = '%',
    O_MUL       = '*',
    O_HAT       = '^',
    O_EQUALS    = '=',
    O_GT        = '>',
    O_LT        = '<',
    O_GE        = 'c',  // >=
    O_LE        = 'd',  // <=
    O_SHIFTRIGHT= 'e',  // >>
    O_SHIFTLEFT = 'f',  // <<
    O_BOOLEQUALS= 'g',  // ==
    O_NOTEQUALS = 'h',  // !=
    O_BOOLAND   = 'i',  // &&
    O_BOOLOR    = 'j',  // ||
    O_BOOLNOT   = '!',
    O_BITAND    = '&',
    O_BITOR     = '|',
    O_BITNOT    = '~',

    O_IFTHEN	= '?',
    O_ELSE		= ':',
} operator_t;


typedef struct {
	token_type_t type;
	union{
		int64_t integer;
		double real;
		int boolean;
		char *string;
		operator_t operator;
		char *function;
		char *parameter;
	} value;
} value_t;


typedef struct token_s{
	struct token_s *next;
	struct token_s *children;
	size_t position;
	value_t param;
} token_t;


typedef struct function_s{
	char *name;
	int argc;
}function_t;



#define IMPORT_TO_VALUE_T( from, to) {\
	if((to)->type==T_STRING && (to)->value.string){\
		free( (to)->value.string);\
		(to)->value.string=NULL;\
	}\
	if( (from)->type==EXP_INTEGER){\
		(to)->type=T_INTEGER;\
		(to)->value.integer=(from)->value.integer;\
	}else if( (from)->type==EXP_REAL){\
		(to)->type=T_REAL;\
		(to)->value.real=(from)->value.real;\
	}else if( (from)->type==EXP_BOOLEAN){\
		(to)->type=T_BOOLEAN;\
		(to)->value.boolean=(from)->value.boolean;\
	}else if( (from)->type==EXP_STRING){\
		(to)->type=T_STRING;\
		if((from)->value.string){\
			(to)->value.string=strdup((from)->value.string);\
		}else{\
			(to)->value.string=strdup("NULL");\
		}\
	}else{\
		(to)->type=T_NONE;\
	}\
}

#define EXPORT_FROM_VALUE_T( from, to) {\
	if((to)->type==EXP_STRING && (to->value.string)){\
		free( (to)->value.string);\
		(to)->value.string=NULL;\
	}\
	if( (from)->type==T_INTEGER){\
		(to)->type=EXP_INTEGER;\
		(to)->value.integer=(from)->value.integer;\
	}else if( (from)->type==T_REAL){\
		(to)->type=EXP_REAL;\
		(to)->value.real=(from)->value.real;\
	}else if( (from)->type==T_BOOLEAN){\
		(to)->type=EXP_BOOLEAN;\
		(to)->value.boolean=(from)->value.boolean;\
	}else if( (from)->type==T_STRING){\
		(to)->type=EXP_STRING;\
		if((from)->value.string){\
			(to)->value.string=strdup((from)->value.string);\
		}else{\
			(to)->value.string=strdup("NULL");\
		}\
	}else{\
		(to)->type=EXP_NONE;\
	}\
}








//from eval.c
int exp_to_double( value_t *v, double *ret);
int exp_to_integer( value_t *v, int64_t *ret);
int exp_to_boolean(value_t *v, int *ret);
int exp_to_string(value_t *v, char **ret);
int exp_eval_operator( token_t **stack, operator_t operator, int *operand_count);
int exp_is_integer( value_t *v, int64_t *i);

//from function.c
int exp_call_function( expression_t *exp, char *fname, int argc, token_t **stack, int *stack_len);

//from exp_rpn.c
int exp_rpn( expression_t *exp, token_t *input, value_t *ret, exp_error_t *ercode, char *error, int *error_pos);

//from shunting-yard.c
token_t *exp_shunting_yard( token_t *input, int if_operand, token_t **new_input, exp_error_t *ercode, char *error, int *error_pos);

//from tokenizer.c
int exp_op_argument_count( operator_t op);
int exp_op_is_lefttoright( operator_t op);
token_t *exp_token_free( token_t *head);
token_t *exp_token_dup( token_t *tok);
int exp_check( token_t *token, exp_error_t *ercode, char *error, int *error_pos);
token_t *exp_parse( char *exp, exp_error_t *ercode, char *error, int *error_pos);
#ifdef EXP_DEBUG
void token_print( char *msg, token_t *token, int recur);
#endif










#endif /* LIBEXPRESSION_PRIVATE_H_ */
