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
 * Reverse polish notation implementation.
 */

#include "libexpression-private.h"


#define operator_to_string( op) ({\
	char *ret;\
	switch(op){\
		case O_BOOLNOT:    ret="Logical NOT"; break;\
		case O_BITNOT:     ret="Bitwise NOT"; break;\
		case O_DIV:        ret="Division"; break;\
		case O_MOD:        ret="Modulo"; break;\
		case O_MUL:        ret="Multiplication"; break;\
		case O_UPLUS:      ret="Unary plus"; break;\
		case O_PLUS:       ret="Plus"; break;\
		case O_UMINUS:     ret="Unary minus"; break;\
		case O_MINUS:      ret="Minus"; break;\
		case O_SHIFTLEFT:  ret="Bitwise left shift"; break;\
		case O_SHIFTRIGHT: ret="Bitwise right shift"; break;\
		case O_GT:         ret="Greater than"; break;\
		case O_LT:         ret="Less than"; break;\
		case O_GE:         ret="Greater than or equal to"; break;\
		case O_LE:         ret="Less than or equal to"; break;\
		case O_NOTEQUALS:  ret="Not equal"; break;\
		case O_BOOLEQUALS: ret="Equal"; break;\
		case O_BITAND:     ret="Bitwise AND"; break;\
		case O_HAT:        ret="Power"; break;\
		case O_BITOR:      ret="Bitwise OR"; break;\
		case O_BOOLAND:    ret="Logical AND"; break;\
		case O_BOOLOR:     ret="Logical OR"; break;\
		case O_EQUALS:     ret="Equal"; break;\
		default:           ret="Unknown";\
	}\
	ret;\
})


static int exp_eval_if( expression_t *exp, token_t **stack, int *stack_len, exp_error_t *ercode, char *error, int *error_pos){
	token_t *s=*stack;
	token_t *temp, *opqueue, *result;
	int c=3;
	int b1;
	int ret;
	int64_t r;

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
		*ercode=EXP_ER_NOMEM;
		strcpy(error, "Memory error");
		*error_pos=0;
		return 1;
	}

	if( 0 !=(ret=exp_to_boolean( &opqueue->param, &b1))){
		exp_token_free(opqueue);
		exp_token_free(result);
		*stack=s;
		*error_pos=opqueue->position;
		*ercode=ret;
		switch( ret){
			case EXP_ER_NONBOOLEAN:  sprintf(error, "Conditional statement requires boolean operand"); break;
			case EXP_ER_INVALARGV:   strcpy(error, "Invalid operand provided to evaluate conditional statement"); break;
			default: strcpy(error, "Error occured"); break;
		}
		return 1;

	}else{
		if( b1){
			if( 0 !=exp_rpn( exp, opqueue->next->children, &result->param, ercode, error, error_pos)){
				exp_token_free(opqueue);
				exp_token_free(result);
				*stack=s;
				return 1;
			}
		}else{
			if( 0 !=exp_rpn( exp, opqueue->next->next->children, &result->param, ercode, error, error_pos)){
				exp_token_free(opqueue);
				exp_token_free(result);
				*stack=s;
				return 1;
			}
		}


		if( result->param.type==T_REAL && 0==exp_is_integer( &result->param, &r)){
			result->param.type=T_INTEGER;
			result->param.value.integer=r;
		}
		result->position=opqueue->position;
		exp_token_free(opqueue);
		result->next=s;
		*stack=result;
		(*stack_len)+=1;
		return 0;
	}
}

int exp_rpn( expression_t *exp, token_t *input, value_t *ret, exp_error_t *ercode, char *error, int *error_pos){
	token_t *in, *curr, *stack;
	int stack_len;
	int status;

	in=exp_token_dup( input);
	stack=NULL;
	stack_len=0;

	//While there are input tokens left
	while(in){
		//Read the next token from input.
		curr=in;
		in=in->next;
		curr->next=NULL;

		switch( curr->param.type){
			//case T_PARAMETER:
			//All parameters were substituded on the previous step

			case T_BOOLEAN:
			case T_INTEGER:
			case T_REAL:
			case T_STRING:
			case T_IFSTATEMENT:
				//If the token is a value
				//	Push it onto the stack

				curr->next=stack;
				stack=curr;
				stack_len++;
				break;

			case T_OPERATOR:
				//Otherwise, the token is an operator (operator here includes both operators, and functions).
				//	It is known a priori that the operator takes n arguments.
				//	If there are fewer than n values on the stack
				//		The user has not input sufficient values in the expression
				//	Else, Pop the top n values from the stack.
				//		Evaluate the operator, with the values as arguments.
				//		Push the returned results, if any, back onto the stack

				if(stack_len>=exp_op_argument_count( curr->param.value.operator)){
					status=exp_eval_operator( &stack, curr->param.value.operator, &stack_len);
					if( 0 !=status){
						*ercode=status;
						switch( status){
							case EXP_ER_INVALARGC:     sprintf(error, "%s operator does not have sufficient number of operands", operator_to_string(curr->param.value.operator)); break;
							case EXP_ER_INVALARGV:     strcpy(error, "Invalid operand provided to evaluate expression with operator"); break;
							case EXP_ER_INVALOPERATOR: strcpy(error, "Invalid operator"); break;
							case EXP_ER_NOMEM:         strcpy(error, "Memory error occured while expression was evaluated"); break;
							case EXP_ER_COMPLEX:       strcpy(error, "Complex result when evaluating expression"); break;
							case EXP_ER_DIVBYZERO:     strcpy(error, "Division by zero"); break;
							case EXP_ER_NONINTEGER:    sprintf(error, "%s operator requires integer operands", operator_to_string(curr->param.value.operator)); break;
							case EXP_ER_NONNUMERIC:    sprintf(error, "%s operator requires numeric or boolean operands", operator_to_string(curr->param.value.operator)); break;
							case EXP_ER_NONBOOLEAN:    sprintf(error, "%s operator requires boolean operands", operator_to_string(curr->param.value.operator)); break;
							case EXP_ER_NONSTRING:     sprintf(error, "%s operator requires string operands", operator_to_string(curr->param.value.operator)); break;
							case EXP_ER_INTOVERFLOW:   strcpy(error, "Overflow occurs when converting operator to integer"); break;


							default: strcpy(error, "Error occured"); break;
						}
						*error_pos=curr->position;
						exp_token_free(curr);
						exp_token_free(stack);
						exp_token_free(in);
						return -1;

					}else{
						//we should free operator token
						exp_token_free( curr);
					}

				}else{
					*ercode=EXP_ER_INVALEXPR;
					strcpy(error, "Operator does not have sufficient number of operands");
					*error_pos=curr->position;
					exp_token_free(curr);
					exp_token_free(stack);
					exp_token_free(in);
					return -1;
				}
				break;

			case T_IFCONDITION:
				if(stack_len>=3 && stack->param.type==T_IFSTATEMENT && stack->next->param.type==T_IFSTATEMENT){
					status=exp_eval_if( exp, &stack, &stack_len, ercode, error, error_pos);
					if( 0 !=status){
						exp_token_free(curr);
						exp_token_free(stack);
						exp_token_free(in);
						return -1;

					}else{
						//we should free operator token
						exp_token_free( curr);
					}

				}else{
					*ercode=EXP_ER_INVALEXPR;
					strcpy(error, "Conditional expression does not have sufficient number of operands");
					*error_pos=curr->position;
					exp_token_free(curr);
					exp_token_free(stack);
					exp_token_free(in);
					return -1;
				}
				break;

			case T_FUNCTION:
				if(stack_len>=1 && stack->param.type==T_INTEGER){
					token_t *temp;
					int argc, i;

					//pop argc
					argc=stack->param.value.integer;
					temp=stack;
					stack=stack->next;
					stack_len--;
					temp->next=NULL;
					exp_token_free( temp);

					//test for tokens in stack
					if( argc>stack_len){
						*ercode=EXP_ER_INVALEXPR;
						sprintf(error, "Algorithm error: stack length is less than arguments count");
						*error_pos=curr->position;
						exp_token_free(curr);
						exp_token_free(stack);
						exp_token_free(in);
						return -1;
					}

					temp=stack;
					i=argc;
					while( temp && i){
						if( !(temp->param.type==T_BOOLEAN ||temp->param.type==T_INTEGER ||
								temp->param.type==T_REAL ||temp->param.type==T_STRING)){
							*ercode=EXP_ER_INVALEXPR;
							sprintf(error, "Algorithm error: unsupported function argument in stack");
							*error_pos=curr->position;
							exp_token_free(curr);
							exp_token_free(stack);
							exp_token_free(in);
							return -1;
						}
						i--;
						temp=temp->next;
					}

					status=exp_call_function( exp, curr->param.value.function, argc, &stack, &stack_len);
					if( 0 !=status){
						*ercode=status;
						switch( status){
							case EXP_ER_INVALARGV:     strcpy(error, "Invalid function argument" ); break;
							case EXP_ER_INVALARGCHIGH: strcpy(error, "Too many arguments passed to function" ); break;
							case EXP_ER_INVALARGCLOW:  strcpy(error, "Too few arguments passed to function" ); break;
							case EXP_ER_INVALFUNC:     strcpy(error, "Unknown function" ); break;
							case EXP_ER_TRIGONOMETRIC: strcpy(error, "Function argument is not in range" ); break;
							case EXP_ER_COMPLEX:       strcpy(error, "Complex result when evaluating function"); break;
							case EXP_ER_INTOVERFLOW:   strcpy(error, "Overflow occurs when converting argument to integer"); break;
							case EXP_ER_NONINTEGER:    strcpy(error, "Function requires integer operands"); break;
							case EXP_ER_NONNUMERIC:    strcpy(error, "Function requires numeric or boolean operands"); break;
							case EXP_ER_NONBOOLEAN:    strcpy(error, "Function requires boolean operands"); break;
							case EXP_ER_NONSTRING:     strcpy(error, "Function requires string operands"); break;
							case EXP_ER_NOMEM:         strcpy(error, "Memory error occured while expression was evaluated"); break;
							case EXP_ER_INVALRET:      strcpy(error, "Unknown type was returned by user defined function handler"); break;
							case EXP_ER_USERFUNCERROR: strcpy(error, "Error in user defined function handler"); break;
							case EXP_ER_DIVBYZERO:     strcpy(error, "Division by zero"); break;
							default: sprintf(error, "Error occured (%d)", status); break;
						}
						*error_pos=curr->position;
						exp_token_free(curr);
						exp_token_free(stack);
						exp_token_free(in);
						return -1;
					}else{
						exp_token_free( curr);
					}

				}else{
					*ercode=EXP_ER_INVALEXPR;
					sprintf(error, "Algorithm error: no argument count found for function");
					*error_pos=curr->position;
					exp_token_free(curr);
					exp_token_free(stack);
					exp_token_free(in);
					return -1;
				}
				break;

			default:
				*ercode=EXP_ER_INVALEXPR;
				strcpy(error, "Invalid or unsupported token");
				*error_pos=curr->position;
				exp_token_free(curr);
				exp_token_free(stack);
				exp_token_free(in);
				return -1;
				break;
		}
	}
	if(stack_len==0){
		*ercode=EXP_ER_INVALEXPR;
		strcpy(error, "Expression is possibly malformed, it has too many operators");
		*error_pos=0;
		return -1;

	}else if(stack_len>1){
		*ercode=EXP_ER_INVALEXPR;
		strcpy(error, "Expression is possibly malformed, it has too many operands");
		*error_pos=0;
		exp_token_free( stack);
		return -1;

	}else if( stack->param.type==T_BOOLEAN ||stack->param.type==T_INTEGER ||
			stack->param.type==T_REAL ||stack->param.type==T_STRING){
		if(stack->param.type==T_STRING){
			ret->type=stack->param.type;
			if( stack->param.value.string){
				ret->value.string=strdup(stack->param.value.string);
			}else{
				ret->value.string=strdup("NULL");
			}
		}else{
			memcpy( ret, &stack->param, sizeof( value_t));
		}
		exp_token_free( stack);
		return 0;

	}else{
		*ercode=EXP_ER_INVALEXPR;
		strcpy(error, "Expression is possibly malformed");
		*error_pos=0;
		exp_token_free( stack);
		return -1;
	}
}


