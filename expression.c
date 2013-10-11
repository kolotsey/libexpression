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
 * This example file is a complete program that takes an math or logical
 * expression as input, solves or simplifies it, and returns its result.
 *
 * To compile this example, install libexpression library and then run in
 * console:
 *    gcc -lexpression -o expression expression.c
 *
 * This will compile and build this example file into a program `expression'
 * in the same directory. To execute and test this program, run in console,
 * for example:
 *    ./expression "2+2"
 *
 * You can use any valid math or logic expression:
 *    ./expression "0xff+5*((-2)^7-3/2) > cos(90*PI/180)? True : False"
 */

/**
 * @file expression.c
 * Libexpression usage example
 * @ingroup libexpression
 * @{
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>


#include "libexpression.h"


/*
 * This callback shows how to write a callback for undefined
 * constants/parameters. There are three variables passed to this callback
 * from the library:
 *    user_data      - pointer to a user-defined data. You can set pointer to
 *                     any structure before calling solving routines by
 *                     using exp_set_user_data().
 *    parameter_name - name of the undefined parameter being processed.
 *    result         - pointer to a `exp_value_t' structure that must be filled
 *                     with result.
 * Callback should return 0 if a substitution for the parameter was found.
 * If no substitution was found, callback should return 1. In this case solving
 * routine that called this callback will generate error, e.g. Unknown
 * parameter name.
 */
static int phandler( void *user_data, char *parameter_name, exp_value_t *result){
	//First, you shoud check the name of the parameter.
	if( 0==strcmp( parameter_name, "time")){
		//If the parameter is known, fill `result' structure and return 0.
		result->type=EXP_INTEGER;
		result->value.integer=time( NULL);
		return 0;
	}else if( 0==strcmp( parameter_name, "hello")){
		//If substitution for the parameter is a string, you should duplicate
		//it yourself. This duplicated string will be automatically cleaned up
		//when not needed.
		result->type=EXP_STRING;
		result->value.string=strdup("Hello world!");
		return 0;
	}
	//If no substitution for the parameter is found, return 1.
	return 1;
}


/*
 * The fhandler() routine shows how to write a callback for undefined
 * functions. There are five variables passed to this callback
 * from the library:
 *    user_data      - pointer to a user-defined data. You can set pointer to
 *                     any structure before calling solving routines by
 *                     using exp_set_user_data().
 *    function_name  - name of the function being processed.
 *    argc           - number of arguments passed to the function being
 *                     processed.
 *    argv           - array of `exp_value_t' structures with arguments.
 *    result         - pointer to a `exp_value_t' structure that must be filled
 *                     with result.
 * Callback should return 0 if a function can be processed and number of passed
 * parameters is correct and all the passed parameters are of correct type.
 * If function could not be processed, callback should return 1. In this case
 * solving routine that called this callback will generate EXP_ER_INVALFUNC
 * (Unknown function name) error.
 * If number of parameters is incorrect or the type of one of the parameters is
 * incorrect, the callback should return 2. In this case solving routine that
 * called this callback will generate EXP_ER_USERFUNCERROR error.
 */
static int fhandler( void *user_data, char *function_name, int argc, exp_value_t argv[], exp_value_t *result){
	int i;
	//First, check the name of the function
	if( 0==strcasecmp( function_name, "sleep")){
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
			result->type=EXP_BOOLEAN;
			result->value.boolean=1;
			sleep(argv[0].value.integer);
			return 0;
		}

	}else if( 0==strcasecmp( function_name, "uppercase") || 0==strcasecmp( function_name, "upper")){
		if( argc !=1){
			return 2;

		}else if( argv[0].type!=EXP_STRING){
			return 2;

		}else{
			//If the result of the processed function is a string, you should allocate
			//it yourself. This allocated string will be automatically
			//cleaned up when not needed.
			result->type=EXP_STRING;
			result->value.string=strdup( argv[0].value.string);
			for( i=0; i<strlen( result->value.string); i++){
				result->value.string[i]=toupper(result->value.string[i]);
			}
			return 0;
		}

	}else if( 0==strcasecmp( function_name, "lowercase") || 0==strcasecmp( function_name, "lower")){
		if( argc !=1){
			return 2;

		}else if( argv[0].type!=EXP_STRING){
			return 2;

		}else{
			result->type=EXP_STRING;
			result->value.string=strdup( argv[0].value.string);
			for( i=0; i<strlen( result->value.string); i++){
				result->value.string[i]=tolower(result->value.string[i]);
			}
			return 0;
		}

	}else{
		//If no function is found/defined with specified name, return 1.
		return 1;
	}
}


/*
 * Usage message is shown when the program is executed without arguments.
 */
static char *program_name="expression";
static void usage(){
	fprintf( stderr,
"Usage: %s <EXPRESSION>\n"
"\n"
"Simple calculator based on libexpression library.\n"
"\n"
"EXPRESSION is any well-formed math or logic expression to solve.\n"
"Wrap EXPRESSION with qoutes or double quotes.\n"
"Try the following:\n"
"    %s \"2+2*2\"\n"
"    %s \"strtoupper('Hello, world!')\"\n"
"    %s \"0xff+5*((-2)^7-3/2) > cos(90*PI/180)? True : False\"\n",
	program_name, program_name, program_name, program_name);
}


/*
 * This main routine creates an `expression_t' structure, defines callbacks,
 * and calls solving routine exp_solve(). After the expression is solved,
 * this routine prints the result and cleans up all previously allocated data.
 */
int main( int argc, char *argv[]){
	expression_t *exp;     //A structure that stores libexpression private data.
	char error[EXP_ERLEN]; //A buffer where solving routine may store error messages. This buffer must be large enough.
	int error_pos;         //Solving routine will store error position (if error occurs) in this variable.
	exp_error_t ercode;    //Solving routine will store error code (if error occurs) in this variable.
	exp_value_t *v;        //Pointer to a structure containing result.
	char *result;          //Pointer to a buffer with the result converted to a string
	int ret=0;



	//Copy name of the program to a global variable
	//This name is used in usage()
	if( argc){
		program_name=argv[0];
	}

	//Check if expression was really provided by user as a program argument.
	//If not, then display usage message.
	if(argc<2){
		usage();
		return 1;
	}

	//This call to srand() is required, because in the library the call to
	//this function is not made, however random functions are used in order
	//to provide support for random numbers in expressions.
	srand(time(NULL));

	//Create expression object from user input
	if(NULL==( exp=exp_create( argv[ 1], &ercode, error, &error_pos))){
		ret=1;

	}else{
		//Add some handlers that resolve our special functions
		//and constants
		exp_set_parameter_handler( exp, phandler);
		exp_set_function_handler( exp, fhandler);

		//Pass expression object to a solving routive ang get result
		if(NULL ==(v=exp_solve( exp, &ercode, error, &error_pos))){
			ret=1;

		}else{
			//Solving routine returned result. That means, no error occured.
			//Display the result to the user.
			result=exp_value_to_string( v);

			if( result){
				printf("%s\n", result);

			}else{
				printf( error, "Result is invalid (e.g. result type is invalid or memory error)\n");
				error_pos=-1;
				ret=1;
			}
			//Free result structure
			exp_value_free( v);
		}
		//Free expression private data
		exp_free( exp);
	}

	//If any error occured while expression object was created
	//or expression was solved, then `ret' variable is set to 1
	//in this module. We check this variable and if it is set,
	//then we display error message to the user including
	//column number where the error occured.
	if( ret){
		if(error_pos ==-1){
			fprintf( stdout, "%s\n", error);

		}else{
			char col[1024];
			int i;
			for(i=0; i< error_pos; i++) col[i]=' ';
			col[i++]='^';
			col[i++]=0;
			fprintf( stderr, "%s\n%s\n", argv[1], col);
			fprintf( stdout, "Char %d: %s\n", error_pos+1, error);
		}
	}
	return ret;
}

/** @} */
