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
 * Definitions of various operators.
 * Function exp_eval_operator is called from rpn.c.
 */

#include "libexpression-private.h"

/*
 * @return Returns 0 if the string contains a number or EXP_ER_NONNUMERIC otherwise
 */
static int is_number( char *s, int64_t *i, double *d){
	int slen;
	int rlen;
	int c;

	//trim string
	while(IS_SPACE( *s)) s++;
	slen=strlen( s);
	while( slen && IS_BLANK(s[slen-1])) slen--;

	if(slen>=3 && (0==strncmp("0x", s, 2)) && (-1 !=( c=IS_HEXADEMICAL(s[2])))){
		uint64_t num;

		//hexademical value
		rlen=3;
		num=c;
		s+=3;
		while( -1 !=(c=IS_HEXADEMICAL( *s))){
			num=num*16+c;
			s++;
			rlen++;
		}
		if(rlen==slen){
			*i=(int64_t)num;
			*d=(double)num;
			return 0;
		}else{
			return EXP_ER_NONNUMERIC;
		}

	}else{
		//test if value is real or integer
		char *p=s;
		int is_real=0;
		int efound=0;
		int pointfound=0;
		int neg=0;
		int signfound=0;

		while( *p){
			if( is_real){
				if( !IS_NUMERIC( *p)){
					if( *p=='.' && !pointfound){
						pointfound=1;
					}else if( *p=='e' && !efound && (p-s>0) && ((*(p+1)=='+') || (*(p+1)=='-'))){
						efound=1;
						pointfound=1;
						p++;
					}else {
						break;
					}
				}
			}else{
				if( !IS_NUMERIC( *p)){
					if( *p=='.'){
						is_real=1;
						pointfound=1;
					}else if(*p=='-' && !signfound){
						neg=1;
						signfound=1;
					}else if(*p=='+' && !signfound){
						signfound=1;
					}else if( *p=='e' && (p-s>0) && ((*(p+1)=='+') || (*(p+1)=='-'))){
						is_real=1;
						efound=1;
						pointfound=1;
						p++;//also shift a sign

					}else{
						break;
					}
				}
			}
			p++;
		}
		//lets see if token is number and if it is real number or integer number
		//number length is
		rlen=p-s;
		if(rlen==0 || (rlen==1 && pointfound)){
			if( 0==slen){
				return EXP_ER_NONNUMERIC;
			}
			//it might be a boolean string
			if(0==strncasecmp( s, "true", slen) ||0==strncasecmp( s, "yes", slen) ||0==strncasecmp( s, "on", slen) ||
					0==strncasecmp( s, "t", slen) ||0==strncasecmp( s, "y", slen)){
				*i=1;
				*d=1;
				return 0;
			}else if(0==strncasecmp( s, "false", slen) ||0==strncasecmp( s, "no", slen) ||0==strncasecmp( s, "off", slen) ||
					0==strncasecmp( s, "f", slen) ||0==strncasecmp( s, "n", slen)){
				*i=0;
				*d=0;
				return 0;
			}
			return EXP_ER_NONNUMERIC;

		}else if(rlen==slen){
			//ok, it's a number
			if(is_real){
				*i=(int64_t)atof( s);
				*d=atof( s);
			}else{
				*i=atoll( s);
				*d=(double)atoll( s);
			}
			return 0;
		}else{
			return EXP_ER_NONNUMERIC;
		}
	}
}

int exp_to_boolean(value_t *v, int *ret){
	char *s;
	int slen;
	switch( v->type){
		case T_INTEGER:
			*ret= (v->value.integer==0)? 0 : 1;
			break;
		case T_REAL:
			if((int64_t)v->value.real==0){
				*ret=0;
			}else if(((int64_t)v->value.real==1)){
				*ret=1;
			}else{
				return EXP_ER_NONBOOLEAN;
			}
			break;
		case T_BOOLEAN:
			*ret=v->value.boolean;
			break;
		case T_STRING:
			s=v->value.string;
			while( IS_SPACE( *s)) s++;
			slen=strlen(s);
			while( slen && IS_BLANK(s[slen-1])) slen--;
			if(0==strncasecmp( s, "true", slen) ||0==strncasecmp( s, "yes", slen) ||0==strncasecmp( s, "on", slen) ||
					0==strncasecmp( s, "t", slen) ||0==strncasecmp( s, "y", slen)){
				*ret=1;
			}else if(0==strncasecmp( s, "false", slen) ||0==strncasecmp( s, "no", slen) ||0==strncasecmp( s, "off", slen) ||
					0==strncasecmp( s, "f", slen) ||0==strncasecmp( s, "n", slen)){
				*ret=0;
			}else{
				return EXP_ER_NONBOOLEAN;
			}
			break;
		default:
			return EXP_ER_INVALARGV;
			break;
	}
	return 0;
}

int exp_to_double( value_t *v, double *ret){
	int64_t i;
	switch( v->type){
		case T_INTEGER:
			*ret=(double)v->value.integer;
			break;
		case T_REAL:
			*ret=v->value.real;
			break;
		case T_BOOLEAN:
			if(v->value.boolean){
				*ret=1;
			}else{
				*ret=0;
			}
			break;
		case T_STRING:
			if(v->value.string==NULL ||( 0 !=is_number(v->value.string, &i, ret))){
				return EXP_ER_NONNUMERIC;
			}
			break;
		default:
			return EXP_ER_INVALARGV;
			break;
	}
	return 0;
}

int exp_to_integer( value_t *v, int64_t *ret){
	double d;
	switch( v->type){
		case T_INTEGER:
			*ret=v->value.integer;
			break;
		case T_REAL:
			if((v->value.real > INT64_MAX) || (v->value.real < INT64_MIN)){
				return EXP_ER_INTOVERFLOW;
			}else{
				*ret=(int64_t)v->value.real;
			}
			break;
		case T_BOOLEAN:
			if(v->value.boolean){
				*ret=1;
			}else{
				*ret=0;
			}
			break;
		case T_STRING:
			if(v->value.string==NULL ||( 0 !=is_number(v->value.string, ret, &d))){
				return EXP_ER_NONNUMERIC;
			}
			break;
		default:
			return EXP_ER_INVALARGV;
			break;
	}
	return 0;
}

int exp_is_integer( value_t *v, int64_t *i){
	int64_t i1;
	double d1;
	switch( v->type){
		case T_INTEGER:
			*i=v->value.integer;
			return 0;
			break;
		case T_REAL:
			if( v->value.real>INT64_MAX || v->value.real< INT64_MIN){
				return EXP_ER_NONINTEGER;
			}else if( round(v->value.real) !=v->value.real){
				return EXP_ER_NONINTEGER;
			}else{
				*i=round(v->value.real);
				return 0;
			}
			break;
		case T_BOOLEAN:
			*i=v->value.boolean? 1 : 0;
			return 0;
			break;
		case T_STRING:
			if(v->value.string==NULL ||( 0 !=is_number(v->value.string, &i1, &d1))){
				return EXP_ER_NONNUMERIC;
			}else if(d1<=INT64_MAX && d1>=INT64_MIN && i1==round( d1)){
				return 0;
			}else{
				return EXP_ER_NONINTEGER;
			}
			break;
		default:
			return EXP_ER_INVALARGV;
			break;
	}
	//Can not get here
	return EXP_ER_INVALARGV;
}



int exp_to_string(value_t *v, char **ret){
	char *s;

	switch( v->type){
		case T_INTEGER:
			if(( s=malloc(128))){
				snprintf(s, 128, "%lld", (long long int)v->value.integer);
			}else{
				return EXP_ER_NOMEM;
			}
			break;
		case T_REAL:
			if(( s=malloc(128))){
				snprintf(s, 128, "%f", v->value.real);
			}else{
				return EXP_ER_NOMEM;
			}
			break;
		case T_BOOLEAN:
			if(( s=malloc(8))){
				sprintf(s, "%s", v->value.boolean? "true" : "false");
			}else{
				return EXP_ER_NOMEM;
			}
			break;
		case T_STRING:
			if(NULL==( s=strdup( v->value.string))){
				return EXP_ER_NOMEM;
			}
			break;
		default:
			return EXP_ER_INVALARGV;
			break;
	}
	*ret=s;
	return 0;
}

static int operator_evaluate_boolnot( token_t *op, value_t *ret){
	int status, i1=0;

	if( 0 !=(status=exp_to_boolean( &op->param, &i1))){
		return status;
	}
	ret->value.boolean= i1? 0 : 1;
	ret->type=T_BOOLEAN;
	return 0;
}

static int operator_evaluate_bitnot( token_t *op, value_t *ret){
	int status;
	int64_t i1=0;

	if( 0 !=(status=exp_is_integer( &op->param, &i1))){
		return status;
	}
	ret->value.integer= ~i1;
	ret->type=T_INTEGER;
	return 0;
}

static int operator_evaluate_uminus( token_t *op, value_t *ret){
	if(op->param.type==T_INTEGER){
		ret->value.integer=-op->param.value.integer;
		ret->type=T_INTEGER;
	}else if( op->param.type==T_REAL){
		ret->value.real=-op->param.value.real;
		ret->type=T_REAL;
	}else if( op->param.type==T_BOOLEAN){
		ret->value.integer=op->param.value.boolean? -1 : 0;
		ret->type=T_INTEGER;
	}else if(op->param.type==T_STRING){
		int64_t i;
		double d;
		if(op->param.value.string && 0==is_number( op->param.value.string, &i, &d)){
			ret->value.real=-d;
			ret->type=T_REAL;
		}else{
			return EXP_ER_NONNUMERIC;
		}
	}else{
		return EXP_ER_INVALARGV;
	}
	return 0;
}

static int operator_evaluate_uplus( token_t *op, value_t *ret){
	if(op->param.type==T_INTEGER){
		ret->value.integer=op->param.value.integer;
		ret->type=T_INTEGER;
	}else if( op->param.type==T_REAL){
		ret->value.real=op->param.value.real;
		ret->type=T_REAL;
	}else if( op->param.type==T_BOOLEAN){
		ret->value.integer=op->param.value.boolean? 1 : 0;
		ret->type=T_INTEGER;
	}else if(op->param.type==T_STRING){
		int64_t i;
		double d;
		if(op->param.value.string && 0==is_number( op->param.value.string, &i, &d)){
			ret->value.real=d;
			ret->type=T_REAL;
		}else{
			return EXP_ER_NONNUMERIC;
		}
	}else{
		return EXP_ER_INVALARGV;
	}
	return 0;
}

static int operator_evaluate_hat( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		return status;
	}else{
		if((d1==0) && (d2<=0)){
			return EXP_ER_DIVBYZERO;

		}else{
			if(d1<0){
				int64_t i2;
				if(0==(status=exp_is_integer( &op->next->param, &i2))){
					ret->value.real=pow( d1, i2);
					ret->type=T_REAL;
					return 0;

				}else{
					if( status==EXP_ER_NONINTEGER){
						return EXP_ER_COMPLEX;
					}else{
						return status;
					}
				}
			}else{
				ret->value.real=pow( d1, d2);
				ret->type=T_REAL;
				return 0;
			}
		}
	}
}

static int operator_evaluate_div( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		return status;
	}else if(d2==0){
		return EXP_ER_DIVBYZERO;
	}else{
		ret->value.real=d1 / d2;
		ret->type=T_REAL;
		return 0;
	}
}

static int operator_evaluate_mod( token_t *op, value_t *ret){
	int status;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;

	}else{
		int64_t i1=0, i2=0;
		if((0==(status=exp_is_integer( &op->param, &i1))) && (0==(status=exp_is_integer( &op->next->param, &i2)))){
			if(i2==0){
				return EXP_ER_DIVBYZERO;
			}else{
				ret->value.integer=i1 % i2;
				ret->type=T_INTEGER;
				return 0;
			}


		}else{
			return status;
		}
	}
}

static int operator_evaluate_mul( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		return status;
	}else{
		ret->value.real=d1 * d2;
		ret->type=T_REAL;
		return 0;
	}
}

static int operator_concat( token_t *op, value_t *ret){
	int status;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if( op->param.type==T_STRING ||op->next->param.type==T_STRING ){
		char *s1;
		if( 0==(status=exp_to_string( &op->param, &s1))){
			char *s2;
			if( 0==(status=exp_to_string( &op->next->param, &s2))){
				char *s;
				int l=strlen(s1)+strlen(s2);
				if(( s=malloc( l+1))){
					snprintf( s, l+1, "%s%s", s1, s2);
					ret->type=T_STRING;
					ret->value.string=s;
				}else{
					status=EXP_ER_NOMEM;
				}
				free( s2);
			}
			free( s1);
		}
		return status;

	}else{
		return EXP_ER_NONSTRING;
	}
}

static int operator_evaluate_plus( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		if( op->param.type==T_STRING ||op->next->param.type==T_STRING ){
			if( 0 != operator_concat( op, ret)){
				return status;
			}else{
				return 0;
			}
		}else{
			return status;
		}
	}else{
		ret->value.real=d1 + d2;
		ret->type=T_REAL;
		return 0;
	}
}

static int operator_evaluate_minus( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		return status;
	}else{
		ret->value.real=d1 - d2;
		ret->type=T_REAL;
		return 0;
	}
}

static int operator_evaluate_shiftleft( token_t *op, value_t *ret){
	int status;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else{
		int64_t i1, i2;
		if((0==(status=exp_is_integer( &op->param, &i1))) && (0==(status=exp_is_integer( &op->next->param, &i2)))){
			ret->value.integer=i1 << i2;
			ret->type=T_INTEGER;
			return 0;
		}else{
			return EXP_ER_NONINTEGER;
		}
	}
}

static int operator_evaluate_shiftright( token_t *op, value_t *ret){
	int status;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else{
		int64_t i1, i2;
		if((0==(status=exp_is_integer( &op->param, &i1))) && (0==(status=exp_is_integer( &op->next->param, &i2)))){
			ret->value.integer=i1 >> i2;
			ret->type=T_INTEGER;
			return 0;
		}else{
			return EXP_ER_NONINTEGER;
		}
	}
}

static int operator_evaluate_gt( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		if( op->param.type==T_STRING && op->next->param.type==T_STRING){
			if(op->param.value.string && op->next->param.value.string){
				ret->type=T_BOOLEAN;
				if( strcmp( op->param.value.string, op->next->param.value.string)>0){
					ret->value.boolean=1;
				}else{
					ret->value.boolean=0;
				}
				return 0;
			}else{
				return EXP_ER_NOMEM;
			}
		}else{
			return status;
		}
	}else{
		ret->value.boolean=d1 > d2 ? 1 : 0;
		ret->type=T_BOOLEAN;
		return 0;
	}
}

static int operator_evaluate_lt( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		if( op->param.type==T_STRING && op->next->param.type==T_STRING){
			if(op->param.value.string && op->next->param.value.string){
				ret->type=T_BOOLEAN;
				if( strcmp( op->param.value.string, op->next->param.value.string)<0){
					ret->value.boolean=1;
				}else{
					ret->value.boolean=0;
				}
				return 0;
			}else{
				return EXP_ER_NOMEM;
			}
		}else{
			return status;
		}
	}else{
		ret->value.boolean=d1 < d2 ? 1 : 0;
		ret->type=T_BOOLEAN;
		return 0;
	}
}

static int operator_evaluate_ge( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		if( op->param.type==T_STRING && op->next->param.type==T_STRING){
			if(op->param.value.string && op->next->param.value.string){
				ret->type=T_BOOLEAN;
				if( strcmp( op->param.value.string, op->next->param.value.string)>=0){
					ret->value.boolean=1;
				}else{
					ret->value.boolean=0;
				}
				return 0;
			}else{
				return EXP_ER_NOMEM;
			}
		}else{
			return status;
		}
	}else{
		ret->value.boolean=d1 >= d2 ? 1 : 0;
		ret->type=T_BOOLEAN;
		return 0;
	}
}

static int operator_evaluate_le( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		if( op->param.type==T_STRING && op->next->param.type==T_STRING){
			if(op->param.value.string && op->next->param.value.string){
				ret->type=T_BOOLEAN;
				if( strcmp( op->param.value.string, op->next->param.value.string)<=0){
					ret->value.boolean=1;
				}else{
					ret->value.boolean=0;
				}
				return 0;
			}else{
				return EXP_ER_NOMEM;
			}
		}else{
			return status;
		}
	}else{
		ret->value.boolean=d1 <= d2 ? 1 : 0;
		ret->type=T_BOOLEAN;
		return 0;
	}
}

static int operator_evaluate_boolequals( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		if( op->param.type==T_STRING && op->next->param.type==T_STRING){
			if(op->param.value.string && op->next->param.value.string){
				ret->type=T_BOOLEAN;
				if( strcmp( op->param.value.string, op->next->param.value.string)==0){
					ret->value.boolean=1;
				}else{
					ret->value.boolean=0;
				}
				return 0;
			}else{
				return EXP_ER_NOMEM;
			}
		}else{
			return status;
		}
	}else{
//printf("eval %f==%f\n", d1, d2);
		ret->value.boolean=d1 == d2 ? 1 : 0;
		ret->type=T_BOOLEAN;
		return 0;
	}
}

static int operator_evaluate_notequals( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		if( op->param.type==T_STRING && op->next->param.type==T_STRING){
			if(op->param.value.string && op->next->param.value.string){
				ret->type=T_BOOLEAN;
				if( strcmp( op->param.value.string, op->next->param.value.string)!=0){
					ret->value.boolean=1;
				}else{
					ret->value.boolean=0;
				}
				return 0;
			}else{
				return EXP_ER_NOMEM;
			}
		}else{
			return status;
		}
	}else{
		ret->value.boolean=d1 != d2 ? 1 : 0;
		ret->type=T_BOOLEAN;
		return 0;
	}
}

static int operator_evaluate_bitand( token_t *op, value_t *ret){
	int status;
	int64_t i1=0, i2=0;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else{
		if((0==(status=exp_is_integer( &op->param, &i1))) && (0==(status=exp_is_integer( &op->next->param, &i2)))){
			ret->value.integer=i1 & i2;
			ret->type=T_INTEGER;
			return 0;
		}else{
			return EXP_ER_NONINTEGER;
		}
	}
}

static int operator_evaluate_bitor( token_t *op, value_t *ret){
	int status;
	int64_t i1=0, i2=0;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else{
		if((0==(status=exp_is_integer( &op->param, &i1))) && (0==(status=exp_is_integer( &op->next->param, &i2)))){
			ret->value.integer=i1 | i2;
			ret->type=T_INTEGER;
			return 0;
		}else{
			return EXP_ER_NONINTEGER;
		}
	}
}

static int operator_evaluate_booland( token_t *op, value_t *ret){
	int status;
	int d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_boolean( &op->param, &d1))) ||(0 !=(status=exp_to_boolean( &op->next->param, &d2)))){
		return status;
	}else{
		ret->value.boolean=d1 && d2;
		ret->type=T_BOOLEAN;
		return 0;
	}
}

static int operator_evaluate_boolor( token_t *op, value_t *ret){
	int status;
	int d1, d2;

	if( NULL==op->next){
		return EXP_ER_INVALARGC;
	}else if((0 !=(status=exp_to_boolean( &op->param, &d1))) ||(0 !=(status=exp_to_boolean( &op->next->param, &d2)))){
		return status;
	}else{
		ret->value.boolean=d1 || d2;
		ret->type=T_BOOLEAN;
		return 0;
	}
}

static int operator_evaluate_equals( token_t *op, value_t *ret){
	return operator_evaluate_boolequals( op, ret);
}



typedef int operator_f( token_t *, value_t *);

int exp_eval_operator( token_t **stack, operator_t operator, int *stack_len){
	token_t *s=*stack;
	token_t *temp, *opqueue, *result;
	int status;
	int c;
	operator_f *f;


	switch( operator) {
		case O_BOOLNOT:    c=1; f=operator_evaluate_boolnot; break;
		case O_BITNOT:     c=1; f=operator_evaluate_bitnot; break;
		case O_UMINUS:     c=1; f=operator_evaluate_uminus; break;
		case O_UPLUS:      c=1; f=operator_evaluate_uplus; break;

		case O_EQUALS:     c=2; f=operator_evaluate_equals; break;
		case O_HAT:        c=2; f=operator_evaluate_hat; break;
		case O_DIV:        c=2; f=operator_evaluate_div; break;
		case O_MOD:        c=2; f=operator_evaluate_mod; break;
		case O_MUL:        c=2; f=operator_evaluate_mul; break;
		case O_PLUS:       c=2; f=operator_evaluate_plus; break;
		case O_MINUS:      c=2; f=operator_evaluate_minus; break;
		case O_SHIFTLEFT:  c=2; f=operator_evaluate_shiftleft; break;
		case O_SHIFTRIGHT: c=2; f=operator_evaluate_shiftright; break;
		case O_GT:         c=2; f=operator_evaluate_gt; break;
		case O_LT:         c=2; f=operator_evaluate_lt; break;
		case O_GE:         c=2; f=operator_evaluate_ge; break;
		case O_LE:         c=2; f=operator_evaluate_le; break;
		case O_NOTEQUALS:  c=2; f=operator_evaluate_notequals; break;
		case O_BOOLEQUALS: c=2; f=operator_evaluate_boolequals; break;
		case O_BITAND:     c=2; f=operator_evaluate_bitand; break;
		case O_BITOR:      c=2; f=operator_evaluate_bitor; break;
		case O_BOOLAND:    c=2; f=operator_evaluate_booland; break;
		case O_BOOLOR:     c=2; f=operator_evaluate_boolor; break;
		default:
			return EXP_ER_INVALOPERATOR;
    }

	if(*stack_len <c){
		return EXP_ER_INVALARGC;

	}else{
		(*stack_len)-=c;
		opqueue=NULL;
		while( c){
			temp=s;
			s=s->next;

			temp->next=opqueue;
			opqueue=temp;

			c--;
		}
		if(NULL==(result=calloc( 1, sizeof(token_t)))){
			exp_token_free(opqueue);
			*stack=s;
			return EXP_ER_NOMEM;
		}

		if(0 !=(status=(*f)( opqueue, &result->param))){
			exp_token_free(opqueue);
			exp_token_free(result);
			*stack=s;
			return status;

		}else{
			int64_t r;

			if( result->param.type==T_REAL && 0==exp_is_integer( &result->param, &r)){
				result->param.type=T_INTEGER;
				result->param.value.integer=r;
			}
			exp_token_free(opqueue);
			result->position=0;
			result->next=s;
			*stack=result;
			(*stack_len)+=1;
			return 0;
		}
	}
}





