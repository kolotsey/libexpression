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
 * API functions.
 */

#include "libexpression-private.h"


/*
 * Substitutes parameter with its value for the following constants:
 * pi, e, true, false
 *
 * @param token_t *token Token item which type MUST be T_PARAMETER
 * @return If the parameter name is valide, returns 0, otherwise returns 1
 */
static int substitute_parameter( char *parameter_name, value_t *result){
	if( NULL==parameter_name){
		return 1;
	}else{
		if( 0==strcasecmp( "pi", parameter_name)){
			result->type=T_REAL;
			result->value.real=3.1415926535897932384626433832795028841971693993751058209749;

		}else if( 0==strcasecmp( "e", parameter_name)){
			result->type=T_REAL;
			result->value.real=2.7182818284590452353602874713526624977572470936999595749669;

		}else if(0==strcasecmp( "true", parameter_name) ||0==strcasecmp( "yes", parameter_name)
				||0==strcasecmp( "on", parameter_name)){
			result->type=T_BOOLEAN;
			result->value.boolean=1;

		}else if(0==strcasecmp( "false", parameter_name) ||0==strcasecmp( "no", parameter_name)
				||0==strcasecmp( "off", parameter_name)){
			result->type=T_BOOLEAN;
			result->value.boolean=0;

		}else{
			return 1;
		}
	}
	return 0;
}


static int substitute_parameters_in_expr( expression_t *exp, token_t *t, exp_error_t *ercode, char *error, int *erpos){

	//Substitute parameters
	while(t){
		if(t->param.type==T_PARAMETER){
			char *p=t->param.value.parameter;
			if( 0 ==substitute_parameter( p, &t->param)){
				//the parameter was successfully substituted with its value
				free( p);
			}else{
				exp_value_t exv;
				if( exp->phandler && 0==exp->phandler( exp->user_data, p, &exv)){
					//the parameter was successfully substituted with its value
					free( p);
					IMPORT_TO_VALUE_T( &exv, &t->param);
					if( exv.type==EXP_STRING && exv.value.string){
						free(exv.value.string);
					}
					if( t->param.type==T_NONE){
						*erpos=t->position;
						*ercode=EXP_ER_INVALRET;
						strcpy( error, "Unknown type was returned by user defined parameter handler");
						return 1;
					}

				}else{
					//could not subst the parameter
					*erpos=t->position;
					*ercode=EXP_ER_INVALPARAM;
					snprintf( error, EXP_ERLEN, "Unknown parameter '%s'", p);
					return 1;
				}
			}
		}else if( t->param.type==T_IFSTATEMENT){
			if( 0 !=substitute_parameters_in_expr( exp, t->children, ercode, error, erpos)){
				return 1;
			}
		}
		t=t->next;
	}
	return 0;
}


/*
 * Creates a structure expression_t with the parsed expression
 *
 * @param char *e String with the expr
 * @param char *error Error string is returned if e could not be parsed
 * @return expression_t* Expr if e was parsed successfully or NULL if error occured.
 *         In the later case error string is set.
 */
expression_t *exp_create( const char *e, exp_error_t *ercode, char *error, int *erpos){
	expression_t *ret=NULL;
	token_t *a, *b;
	char *ecopy;

	if(NULL==(ecopy=strdup( e))){
		*ercode=EXP_ER_NOMEM;
		strcpy( error, "Memory error");
		*erpos=0;
		return NULL;
	}

	//strtolower( ecopy);

	if(( a=exp_parse( ecopy, ercode, error, erpos))){
		if( 0 ==exp_check( a, ercode, error, erpos)){
			if(( b=exp_shunting_yard( a, 0, NULL, ercode, error, erpos))){
				if(NULL==( ret=calloc( 1, sizeof( expression_t)))){
					exp_token_free( b);
					*ercode=EXP_ER_NOMEM;
					strcpy( error, "Memory error");
					*erpos=0;
				}else{
					ret->tokens=b;
					ret->e=ecopy;
					ecopy=NULL;
				}
			}
		}
		exp_token_free( a);
	}
	if( ecopy) free( ecopy);
	return ret;
}


/*
 * Return 0 if exprs are equal
 */
int exp_equals( expression_t *a, expression_t *b){
	return strcmp( a->e, b->e);
}


/*
 * Solve expr and return result
 * @return value if exp_rpn succeeded,
 *         otherwise NULL is returned and error string is set.
 *         Returned value should be freed
 */
exp_value_t *exp_solve( expression_t *exp, exp_error_t *ercode, char *error, int *erpos){
	int status;
	value_t v;
	token_t *tokens;
	exp_value_t *result=NULL;

	tokens=exp_token_dup( exp->tokens);

	if( 0==substitute_parameters_in_expr( exp, tokens, ercode, error, erpos)){
		//call exp_rpn algorithm
		if(0 ==(status=exp_rpn( exp, tokens, &v, ercode, error, erpos))){

			if( NULL==( result=calloc(1, sizeof(exp_value_t)))){
				*ercode=EXP_ER_NOMEM;
				strcpy(error, "Memory error");
				if( v.type==T_STRING && v.value.string) free( v.value.string);
				*erpos=0;

			}else{
				EXPORT_FROM_VALUE_T( &v, result);
				if( v.type==T_STRING && v.value.string) free( v.value.string);
				if( result->type==EXP_NONE){
					*ercode=EXP_ER_INVALEXPR;
					strcpy(error, "Result type is invalid");
					*erpos=0;
					free( result);
					result=NULL;
				}
			}
		}
	}
	exp_token_free( tokens);
	return result;
}



char *exp_value_to_string_r( exp_value_t *ev, char *buffer, int buffer_max_len){
	if( NULL==buffer || buffer_max_len<1){
		errno=ENOBUFS;
		return NULL;

	}else{
		if(ev->type==EXP_REAL){
			snprintf( buffer, buffer_max_len, "%0.9f", ev->value.real);
			buffer[buffer_max_len-1]=0;
			if( strchr( buffer, '.')){
				int l=strlen( buffer);
				while( l && '0'==buffer[l-1]) l--;
				if( l && buffer[l-1]=='.') l--;
				buffer[l]=0;
			}
			return buffer;

		}else if(ev->type==EXP_INTEGER){
			snprintf( buffer, buffer_max_len, "%lld", ev->value.integer);
			buffer[buffer_max_len-1]=0;
			return buffer;

		}else if(ev->type==EXP_BOOLEAN){
			snprintf( buffer, buffer_max_len, "%s", ev->value.boolean? "True" : "False");
			buffer[buffer_max_len-1]=0;
			return buffer;

		}else if(ev->type==EXP_STRING){
			snprintf( buffer, buffer_max_len, "%s", ev->value.string);
			buffer[buffer_max_len-1]=0;
			return buffer;

		}else{
			//result type is invalid
			errno=EINVAL;
			return NULL;
		}
	}
}

static char *temp_buffer=NULL;
static int temp_buffer_len=0;
char *exp_value_to_string( exp_value_t *ev){

	int new_buffer_len;
	if(ev->type==EXP_STRING){
		new_buffer_len=strlen( ev->value.string)+1;
	}else{
		new_buffer_len=1024;
	}

	//Alloc temporary buffer
	if( temp_buffer_len <new_buffer_len){
		if((temp_buffer=realloc(temp_buffer, new_buffer_len))){
			temp_buffer_len=new_buffer_len;

		}else{
			temp_buffer_len=0;
			errno=ENOMEM;
			return NULL;
		}
	}

	return exp_value_to_string_r( ev, temp_buffer, temp_buffer_len);
}


expression_t *exp_free( expression_t *exp){
	exp_token_free( exp->tokens);
	free( exp->e);
	free( exp);
	return NULL;
}

exp_value_t *exp_value_free( exp_value_t *ev){
	if(ev->type==EXP_STRING && ev->value.string) free (ev->value.string);
	free( ev);
	return NULL;
}


