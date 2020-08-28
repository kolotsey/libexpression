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
 * Definitions of various math/string/logical functions.
 * These functions are available to use in expressions that are processed by
 * libexpression library.
 * Function exp_call_function() is called from rpn.c.
 */


#include "libexpression-private.h"


/**
 * @file functions.c
 * This file implements all embedded functions that are avaiable to use in
 * expressions that are processed by libexpression.
 * @ingroup libexpression
 * @{
 */


/**
 * @page embedded_function Embedded Functions Overview
 *
 * @section embedded_functions_overview Embedded Functions in libexpression
 *
 * @c Libexpression library contains several embedded functions that can be used
 * in expressions. When @c libexpression locates a function in an expression, it
 * tries to find the implementation of this function in the list of embedded
 * functions. If @c libexpression can not find an implementation of the function
 * with given name, it calls a callback. The callback is implemented in code
 * that uses this library.
 *
 * You can implement any required function in your callback. See
 * exp_set_function_handler() for more information.
 *
 * Almost all function that are available in @c libexpression are
 * implemented in standard @c c libraries, such as libm. The @c libexpression
 * library only defines interfaces for these functions.
 *
 *
 */


/**
 * @page math_functions Math functions
 *
 * In this section all available embedded math functions are documented. These
 * functions can be used in expressions.
 *
 * Almost all math function that are available in @c libexpression are
 * implemented in @c libm C library. The @c libexpression
 * library only defines interfaces for these functions.
 */

/**
 * @page math_functions
 * @section abs abs(x)
 *
 * The @c abs(a) function returns the absolute value of the integer or double
 * argument @a @c x.
 */
static int call_abs( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		ret->value.real=fabs(d1);
		//ret->value.real=d1<0? -d1 : d1;
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page math_functions
 * @section acos acos(x)
 *
 * This function computes the arccosine (specified in radians) of an argument
 * and returns its value in the range (0, PI). The argument of the function
 * should be a number in the range (-1, 1). The function may return
 * @c EXP_ER_TRIGONOMETRIC error if computation is not possible.
 */
static int call_acos( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		if( d1<-1 || d1>1){
			return EXP_ER_TRIGONOMETRIC;
		}else{
			ret->value.real=acos( d1);
			ret->type=T_REAL;
			return 0;
		}
	}
}

/**
 * @page math_functions
 * @section asin asin(x)
 *
 * This function computes the arcsine (specified in radians) of an argument
 * and returns its value in the range (-PI/2, PI/2). The argument of the function
 * should be a number in the range (-1, 1). The function may return
 * @c EXP_ER_TRIGONOMETRIC error if computation is not possible.
 */
static int call_asin( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		if( d1<-1 || d1>1){
			return EXP_ER_TRIGONOMETRIC;
		}else{
			ret->value.real=asin( d1);
			ret->type=T_REAL;
			return 0;
		}
	}
}

/**
 * @page math_functions
 * @section atan atan(x)
 *
 * This function computes the arctangent (specified in radians) of an argument
 * and returns its value in the range (-PI/2, PI/2).
 *
 */
static int call_atan( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		ret->value.real=atan( d1);
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page math_functions
 * @section atan2 atan2(y, x)
 *
 * This function computes the value of the arctangent (specified in radians)
 * of @c y/x, using the signs of both arguments to determine the quadrant of the
 * return value. A @c EXP_ER_DIVBYZERO error occurs if @a @c y is zero.
 *
 */
static int call_atan2( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next==NULL) return EXP_ER_INVALARGCLOW;
	if( op->next->next) return EXP_ER_INVALARGCHIGH;

	if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		return status;
	}else{
		if( d1 !=0 && d2==0){
			return EXP_ER_DIVBYZERO;
		}else{
			ret->value.real=atan2( d1, d2);
			ret->type=T_REAL;
			return 0;
		}
	}
}


/**
 * @page math_functions
 * @section ceil ceil(x)
 *
 * The @c ceil() function rounds the value of @a @c x up to the next integer (rounding
 * towards the "ceiling").
 *
 */
static int call_ceil( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		if(d1<INT64_MIN || d1>INT64_MAX){
			return EXP_ER_INTOVERFLOW;
		}else{
			ret->value.integer=(int64_t)ceil(d1);
			ret->type=T_INTEGER;
			return 0;
		}
	}
}

/**
 * @page math_functions
 * @section cos cos(a)
 *
 * This function computes the cosine of @a @c a (specified in radians).
 *
 */
static int call_cos( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		ret->value.real=cos( d1);
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page math_functions
 * @section cosh cosh(a)
 *
 * This function computes the hyperbolic cosine (specified in radians) of @a @c a.
 * A @c EXP_ER_TRIGONOMETRIC error occurs if the magnitude of @a @c a is too large
 */
static int call_cosh( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		errno=0;
		d1=cosh( d1);
		if( errno !=0){
			return EXP_ER_TRIGONOMETRIC;
		}
		ret->value.real=d1;
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page math_functions
 * @section exp exp(a)
 *
 * The @c exp() function computes the exponential function of @a @c a (e^a).
 */
static int call_exp( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		ret->value.real=exp( d1);
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page math_functions
 * @section floor floor(x)
 *
 * This function computes the largest integer <= @a @c x (rounding towards the "floor").
 */
static int call_floor( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		if(d1<INT64_MIN || d1>INT64_MAX){
			return EXP_ER_INTOVERFLOW;
		}else{
			ret->value.integer=(int64_t)floor(d1);
			ret->type=T_INTEGER;
			return 0;
		}
	}
}


/**
 * @page math_functions
 * @section fmod fmod(x, y)
 *
 * The @c fmod() function computes the floating-point residue of @a @c x (mod @a @c y),
 * which is the remainder of @c x/y, even if the quotient @c x/y isn't representable.
 * If @a @c y is zero, the function returns 0.
 */
static int call_fmod( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next==NULL) return EXP_ER_INVALARGCLOW;
	if( op->next->next) return EXP_ER_INVALARGCHIGH;

	if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
		return status;
	}else{
		errno=0;
		d1=atan2( d1, d2);
		if( 0 !=errno){
			return EXP_ER_INVALARGV;
		}else{
			ret->value.real=d1;
			ret->type=T_REAL;
			return 0;
		}
	}
}


/**
 * @page math_functions
 * @section log log(x)
 *
 * The @c log() function computes the natural logarithm (base @c e) of @a @c x.
 * A @c EXP_ER_COMPLEX error occurs if @a @c x is not positive.
 */
static int call_log( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		if( d1<=0){
			return EXP_ER_COMPLEX;
		}else{
			ret->value.real=log(d1);
			ret->type=T_REAL;
			return 0;
		}
	}
}

/**
 * @page math_functions
 * @section log10 log10(x)
 *
 * The @c log10() function computes the base 10 logarithm of @a @c x.
 * A @c EXP_ER_COMPLEX error occurs if @a @c x is not positive.
 */
static int call_log10( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		if( d1<=0){
			return EXP_ER_COMPLEX;
		}else{
			ret->value.real=log10(d1);
			ret->type=T_REAL;
			return 0;
		}
	}
}

/**
 * @page math_functions
 * @section min min(a,...)
 *
 * The @c min() function returns the minimum argument from the list of supplied
 * arguments. The @c min() function accepts one or more arguments.
 */
static int call_min( token_t *op, value_t *ret){
	int status;
	double d1, min;

	if(NULL==op) return EXP_ER_INVALARGCLOW;

	if((0 !=(status=exp_to_double( &op->param, &d1)))){
		return status;
	}
	min=d1;
	op=op->next;
	while( op){
		if((0 !=(status=exp_to_double( &op->param, &d1)))){
			return status;
		}
		min=d1<min? d1 : min;
		op=op->next;
	}

	ret->value.real=min;
	ret->type=T_REAL;
	return 0;
}

/**
 * @page math_functions
 * @section max max(a,...)
 *
 * The @c max() function returns the maximum argument from the list of supplied
 * arguments. The @c max() function acepts one or more arguments.
 */
static int call_max( token_t *op, value_t *ret){
	int status;
	double d1, max;

	if(NULL==op) return EXP_ER_INVALARGCLOW;

	if((0 !=(status=exp_to_double( &op->param, &d1)))){
		return status;
	}
	max=d1;
	op=op->next;
	while( op){
		if((0 !=(status=exp_to_double( &op->param, &d1)))){
			return status;
		}
		max=d1>max? d1 : max;
		op=op->next;
	}

	ret->value.real=max;
	ret->type=T_REAL;
	return 0;
}

/**
 * @page math_functions
 * @section pow pow(x, y)
 *
 * The @c pow() function computes @a @c x raised to the power of @a @c y.
 * A @c EXP_ER_DIVBYZERO error occurs if @a @c x = 0, and @a @c y <= 0, or if @a @c x is
 * negative, and @a @c y isn't an integer.
 */
static int call_pow( token_t *op, value_t *ret){
	int status;
	double d1, d2;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next==NULL) return EXP_ER_INVALARGCLOW;
	if( op->next->next) return EXP_ER_INVALARGCHIGH;

	if((0 !=(status=exp_to_double( &op->param, &d1))) ||(0 !=(status=exp_to_double( &op->next->param, &d2)))){
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


/**
 * @page math_functions
 * @section random random(a, b)
 *
 * Generate a pseudo-random number.
 * The @c random() function uses implementation of @c random() from @c libm
 * library. The function returns a pseudo-random number in the range [0, 1) if no
 * arguments were supplied. The function returns a pseudo-random number in the
 * range [0, a). If only one argument @a @c a was supplied. The function returns
 * a pseudo-random number in the range [a, b). If two argument @a @c a and @a @c b
 * were supplied.
 *
 * The function does not start a new sequence of pseudo-random integers when
 * called. That is, a program code that uses @c libexpression library should
 * call @c srand() standard C function when initialising libexpression.
 *
 * The function @c rand(a, b) is an alias to this function.
 */
static int call_random( token_t *op, value_t *ret){
	if(op){
		double rmin, rmax;
		int status;

		if( op->next){
			//two arguments: low and high borders
			if(op->next->next) return EXP_ER_INVALARGCHIGH;
			if(0 !=(status=exp_to_double( &op->param, &rmin)) ||
					0 !=(status=exp_to_double( &op->next->param, &rmax))){
				return status;
			}else{
				ret->value.real=rmin+(double)rand()*(rmax-rmin)/(double)RAND_MAX;
				ret->type=T_REAL;
				return 0;
			}

		}else{
			//one argument: high border
			if(0 !=(status=exp_to_double( &op->param, &rmax))){
				return status;
			}else{
				ret->value.real=(double)rand()*rmax/(double)RAND_MAX;
				ret->type=T_REAL;
				return 0;
			}
		}

	}else{
		ret->value.real=(double)rand()/(double)RAND_MAX;
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page math_functions
 * @section round round(x)
 *
 * The @c round() function returns the closest integer to @a @c x.
 *
 */
static int call_round( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		if(d1<INT64_MIN || d1>INT64_MAX){
			return EXP_ER_INTOVERFLOW;
		}else{
			ret->value.integer=(int64_t)round(d1);
			ret->type=T_INTEGER;
			return 0;
		}
	}
}

/**
 * @page math_functions
 * @section sin sin(a)
 *
 * The @c sin() function computes the sine (specified in radians) of @a @c a.
 *
 */
static int call_sin( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		ret->value.real=sin( d1);
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page math_functions
 * @section sinh sinh(a)
 *
 * The @c sinh() function computes the hyperbolic sine (specified in radians) of
 * @a @c a. A @c EXP_ER_TRIGONOMETRIC error occurs if the magnitude of @a @c a is too large.
 *
 */
static int call_sinh( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		errno=0;
		d1=sinh( d1);
		if( errno !=0){
			return EXP_ER_TRIGONOMETRIC;
		}
		ret->value.real=d1;
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page math_functions
 * @section sqr sqr(x)
 *
 * The @c sqr() function computes the square of an argument.
 *
 */
static int call_sqr( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		ret->value.real=d1 * d1;
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page math_functions
 * @section sqrt sqrt(x)
 *
 * The function computes the nonnegative square root of @a @c x.
 * A @c EXP_ER_COMPLEX error occurs if the argument is negative.
 *
 */
static int call_sqrt( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		if( d1<0){
			return EXP_ER_COMPLEX;
		}else{
			ret->value.real=sqrt(d1);
			ret->type=T_REAL;
			return 0;
		}
	}
}

/**
 * @page math_functions
 * @section tan tan(a)
 *
 * The function computes the tangent (specified in radians) of @a @c a.
 *
 */
static int call_tan( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		ret->value.real=tan( d1);
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page math_functions
 * @section tanh tanh(a)
 *
 * These functions compute the hyperbolic tangent (specified in radians) of @a @c a.
 *
 */
static int call_tanh( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		errno=0;
		d1=tanh( d1);
		if( errno !=0){
			return EXP_ER_TRIGONOMETRIC;
		}
		ret->value.real=d1;
		ret->type=T_REAL;
		return 0;
	}
}


/**
 * @page conversion_functions Conversion Functions
 *
 * This section contains all embedded conversion functions implemented in
 * @c libexpression that could be used in expressions.
 *
 */


/**
 * @page conversion_functions
 * @section bin2dec bin2dec(s)
 *
 * The function converts a binary representation of a number to an integer.
 * The argument supplied to the function should be a string.
 *
 * The easier way to use binary numbers in expression is to use a special binary
 * notation to represent a number. Binary notation uses prefix @c 0b to represent
 * the numbers, e.g. @code 0b101 @endcode stands for 5.
 */
static int call_bin2dec( token_t *op, value_t *ret){
	uint64_t result;
	char *s;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(op->param.type !=T_STRING) return EXP_ER_NONSTRING;

	result=0;
	if( op->param.value.string){
		s=op->param.value.string;
		while( *s){
			if( *s=='0' || *s=='1'){
				result=(result<<1) | (*s=='1'? 1 : 0);
				if( result>INT64_MAX){
					return EXP_ER_INTOVERFLOW;
				}
			}else{
				return EXP_ER_INVALARGV;
			}
			s++;
		}
	}
	ret->type=T_INTEGER;
	ret->value.integer=(int64_t)result;
	return 0;
}

/**
 * @page conversion_functions
 * @section boolean boolean(b)
 *
 * The function converts an argument to a boolean value.
 * If the argument could not be represented as boolean, @c EXP_ER_NONBOOLEAN is
 * returned.
 *
 * The function @c bool() is an alias to this function.
 */
static int call_boolean( token_t *op, value_t *ret){
	int status;
	int b1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_boolean( &op->param, &b1))){
		return status;
	}else{
		ret->value.boolean=b1;
		ret->type=T_BOOLEAN;
		return 0;
	}
}

/**
 * @page conversion_functions
 * @section dec2bin dec2bin(i)
 *
 * The function returns a string containing a binary representation of an argument.
 */

static int call_dec2bin( token_t *op, value_t *ret){
	uint64_t i1, c;
	int64_t i;
	char s[128];

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if( 0!=exp_is_integer( &op->param, &i)) return EXP_ER_NONINTEGER;
	i1=(uint64_t) i;
	c=0;
	do{
		s[c++]=(i1 & 0x01? '1' : '0');
		i1>>=1;
	}while( i1);

	s[c]=0;
	if(NULL==( ret->value.string=malloc( c+1))) return EXP_ER_NOMEM;
	ret->type=T_STRING;
	for( i=0; i<=c; i++){
		ret->value.string[i]=s[c-i-1];
	}
	ret->value.string[c]=0;
	return 0;
}

/**
 * @page conversion_functions
 * @section dec2hex dec2hex(i)
 *
 * The function returns a string containing a hexademical representation of an argument.
 */
static int call_dec2hex( token_t *op, value_t *ret){
	uint64_t i1, c;
	int64_t i;
	char s[128];

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if( 0!=exp_is_integer( &op->param, &i)) return EXP_ER_NONINTEGER;
	i1=(uint64_t) i;
	c=0;
	do{
		if((i1 & 0x0f) <10){
			s[c++]=(i1 & 0x0f)+'0';
		}else{
			switch( i1 & 0x0f){
				case 0x0a: s[c++]='a'; break;
				case 0x0b: s[c++]='b'; break;
				case 0x0c: s[c++]='c'; break;
				case 0x0d: s[c++]='d'; break;
				case 0x0e: s[c++]='e'; break;
				case 0x0f: s[c++]='f'; break;
				default:   s[c++]='0'; break;
			}
		}
		i1=i1>>4;
	}while( i1);

	s[c]=0;
	if(NULL==( ret->value.string=malloc( c+1))) return EXP_ER_NOMEM;
	ret->type=T_STRING;
	for( i=0; i<=c; i++){
		ret->value.string[i]=s[c-i-1];
	}
	ret->value.string[c]=0;
	return 0;
}

/**
 * @page conversion_functions
 * @section dec2oct dec2oct(i)
 *
 * The function returns a string containing a octal representation of an argument.
 */
static int call_dec2oct( token_t *op, value_t *ret){
	uint64_t i1, c;
	int64_t i;
	char s[128];

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if( 0!=exp_is_integer( &op->param, &i)) return EXP_ER_NONINTEGER;
	i1=(uint64_t) i;
	c=0;
	do{
		s[c++]=(i1 & 0x07)+'0';
		i1=i1>>3;
	}while( i1);

	s[c]=0;
	if(NULL==( ret->value.string=malloc( c+1))) return EXP_ER_NOMEM;
	ret->type=T_STRING;
	for( i=0; i<=c; i++){
		ret->value.string[i]=s[c-i-1];
	}
	ret->value.string[c]=0;
	return 0;
}

/**
 * @page conversion_functions
 * @section float float(f)
 *
 * The function converts an argument to a floating point number.
 * If the argument could not be converted to float, @c EXP_ER_NONNUMERIC is
 * returned.
 *
 * The function @c double() is an alias to this function.
 */
static int call_double( token_t *op, value_t *ret){
	int status;
	double d1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_double( &op->param, &d1))){
		return status;
	}else{
		ret->value.real=d1;
		ret->type=T_REAL;
		return 0;
	}
}

/**
 * @page conversion_functions
 * @section hex2dec hex2dec(s)
 *
 * The function converts a string with hexademical representation of a number
 * to a number.
 *
 * The easier way to use hexademical numbers in expression is to use a special hex
 * notation to represent a number. Hex notation uses prefix @c 0x to represent
 * the numbers, e.g. @code 0xff @endcode stands for 255.
 */
static int call_hex2dec( token_t *op, value_t *ret){
	uint64_t result;
	char *s;
	int i;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(op->param.type !=T_STRING) return EXP_ER_NONSTRING;

	result=0;
	if( op->param.value.string){
		s=op->param.value.string;
		while( *s){
			i=IS_HEXADEMICAL(*s);
			if( -1==i){
				return EXP_ER_INVALARGV;
			}else{
				result=result*16+i;
				if( result>INT64_MAX){
					return EXP_ER_INTOVERFLOW;
				}
			}
			s++;
		}
	}
	ret->type=T_INTEGER;
	ret->value.integer=(int64_t)result;
	return 0;
}

/**
 * @page conversion_functions
 * @section integer integer(i)
 *
 * The function converts an argument to an integer.
 * If the argument could not be converted to integer, @c EXP_ER_NONNUMERIC is
 * returned.
 *
 * The function @c int() is an alias to this function.
 */
static int call_integer( token_t *op, value_t *ret){
	int status;
	int64_t i1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_integer( &op->param, &i1))){
		return status;
	}else{
		ret->value.integer=i1;
		ret->type=T_INTEGER;
		return 0;
	}
}

/**
 * @page conversion_functions
 * @section oct2dec oct2dec(s)
 *
 * The function converts a string containing a octal representation of a number
 * to integer.
 *
 * The easier way to use octal numbers in expressions is to use a special octal
 * notation to represent a number. Octal notation uses prefix @c 0o to represent
 * the numbers, e.g. @code 0o777 @endcode stands for 511.
 */
static int call_oct2dec( token_t *op, value_t *ret){
	uint64_t result;
	char *s;
	int i;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(op->param.type !=T_STRING) return EXP_ER_NONSTRING;

	result=0;
	if( op->param.value.string){
		s=op->param.value.string;
		while( *s){
			i=IS_OCTAL(*s);
			if( -1==i){
				return EXP_ER_INVALARGV;
			}else{
				result=result*8+i;
				if( result>INT64_MAX){
					return EXP_ER_INTOVERFLOW;
				}
			}
			s++;
		}
	}
	ret->type=T_INTEGER;
	ret->value.integer=(int64_t)result;
	return 0;
}

/**
 * @page conversion_functions
 * @section string string(s)
 *
 * The function converts an argument to a string.
 *
 * The function @c str() is an alias to this function.
 */
static int call_string( token_t *op, value_t *ret){
	int status;
	char *s1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_string( &op->param, &s1))){
		return status;
	}else{
		ret->value.string=s1;
		ret->type=T_STRING;
		return 0;
	}
}

/**
 * @page string_functions String Functions
 *
 * This section contains all embedded string functions implemented in
 * @c libexpression that could be used in expressions.
 *
 */

/**
 * @page string_functions
 * @section ltrim ltrim(s)
 *
 * The function strips whitespace from the beginning of a string.
 *
 */
static int call_ltrim( token_t *op, value_t *ret){
	int status;
	char *s1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_string( &op->param, &s1))){
		return status;
	}else{
		char *s2=s1;
		while( *s2 && IS_BLANK( *s2)){
			s2++;
		}
		memmove(s1, s2, strlen( s2)+1);
		ret->value.string=s1;
		ret->type=T_STRING;
		return 0;
	}
}

/**
 * @page string_functions
 * @section rtrim rtrim(s)
 *
 * The function strips whitespace from the end of a string.
 *
 */
static int call_rtrim( token_t *op, value_t *ret){
	int status;
	char *s1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_string( &op->param, &s1))){
		return status;
	}else{
		char *s2=s1+strlen(s1);
		while( s2>s1 && IS_BLANK( *(s2-1))){
			s2--;
		}
		*s2=0;
		ret->value.string=s1;
		ret->type=T_STRING;
		return 0;
	}
}

/**
 * @page string_functions
 * @section strcasecmp strcasecmp(s1, s2)
 *
 * The function compares two strings case-insensitively and returns 0 if they are equal.
 *
 */
static int call_strcasecmp( token_t *op, value_t *ret){
	int status;
	char *s1, *s2;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if(NULL==op->next) return EXP_ER_INVALARGCLOW;
	if( op->next->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_string( &op->param, &s1))){
		return status;
	}else{
		if(0 !=(status=exp_to_string( &op->param, &s2))){
			free( s1);
			return status;
		}else{
			ret->value.boolean = (0==strcasecmp( s1, s2)? 1 : 0);
			ret->type=T_BOOLEAN;
			free( s1);
			free( s2);
			return 0;
		}
	}
}

/**
 * @page string_functions
 * @section strcmp strcmp(s1, s2)
 *
 * The function compares two strings and returns 0 if they are equal.
 *
 * The easier way to compare strings in equations is to use equals operator,
 * e.g. @code "Hello world"=="Hello"+" world" @endcode
 *
 */
static int call_strcmp( token_t *op, value_t *ret){
	int status;
	char *s1, *s2;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if(NULL==op->next) return EXP_ER_INVALARGCLOW;
	if( op->next->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_string( &op->param, &s1))){
		return status;
	}else{
		if(0 !=(status=exp_to_string( &op->param, &s2))){
			free( s1);
			return status;
		}else{
			ret->value.integer = strcmp( s1, s2);
			ret->type=T_INTEGER;
			free( s1);
			free( s2);
			return 0;
		}
	}
}

/**
 * @page string_functions
 * @section strlen strlen(s)
 *
 * The function returns the length of a string.
 *
 */
static int call_strlen( token_t *op, value_t *ret){
	int status;
	char *s1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_string( &op->param, &s1))){
		return status;
	}else{
		ret->value.integer=strlen( s1);
		ret->type=T_INTEGER;
		free( s1);
		return 0;
	}
}

/**
 * @page string_functions
 * @section strtolower strtolower(s)
 *
 * The function returns string with all alphabetic characters converted to lowercase.
 *
 * The functions @c strlwr() and @c tolower() are aliases to this function.
 */
static int call_strtolower( token_t *op, value_t *ret){
	int status;
	char *s1;
	int i, len;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_string( &op->param, &s1))){
		return status;
	}else{
		len=strlen(s1);
		for( i=0; i<len; i++){
			s1[i]=tolower(s1[i]);
		}
		ret->value.string=s1;
		ret->type=T_STRING;
		return 0;
	}
}

/**
 * @page string_functions
 * @section strtoupper strtoupper(s)
 *
 * The function returns string with all alphabetic characters converted to uppercase.
 *
 * The functions @c strupr() and @c toupper() are aliases to this function.
 */
static int call_strtoupper( token_t *op, value_t *ret){
	int status;
	char *s1;
	int i, len;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_string( &op->param, &s1))){
		return status;
	}else{
		len=strlen(s1);
		for( i=0; i<len; i++){
			s1[i]=toupper(s1[i]);
		}
		ret->value.string=s1;
		ret->type=T_STRING;
		return 0;
	}
}

/**
 * @page string_functions
 * @section capitalise capitalise(s)
 *
 * The function returns capitalised string with first letter converted to uppercase
 */
static int call_capitalise( token_t *op, value_t *ret){
	int status;
	char *s1;
	int i, len;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_string( &op->param, &s1))){
		return status;
	}else{
		len=strlen(s1);
		if( len){
			s1[0]=toupper(s1[0]);
			for( i=1; i<len; i++){
				s1[i]=tolower(s1[i]);
			}
		}
		ret->value.string=s1;
		ret->type=T_STRING;
		return 0;
	}
}

/**
 * @page string_functions
 * @section substr substr(string, start, length)
 *
 * Returns the portion of @a @c string specified by @a @c start and @a @c length.
 *
 * If @a @c length is not supplied, the maximum possible length is used.
 *
 * If @a @c start is non-negative, the returned string will start at the @a @c start'th
 * position in @a @c string, counting from zero. If @a @c start is negative, the
 * returned string will start at the @a @c start'th character from the end of
 * @a @c string.
 *
 * The function @c substring() is an alias to this function.
 *
 */
static int call_substr( token_t *op, value_t *ret){
	int status;
	char *s1;
	int64_t start;
	int string_length;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if(NULL==op->next) return EXP_ER_INVALARGCLOW;
	if( NULL==op->next->next){
		// Two arguments, substr( string, start)
		if(0 !=(status=exp_to_string( &op->param, &s1))){
			return status;
		}else{
			if(0 !=(status=exp_to_integer( &op->next->param, &start))){
				free( s1);
				return status;
			}else{
				string_length=strlen( s1);
				if( start>string_length){
					free( s1);
					return EXP_ER_INVALARGV;
				}else if( start>=0){
					memmove( s1, s1+start, string_length-start+1);
				}else if( start<-string_length){
					free( s1);
					return EXP_ER_INVALARGV;
				}else{
					memmove( s1, s1+string_length+start, -start+1);
				}
				ret->value.string=s1;
				ret->type=T_STRING;
				return 0;
			}
		}
	}else{
		// Three arguments, substr( string, start, length)
		if( op->next->next->next) return EXP_ER_INVALARGCHIGH;
		if(0 !=(status=exp_to_string( &op->param, &s1))){
			return status;
		}else{
			if(0 !=(status=exp_to_integer( &op->next->param, &start))){
				free( s1);
				return status;
			}else{
				int64_t length;
				if(0 !=(status=exp_to_integer( &op->next->next->param, &length))){
					free( s1);
					return status;
				}else{
					string_length=strlen( s1);
					if( length<0){
						free( s1);
						return EXP_ER_INVALARGV;
					}else if( start>string_length){
						free( s1);
						return EXP_ER_INVALARGV;
					}else if( start>=0){
						if( length>string_length-start){
							memmove( s1, s1+start, string_length-start+1);
						}else{
							memmove( s1, s1+start, length);
							s1[length]=0;
						}
					}else if(start < -string_length){
						free( s1);
						return EXP_ER_INVALARGV;
					}else{
						if( length>-start){
							memmove( s1, s1+string_length+start, -start+1);
						}else{
							memmove( s1, s1+string_length+start, length);
							s1[length]=0;
						}
					}
					ret->value.string=s1;
					ret->type=T_STRING;
					return 0;
				}
			}
		}
	}
}

/**
 * @page string_functions
 * @section trim trim(s)
 *
 * The function strips whitespace from the beginning and end of a string.
 *
 */
static int call_trim( token_t *op, value_t *ret){
	int status;
	char *s1;

	if(NULL==op) return EXP_ER_INVALARGCLOW;
	if( op->next) return EXP_ER_INVALARGCHIGH;

	if(0 !=(status=exp_to_string( &op->param, &s1))){
		return status;
	}else{
		char *s2=s1+strlen(s1);
		while( s2>s1 && IS_BLANK( *(s2-1))){
			s2--;
		}
		*s2=0;
		s2=s1;
		while( *s2 && IS_BLANK( *s2)){
			s2++;
		}
		memmove(s1, s2, strlen( s2)+1);
		ret->value.string=s1;
		ret->type=T_STRING;
		return 0;
	}
}


/**
 * @cond
 * This section is not processed by Doxygen
 */

static struct function_table_s{
	char *name;
	int (*func)( token_t *, value_t *);
} function_table[]={
	/*
	 * Math functions
	 */
	{ "abs",    call_abs},
	{ "acos",   call_acos},
	{ "asin",   call_asin},
	{ "atan",   call_atan},
	{ "atan2",  call_atan2},
	{ "ceil",   call_ceil},
	{ "cos",    call_cos},
	{ "cosh",   call_cosh},
	{ "exp",    call_exp},
	{ "floor",  call_floor},
	{ "fmod",   call_fmod},
	{ "log",    call_log},
	{ "log10",  call_log10},
	{ "min",    call_min},
	{ "max",    call_max},
	{ "pow",    call_pow},
	{ "rand",   call_random},
	{ "random", call_random},
	{ "round",  call_round},
	{ "sin",    call_sin},
	{ "sinh",   call_sinh},
	{ "sqr",    call_sqr},
	{ "sqrt",   call_sqrt},
	{ "tan",    call_tan},
	{ "tanh",   call_tanh},
	/*
	 * Conversion functions
	 */
	{ "bin2dec", call_bin2dec},
	{ "bool",    call_boolean},{ "boolean", call_boolean},
	{ "dec2bin", call_dec2bin},
	{ "dec2hex", call_dec2hex},
	{ "dec2oct", call_dec2oct},
	{ "float",   call_double}, { "double",  call_double},
	{ "hex2dec", call_hex2dec},
	{ "integer", call_integer},{ "int",     call_integer},
	{ "oct2dec", call_oct2dec},
	{ "string",  call_string}, { "str",     call_string},
	/*
	 * String functions
	 */
	{ "ltrim",      call_ltrim},
	{ "rtrim",      call_rtrim},
	{ "strcasecmp", call_strcasecmp},
	{ "strcmp",     call_strcmp},
	{ "strlen",     call_strlen},
	{ "strtolower", call_strtolower}, { "strlwr",     call_strtolower}, { "tolower",    call_strtolower}, { "lowercase",    call_strtolower},
	{ "strtoupper", call_strtoupper}, { "strupr",     call_strtoupper}, { "toupper",    call_strtoupper}, { "upeercase",    call_strtoupper},
	{ "capitalise", call_capitalise},
	{ "substr",     call_substr},
	{ "substring",  call_substr},
	{ "trim",       call_trim},
};

int exp_call_function( expression_t *exp, char *fname, int argc, token_t **stack, int *stack_len){
	token_t *opqueue, *temp, *s, *result;
	int status;
	int64_t r;
	int c;
	int cnt;

	(*stack_len)-=argc;
	opqueue=NULL;
	s=*stack;
	c=argc;
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

	c=0;
	cnt=sizeof( function_table)/sizeof( function_table[0]);
	while( c<cnt){
		if( 0==strcmp( function_table[c].name, fname)){
			status=function_table[c].func( opqueue, &result->param);
			break;
		}
		c++;
	}

	if( c==cnt){
		// There is no entry in functions table for this function: it is not
		// defined in this module. It is possible, that the function is defined
		// in user-space, i.e. it is available by function handler.
		if( exp->fhandler){
			exp_value_t *values;
			exp_value_t exv;
			if( NULL==(values=calloc( 1, sizeof( exp_value_t) * argc))){
				status=EXP_ER_NOMEM;
			}else{
				temp=opqueue;
				for( c=0; c< argc; c++){
					exp_value_t *v=&(values[c]);
					EXPORT_FROM_VALUE_T( &temp->param, v);
					temp=temp->next;
				}
				status=exp->fhandler( exp->user_data, fname, argc, values, &exv);
				if(0==status){
					IMPORT_TO_VALUE_T( &exv, &result->param);
					if( exv.type==EXP_STRING && exv.value.string){
						free(exv.value.string);
					}
					if( result->param.type==T_NONE){
						status=EXP_ER_INVALRET;
					}
				}else{
					if( status==1){
						status = EXP_ER_INVALFUNC;
					}else{
						status = EXP_ER_USERFUNCERROR;
					}
				}
				for( c=0; c< argc; c++){
					if( values[c].type==EXP_STRING && values[c].value.string){
						free(values[c].value.string);
					}
				}
				free(values);
			}

		}else{
			status = EXP_ER_INVALFUNC;
		}
	}

	if( status !=0){
		exp_token_free(opqueue);
		exp_token_free(result);
		*stack=s;
		return status;
	}else{
		if( result->param.type==T_REAL && (-0==result->param.value.real)){
			result->param.value.real=0;
		}
//		if( result->param.type==T_REAL){
//			result->param.type=T_INTEGER;
//			result->param.value.integer=r;
//		}
		exp_token_free(opqueue);
		result->position=0;
		result->next=s;
		*stack=result;
		(*stack_len)+=1;
		return 0;
	}
}


/** @endcond */
/** @} */
