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
 * API functions definitions.
 */

#ifndef LIBEXPRESSION_H_
#define LIBEXPRESSION_H_



/**
 * @file libexpression.h
 * libexpression - library that provides a mechanism for solving math expressions
 * @ingroup libexpression
 * @defgroup libexpression libexpression
 * @{
 */



/**
 * @mainpage Libexpression Documentation
 *
 * @section Overview
 *
 * Libexpression library provides a mechanism for solving valid math and logical
 * expressions for other programs. It is intended to allow programmers to
 * integrate a user-frielndly calculators/expressions into their programs.
 *
 * Libexpression is based on RPN algorithm with several powerful modifications,
 * such as trinary operators, variable arguments functions and user defined
 * callbacks.
 *
 * Almost all functions in the library are thread-safe. All non thread-safe
 * functions have their thread-safe analogs. Please, refer to the example below
 * to see how to use this library.
 *
 * @section Requirements
 * This library does not require any extra packages.
 *
 * @section Example
 *
 * See file expression.c. To build this file, run
 *    gcc -lexpression -o expression expression.c
 * in console.
 *
 * @section Copiright Copyright notice
 *
 * Libexpression is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Libexpression is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public Licenise
 * along with libexpression. If not, see <http://www.gnu.org/licenses/>.
 *
 */




/**
 * @brief Minimum length of buffer that libexpression routines can use to store
 * error messages.
 *
 * Almost all routines in the library provide a way to receive an error message
 * when they fail. You need to supply a pointer to the buffer where the
 * routines can store their error messages. The length of the supplied buffer
 * must be at least EXP_ERLEN bytes. The error message returned is
 * NULL-terminated.
 */
#define EXP_ERLEN 1024


/**
 * @brief The all possible error codes that could be returned by the API
 * functions.
 *
 * The library function that can fail are designed to return error code,
 * description of occured error and position of the occured error in the
 * expression.
 *
 * All possible error codes that can be returned by API functions are defined.
 */
typedef enum {
	/**
	 * Expression is not well-formed
	 */
	EXP_ER_INVALEXPR=101,
	/**
	 * Invalid number of arguments supplied to operator
	 */
	EXP_ER_INVALARGC,
	/**
	 * Too many arguments passed to function
	 */
	EXP_ER_INVALARGCHIGH,
	/**
	 * Too few arguments passed to function
	 */
	EXP_ER_INVALARGCLOW,
	/**
	 * Invalid function argument
	 */
	EXP_ER_INVALARGV,
	/**
	 * Unknown function
	 */
	EXP_ER_INVALFUNC,
	/**
	 * Unknown parameter
	 */
	EXP_ER_INVALPARAM,
	/**
	 * Invalid operator
	 */
	EXP_ER_INVALOPERATOR,
	/**
	 * Memory error occured while expression was evaluated
	 */
	EXP_ER_NOMEM,
	/**
	 * Complex result when evaluating function
	 */
	EXP_ER_COMPLEX,
	/**
	 * Division by zero
	 */
	EXP_ER_DIVBYZERO,
	/**
	 * Function requires integer operands
	 */
	EXP_ER_NONINTEGER,
	/**
	 * Function requires numeric or boolean operands
	 */
	EXP_ER_NONNUMERIC,
	/**
	 * Function requires boolean operands
	 */
	EXP_ER_NONBOOLEAN,
	/**
	 * Function requires string operands
	 */
	EXP_ER_NONSTRING,
	/**
	 * Overflow occurs when converting argument to integer
	 */
	EXP_ER_INTOVERFLOW,
	/**
	 * Function argument is not in range
	 */
	EXP_ER_TRIGONOMETRIC,
	/**
	 * Unknown type was returned by user defined function or parameter handler
	 */
	EXP_ER_INVALRET,
	/**
	 * Error in user defined function handler
	 */
	EXP_ER_USERFUNCERROR,
}exp_error_t;

/**
 * @brief Possible variable types.
 *
 * The library supports four common types of variables. They are Integer,
 * Floating point, Boolean and String. Any result that is returned by the
 * library can by of any of this four types. What is more, when a callback
 * is called by the library, the library expect user to provide the returned
 * value with one of the defined types.
 */
typedef enum {
	/**
	 * @brief Unknown value type.
	 *
	 * This type is never used. This type is defined, because it might be used
	 * in future.
	 */
	EXP_NONE=0,
	/**
	 * Integer
	 */
	EXP_INTEGER,
	/**
	 * Floating point
	 */
	EXP_REAL,
	/**
	 * Boolean
	 */
	EXP_BOOLEAN,
	/**
	 * NULL-terminated string
	 */
	EXP_STRING,
} exp_value_type_t;

/**
 * @brief Data structure that represents variables (e.g. evaluation results)
 *
 * This structure contains a field that identifies variable type and a union
 * with a separate field for each type. To completely understand the way this
 * structure is used, see the following example. This example evaluates an
 * expression @verbatim random()>0.5? "Ten" : 10 @endverbatim and prints the
 * result. When the result is printed, it is tested for the variable type.
 * @code
#include <stdio.h>
#include <stdlib.h>
#include "libexpression.h"

int main( int argc, char *argv[]){
	expression_t *exp;
	char error[EXP_ERLEN];
	int error_pos;
	exp_error_t ercode;
	exp_value_t *v;

	//Here we define an expression. This expression returns a variable of type
	//String or Integer depending on the result of random().
	//(You have to call srand() before you use built-in random() as you do it in C)
	srand( time(NULL));
	if(NULL==( exp=exp_create( "random()>0.5? 'Ten' : 10", &ercode, error, &error_pos))){
		fprintf( stderr, "Error at %d: %s\n", error_pos, error);
		return 1;
	}
	if(NULL ==(v=exp_solve( exp, &ercode, error, &error_pos))){
		fprintf( stderr, "Error at %d: %s\n", error_pos+1, error);
		return 1;
	}
	exp_free( exp);

	//Now test the result.
	//Depending on the type of the result, print its value.
	if(v->type==EXP_REAL){
		printf( "%f\n", v->value.real);
	}else if(v->type==EXP_BOOLEAN){
		printf( "%s\n", v->value.boolean? "True" : "False");
	}else if(v->type==EXP_STRING){
		printf( "%s\n", v->value.string);
	}else{
		//EXP_INTEGER
		printf( "%lld\n", v->value.integer);
	}

	//Do some cleanup
	exp_value_free( v);
	return 0;
}
@endcode
 */
typedef struct{
	/**
	 * @brief Variable type.
	 */
	exp_value_type_t type;

	union{
		/**
		 * @brief Value for integer variable.
		 */
		long long int integer;
		/**
		 * @brief Value for floating point variable.
		 */
		double real;
		/**
		 * @brief Value for boolean variable
		 */
		int boolean;
		/**
		 * @brief Value for string variable
		 */
		char *string;

	} value; /**< @brief Union with a special field for every type of variable. */
}exp_value_t;

/**
 * @brief A definition of user supplied callback that exp_solve()
 * calls to evaluate unknown functions in an expression.
 * See exp_set_function_handler() for more information.
 */
typedef int exp_function_handler_f( void *user_data, char *function_name, int argc, exp_value_t *argv, exp_value_t *result);

/**
 * @brief A definition of user supplied callback that exp_solve()
 * calls to evaluate unknown parameters or variable names in an expression.
 * See exp_set_parameter_handler() for more information.
 */
typedef int exp_parameter_handler_f( void *user_data, char *parameter_name, exp_value_t *result);

/**
 * @brief Structure with internal libexpression data.
 *
 * The fields of this structure are not important for the user. This structure
 * is a private libexpression structure. The fields of this structure contain
 * information that libexpression use to evaluate expressions.
 */
typedef struct expression_s{
	/**
	 * @brief Pointer to any user supplied data. User may set this field to
	 * point to any value in memory by means of exp_set_user_data().
	 * See exp_set_user_data() and exp_get_user_data().
	 */
	void *user_data;
	/**
	 * @brief Linked list of tokens that expression consist of.
	 */
	void *tokens;
	/**
	 * @brief Pointer to a buffer containing copy of the supplied expression.
	 */
	char *e;//copy of the provided expr
	/**
	 * @brief Pointer to a user supplied callback that exp_solve() calls to
	 * resolve and evaluate unknown functions. See exp_set_function_handler().
	 */
	exp_function_handler_f *fhandler;
	/**
	 * @brief Pointer to a user supplied callback that exp_solve() calls to
	 * resolve unknown parameters. See exp_set_parameter_handler()
	 */
	exp_parameter_handler_f *phandler;
}expression_t;

/**
 * @brief Parse an expression and construct a libexpression structure.
 *
 * The exp_create() routine parses an expression that is supplied to it and
 * checks it for validity. After the expression is parsed, the @c expression_t
 * structure is constructed that contains all the information about the
 * expression. The structure then should be passed to exp_solve().
 *
 * exp_create() explodes the expression into tokens, checks it for validity and
 * converts into Reverse Polish Notation form (RPN) using an implementation of
 * Shunting-yard algorithm. When converting, exp_create() makes all parameters
 * and function names lowercase.
 *
 * After the RPN form is prepared, exp_create() allocates a new
 * @c expression_t structure. exp_create() then places all the required
 * information about the expression (including its RPN representaton), into the
 * structure. After this routine is done, the pointer to a newly created
 * structure is returned.
 *
 *
 * See example on the main page or in file expression.c for more information.
 *
 * @param e Pointer to a buffer containing NULL-terminated string with the
 *    expression that should be evaluated.
 * @param ercode Pointer to an integer where exp_create() can store error code
 *    if error occurs. This value is set to one from the defined list of
 *    possible error codes. See @c exp_error_t.
 * @param error Pointer to a buffer where exp_create() can store error message
 *    if error occurs. This buffer must be previously allocated. The length of
 *    the buffer must be at least EXP_ERLEN bytes long.
 * @param erpos Pointer to an integer value where exp_create() can store
 *    position of the first error in the expression. If the expression is not
 *    valid, this variable is set to an integer value starting from 0. If the
 *    expression is valid, but other error occured (such as memory error), then
 *    this variable is set to -1.
 * @return A structure containing all the information about the expression.
 *    After the expression is evaluated, the data pointed to by this pointer
 *    should be freed with exp_free().
 *
 * @see exp_solve().
 */
expression_t *exp_create( const char *e, exp_error_t *ercode, char *error, int *erpos);

/**
 * @brief Solve the expression and return its result.
 *
 * The exp_solve() routine receives previously created structure
 * of type @c expression_t and solves the expression. exp_solve() uses
 * an improved implementation of RPN algorithm to evaluate expressions.
 *
 * exp_solve() receives a pointer to a structure containing information about
 * the expression. Then exp_solve() tries to substitute all known parameters,
 * such as Pi, True and False. If the parameter could not be substituted
 * (unknown parameter), exp_solve() calls user-supplied callback (when defined)
 * which in its turn tries to substitute the parameters.
 *
 * After all parameters are resolved, exp_solve() calls RPN algorithm. When
 * the algoritm is performed, the exp_solve() evaluates all partial math
 * expressions and functions in the supplied expression. If the function in
 * the expression is unknown, then exp_solve() calls user-supplied callbak (if
 * defined) to evalute them.
 *
 * If the evaluation succeeds, exp_solve() returns pointer to a structure
 * containing result. If the evaluation fails, exp_solve() rdeturns NULL and
 * sets parameters @c ercode, @c error and @c erpos to proper values.
 *
 * See example on the main page or in file expression.c to understand how to
 * use exp_solve().
 *
 * @param exp Pointer to a structure that was returned by exp_create().
 * @param ercode Pointer to an integer where exp_solve() can store error code
 *    if error occurs. This value is set to one from the defined list of
 *    possible error codes. See @c exp_error_t.
 * @param error Pointer to a buffer where exp_solve() can store error message
 *    if error occurs. This buffer must be preallocated. The length of
 *    the buffer must be at least EXP_ERLEN bytes long.
 * @param erpos Pointer to an integer value where exp_solve() can store
 *    position of the first error in the expression. If the expression is not
 *    valid, this variable is set to an integer value starting from 0. If the
 *    expression is valid, but other error occured (such as memory error), then
 *    this variable is set to -1.
 * @return Pointer to a structure containing result. When the value no longer
 *    needed, the data pointed to by the result should be freed with
 *    exp_value_free(). See @c exp_value_t.
 */
exp_value_t *exp_solve( expression_t *exp, exp_error_t *ercode, char *error, int *erpos);

/**
 * @brief Test two expressions for equality.
 *
 * The routine compares two libexpressin structures and returns 0 if the
 * expresions are equal.
 *
 * @param a, b The expressions to compare
 * @return 0 if expressions are equal, non-zero otherwise.
 */
int exp_equals( expression_t *a, expression_t *b);

/**
 * @brief Convert evaluation result to its string representation.
 *
 * The routine converts passed value to a string. exp_value_to_string() checks the type
 * of the value when converts it. exp_value_to_string() automatically allocates static
 * buffer large enough to contain the result.
 *
 * @par Important:
 * <i>exp_value_to_string() is not thread-safe</i>. It retuns a pointer to a static
 * buffer that is reused every time exp_value_to_string() is called.
 * If you need a thread-safe version of this routine, use
 * exp_value_to_string_r().
 *
 * @see exp_value_to_string_r().
 *
 * @param ev Pointer to a structure, containing result, that was returned by
 *    exp_solve().
 * @return Pointer to a static buffer with the result converted into its string
 *    representation. If error occurs, such as memory error, exp_value_to_string()
 *    returns NULL and sets errno.
 */
char *exp_value_to_string( exp_value_t *ev);

/**
 * @brief Convert evaluation result to its string representation. Thread-safe
 * version of exp_value_to_string().
 *
 * The routine converts passed value to a string. exp_value_to_string_r() checks the type
 * of the value when converts it.
 *
 * This is a thread-safe version of exp_value_to_string().
 *
 * @param ev Pointer to a structure, containing result, that was returned by
 *    exp_solve().
 * @param buffer Pointer to a buffer where the routine can store the result.
 * @param buffer_max_len Length of the buffer @c buffer.
 * @return Pointer to @c buffer with the result converted into its string
 *    representation. If error occurs, such as memory error, exp_value_to_string_r()
 *    returns NULL and sets errno.
 */
char *exp_value_to_string_r( exp_value_t *ev, char *buffer, int buffer_max_len);

/**
 * @brief Free previously allocated expression structure.
 *
 * @param exp Pointer to a expression structure, returned by exp_create().
 * @return Always returns NULL.
 */
expression_t *exp_free( expression_t *exp);

/**
 * @brief Free previously allocated value structure.
 *
 * @param ev Pointer to a value structure, returned by exp_solve().
 * @return Always returns NULL.
 */
exp_value_t *exp_value_free( exp_value_t *ev);

/**
 * @brief Define a callback that exp_solve() will call to evaluate unknown
 * functions in expression.
 *
 * The exp_set_function_handler() sets a callback that is called when
 * exp_solve() finds in expression a function with unknown name. User may
 * define its own functions. In this way when the callback is called, they will
 * be evaluated in user code.
 *
 * To fully understand the way user functions are solved, see the example below.
 * In this example two user functions are defined: @c third and @c fourth.
 * The callback is called for both of them. The functions are evaluated in the
 * callback.
 *
@code
#include <stdio.h>
#include <stdlib.h>
#include "libexpression.h"

//In this callback we define two functions named `third' and `fourth'.
static int function_callback( void *user_data, char *function_name, int argc, exp_value_t argv[], exp_value_t *result){
	int arg;

	//First, check the name of the processed function
	if( 0==strcasecmp( function_name, "third")){
		//Check number of parameters passed to the processed function.
		if( argc !=1){
			//If the number of parameters is incorrect, return 2.
			return 2;
		}else if( argv[0].type!=EXP_INTEGER){
			//If the type of passed parameter(s) is incorrect, return 2.
			return 2;
		}else{
			//Otherwise, if the number of passed parameters is correct and
			//their types are correct, fill the structure `result'
			arg=argv[0].value.integer;
			result->type=EXP_INTEGER;
			result->value.integer=arg*arg*arg;
			return 0;
		}

	}else if( 0==strcasecmp( function_name, "fourth")){
		if( argc !=1){
			return 2;
		}else if( argv[0].type!=EXP_INTEGER){
			return 2;
		}else{
			arg=argv[0].value.integer;
			result->type=EXP_INTEGER;
			result->value.integer=arg*arg*arg*arg;
			return 0;
		}

	}else{
		//If no function is found/defined with specified name, return 1.
		return 1;
	}
}

int main( int argc, char *argv[]){
	expression_t *exp;     //A structure that stores libexpression private data.
	char error[EXP_ERLEN]; //A buffer where solving routine may store error messages. This buffer must be large enough.
	int error_pos;         //Solving routine will store error position (if error occurs) in this variable.
	exp_error_t ercode;    //Solving routine will store error code (if error occurs) in this variable.
	exp_value_t *result;   //Pointer to a structure containing result.

	//The expression below contains two user defined
	//functions: `third' and `fourth'. These two functions are intended to
	//raise argument to the power of 3 and four respectively.
	//Theese functions are not defined in `libexpression' library, but we
	//define them in our callback.
	if(NULL==( exp=exp_create( "third(4) + fourth(3)", &ercode, error, &error_pos))){
		fprintf( stderr, "Error at %d: %s\n", error_pos, error);
		return 1;
	}
	//Set a callback that will be called by exp_solve() to resolve and
	//evaluate unknown functions
	exp_set_function_handler( exp, function_callback);
	//Call exp_solve() to evaluate the expression
	if(NULL ==(result=exp_solve( exp, &ercode, error, &error_pos))){
		fprintf( stderr, "Error at %d: %s\n", error_pos+1, error);
		exp_free( exp);
		return 1;
	}
	//Now print the result.
	printf( "Result=%s\n", exp_value_to_string( result));
	//Cleanup
	exp_free( exp);
	exp_value_free( result);
	return 0;
}
@endcode
 *
 * @param exp Pointer to a libexpression structure returned by exp_create().
 * @param f Pointer to a user-supplied routine that exp_solve() calls to
 * evaluate unknown functions in the expression. The arguments to the callback
 * are:
 *    @li @c user_data - pointer to a user-defined data. You can set pointer to
 *        any structure before calling exp_solve() by using exp_set_user_data().
 *    @li @c function_name - name of the function being processed
 *        by exp_solve().
 *    @li @c argc - number of arguments passed to the function being processed.
 *    @li @c argv - array of @c exp_value_t structures with arguments.
 *    @li @c result - pointer to a @c exp_value_t structure that must be filled
 *        with result. If the type of the result is String, the buffer that
 *        will contain the result must be allocated. It will be freed
 *        automatically.
 *
 * @par
 * Callback should return 0 if a function can be processed and number of passed
 * parameters is correct and all the passed parameters are of correct type.
 * If function could not be processed, callback should return 1. In this case
 * exp_solve() that called this callback will generate error, e.g. Unknown
 * Function Name.
 *
 * @par
 * If number of supplied parameters is incorrect or the type of one of the
 * parameters is incorrect, the callback should return 2. In this case
 * exp_solve() will generate error, e.g. Invalid Function Call.
 */
#define exp_set_function_handler( exp, f) {(exp)->fhandler=(f);}

/**
 * @brief Define a callback that exp_solve() will call to evaluate unknown
 * parameters or constants.
 *
 * The exp_set_parameter_handler() sets a callback that is called when
 * exp_solve() finds in expression a parameter with unknown name. User may
 * define its own parameters/constants. In this way when the callback is called,
 * they will be evaluated in user code.
 *
 * To fully understand the way user parameters are evaluated, see the example
 * below. In this example two user parameters are defined.
 * The callback is called for both of them. The parameters are evaluated in the
 * callback.
 *
@code
#include <stdio.h>
#include <stdlib.h>
#include "libexpression.h"

//In this callback we define two parameters: `hello' and `world'.
static int param_callback( void *user_data, char *parameter_name, exp_value_t *result){
	//First, you shoud check the name of the parameter.
	if( 0==strcmp( parameter_name, "hello")){
		//If the parameter is known, fill `result' structure and return 0.
		result->type=EXP_INTEGER;
		result->value.integer=1;
		return 0;
	}else if( 0==strcmp( parameter_name, "world")){
		result->type=EXP_INTEGER;
		result->value.integer=2;
		return 0;
	}
	//If no substitution for the parameter is found, return 1.
	return 1;
}

int main( int argc, char *argv[]){
	expression_t *exp;     //A structure that stores libexpression private data.
	char error[EXP_ERLEN]; //A buffer where solving routine may store error messages. This buffer must be large enough.
	int error_pos;         //Solving routine will store error position (if error occurs) in this variable.
	exp_error_t ercode;    //Solving routine will store error code (if error occurs) in this variable.
	exp_value_t *result;   //Pointer to a structure containing result.

	//The expression below contains two user defined
	//parameters (constants): `hello' and `world'.
	//Theese parameters are not defined in `libexpression' library, but we define
	//them in our callback.
	if(NULL==( exp=exp_create( "Hello + world", &ercode, error, &error_pos))){
		fprintf( stderr, "Error at %d: %s\n", error_pos, error);
		return 1;
	}
	//Set a callback that will be called by exp_solve() to resolve and
	//evaluate unknown parameters.
	exp_set_parameter_handler( exp, param_callback);
	//Call exp_solve() to evaluate the expression
	if(NULL ==(result=exp_solve( exp, &ercode, error, &error_pos))){
		fprintf( stderr, "Error at %d: %s\n", error_pos+1, error);
		exp_free( exp);
		return 1;
	}
	//Now print the result.
	printf( "Result=%s\n", exp_value_to_string( result));
	//Cleanup
	exp_free( exp);
	exp_value_free( result);
	return 0;
}
@endcode
 *
 * @param exp Pointer to a libexpression structure returned by exp_create().
 * @param p Pointer to a user-supplied routine that exp_solve() calls to
 * evaluate unknown parameters in the expression. The arguments to the callback
 * are:
 *    @li @c user_data - pointer to a user-defined data. You can set pointer to
 *        any structure before calling exp_solve() by using exp_set_user_data().
 *    @li @c parameter_name - name of the parameter being processed
 *        by exp_solve().
 *    @li @c result - pointer to a @c exp_value_t structure that must be filled
 *        with the result. If the type of the result is String, the buffer that
 *        will contain the result must be allocated. It will be freed
 *        automatically.
 *
 * @par
 * Callback should return 0 if a parameter can be substituted.
 * If the parameter could not be processed, callback should return 1. In this
 * case exp_solve() that called this callback will generate error, e.g. Unknown
 * Parameter Name.
 *
 */
#define exp_set_parameter_handler( exp, p) {(exp)->phandler=(p);}

/**
 * @brief Set pointer to user supplied data that can be used in callbacks.
 *
 * Use exp_set_user_data() to store pointer to some user-supplied data in
 * libexpression structure. This data will be available in callbacks. The data
 * then can be received with exp_get_user_data().
 *
 * @param exp Pointer to libexpression structure returned by exp_create().
 *    The data will be associated with this structure.
 * @param data Pointer to a user supplied data that should be associated with
 *    @c exp.
 */
#define exp_set_user_data( exp, data) {(exp)->user_data=(data);}

/**
 * @brief Get user data previously associated with libexpression structure by
 * exp_set_user_data().
 *
 * Use exp_get_user_data() in callbacks to receive previously set user data.
 * To set user data, use exp_set_user_data() before calling exp_solve().
 *
 * @param exp Pointer to libexpression structure which should contain user data.
 * @return Pointer to user data.
 */
#define exp_get_user_data( exp) ((exp)->user_data)

/** @} */

#endif /* LIBEXPRESSION_H_ */
