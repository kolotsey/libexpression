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
 * Shunting-yard implementation.
 */

#include "libexpression-private.h"


/*
 * Returns precedence of the operator
 */
static int op_precedence( operator_t op){
    switch(op)    {
    	case O_UMINUS:
    	case O_UPLUS:      return 20; //unary minus, unary plus
    	case O_HAT:        return 19; // ^
    	case O_BOOLNOT:
		case O_BITNOT:     return 18; // ! ~
		case O_DIV:
		case O_MOD:
		case O_MUL:        return 17; // % * /
		case O_PLUS:
		case O_MINUS:      return 16; // + -
		case O_SHIFTLEFT:
		case O_SHIFTRIGHT: return 15; // >> <<
		case O_GT:
		case O_LT:
		case O_GE:
		case O_LE:         return 14; // < > <= >=
		case O_NOTEQUALS:
		case O_EQUALS:
		case O_BOOLEQUALS: return 13; // != == =
		case O_BITAND:     return 12; // &
		case O_BITOR:      return 11; // |
		case O_BOOLAND:    return 10; // &&
		case O_BOOLOR:     return  9; // ||
		case O_IFTHEN:     return  8; // ?
//		case O_EQUALS:     return  8; // =
		default:
			fprintf(stderr, "Invalid operator %d (%c)\n", op, op);
			return 0;
    }
}


#define STACK_PUSH( stack, stacklen, maxlen, val) ({\
	if(stacklen>=maxlen){\
		if((stack=realloc( stack, sizeof( int*)*(maxlen+128)))){\
			maxlen+=128;\
		}else{\
			maxlen=0;\
			stacklen=0;\
		}\
	}\
	if(stack) stack[stacklen++]=val;\
	(stack);\
})

#define STACK_POP( stack, stacklen) ( stack[stacklen-- - 1])

#define ERROR_OCCURED( _estring, _epos) {\
	strcpy(error, _estring);\
	*error_pos=(_epos);\
	exp_token_free( stack);\
	exp_token_free( out_head);\
	exp_token_free( in);\
	exp_token_free( curr);\
	if( argc_stack) free( argc_stack);\
	if( found_stack) free( found_stack);\
}

token_t *exp_shunting_yard( token_t *input, int if_operand, token_t **new_input, exp_error_t *ercode, char *error, int *error_pos){
	token_t *in, *in1, *curr=NULL;
	token_t *stack=NULL;
	token_t *out_head=NULL, *out_tail=NULL;
	token_t *temp;
	//MOD:Stacks are used to enable support of variable-argument count functions
	int *found_stack=NULL;
	int found_maxlen=0;
	int found_len=0;
	int *argc_stack=NULL;
	int argc_maxlen=0;
	int argc_len=0;
	int colon_found=0;


	//dup input queue
	in=exp_token_dup( input);

	//Shunting-yard algorthm:
	//While there are tokens to be read
	while(in && 0==colon_found){
		//Read a token
		curr=in;
		in=in->next;
		curr->next=NULL;

		switch( curr->param.type){
			case T_BOOLEAN:
			case T_INTEGER:
			case T_REAL:
			case T_STRING:
			case T_PARAMETER:
				//If the token is a number, then add it to the output queue.
				//MOD: If the were values stack has a value on it, pop it and push True.
				if(out_tail){
					out_tail->next=curr;
					out_tail=curr;
				}else{
					out_tail=out_head=curr;
				}
				if(found_len){
					found_stack[found_len-1]=1;
				}
				break;

			case T_FUNCTION:
				//If the token is a function token, then push it onto the stack.
				//MOD: Push 0 onto the arg count stack.
				//MOD: If the were values stack has a value on it, pop it and push True.
				//MOD: Push False onto were values.
				curr->next=stack;
				stack=curr;
				STACK_PUSH( argc_stack, argc_len, argc_maxlen, 0);
				if( found_len){
					found_stack[found_len-1]=1;
				}
				if(NULL==STACK_PUSH( found_stack, found_len, found_maxlen, 0)){
					*ercode=EXP_ER_NOMEM;
					ERROR_OCCURED( "Memory error", 0);
					return NULL;
				}
				break;

			case T_COMMA:
				//If the token is a function argument separator (e.g., a comma):
				//Until the token at the top of the stack is a left parenthesis,
				//Pop operators off the stack onto the output queue.
				//If no left parentheses are encountered, either the separator
				//was misplaced or parentheses were mismatched
				//MOD: Pop were values into w.
				//MOD: If w is True, pop arg count into a, increment a and push back into arg count.
				//MOD: Push False into were values
				temp=stack;
				while(temp && (temp->param.type !=T_LPAREN)){
					stack=stack->next;
					if(out_tail){
						out_tail->next=temp;
						out_tail=temp;
					}else{
						out_tail=out_head=temp;
					}
					temp->next=NULL;
					temp=stack;
				}
				if(stack==NULL || stack->param.type !=T_LPAREN){
					*ercode=EXP_ER_INVALEXPR;
					ERROR_OCCURED("Unexpected comma", curr->position);
					return NULL;
				}

				if(found_len && STACK_POP( found_stack, found_len) && argc_len){
					argc_stack[argc_len-1]=argc_stack[argc_len-1]+1;
					if(NULL==STACK_PUSH( found_stack, found_len, found_maxlen, 0)){
						*ercode=EXP_ER_NOMEM;
						ERROR_OCCURED( "Memory error", 0);
						return NULL;
					}
				}else{
					//Unexpected comma
					*ercode=EXP_ER_INVALEXPR;
					ERROR_OCCURED("Unexpected comma", curr->position);
					return NULL;
				}

				//free comma-token
				exp_token_free( curr);
				curr=NULL;
				break;

			case T_OPERATOR:
				//If the token is an operator, o1, then:
				//	while there is an operator token, o2, at the top of the stack,
				//	and either o1 is left-associative and its precedence is less
				//	than or equal to that of o2, or o1 is right-associative and
				//	its precedence is less than that of o2,
				//		pop o2 off the stack, onto the output queue;
				//	push o1 onto the stack
				if( curr->param.value.operator==O_ELSE){
					if( 0==if_operand){
						*ercode=EXP_ER_INVALEXPR;
						ERROR_OCCURED( "Unexpected colon", curr->position);
						return NULL;

					}else if( 1==if_operand){
						exp_token_free( curr);
						colon_found=1;
						break;

					}else{//2==if_operand
						//don't remove colon, because it is required
						curr->next=in;
						in=curr;
						curr=NULL;
						colon_found=1;
						break;
					}
				}

				temp=stack;
				while( temp && (temp->param.type==T_OPERATOR)){
					if( (exp_op_is_lefttoright( curr->param.value.operator) && (op_precedence( curr->param.value.operator) <= op_precedence( temp->param.value.operator)))
							|| ( (!exp_op_is_lefttoright( curr->param.value.operator)) && (op_precedence( curr->param.value.operator) < op_precedence( temp->param.value.operator)))){
						stack=stack->next;
						if(out_tail){
							out_tail->next=temp;
							out_tail=temp;
						}else{
							out_tail=out_head=temp;
						}
						temp->next=NULL;
						temp=stack;
					}else{
						break;
					}
				}

				// if the operator is O_IF then for all two its operands call S-Y algorith separately
				// and push them onto output queue
				if( curr->param.value.operator==O_IFTHEN){
					char estr[EXP_ERLEN];
					int epos;
					token_t *t;

					in1=NULL;
					if(NULL==(t=exp_shunting_yard( in, 1, &in1, ercode, estr, &epos)) || in1==NULL){
						if( in1==NULL){
							*ercode=EXP_ER_INVALEXPR;
							ERROR_OCCURED( "Colon was not found in conditional expression", curr->position);
						}else{
							ERROR_OCCURED( estr, epos);
						}
						exp_token_free( in1);
						exp_token_free( t);
						return NULL;
					}else{
						token_t *iftrue;

						exp_token_free( in);
						in=in1;
						in1=NULL;
						if( NULL==( iftrue=calloc( 1, sizeof( token_t)))){
							*ercode=EXP_ER_NOMEM;
							ERROR_OCCURED( "Memory error", 0);
							exp_token_free( t);
							return NULL;

						}else{
							iftrue->position=curr->position;
							iftrue->param.type=T_IFSTATEMENT;
							iftrue->children=t;

							in1=NULL;
							if(NULL==(t=exp_shunting_yard( in, 2, &in1, ercode, estr, &epos))){
								ERROR_OCCURED( estr, epos);
								exp_token_free( iftrue);
								exp_token_free( in1);
								return NULL;
							}else{
								token_t *iffalse;

								exp_token_free( in);
								in=in1;
								in1=NULL;
								if( NULL==( iffalse=calloc( 1, sizeof( token_t)))){
									*ercode=EXP_ER_NOMEM;
									ERROR_OCCURED( "Memory error", 0);
									exp_token_free( iftrue);
									exp_token_free( t);
									return NULL;

								}else{
									iffalse->position=curr->position;
									iffalse->param.type=T_IFSTATEMENT;
									iffalse->children=t;


									if(out_tail){
										out_tail->next=iftrue;
										out_tail=iftrue;
									}else{
										out_tail=out_head=iftrue;
									}
									iftrue->next=NULL;

									out_tail->next=iffalse;
									out_tail=iffalse;
									iffalse->next=NULL;

									curr->param.type=T_IFCONDITION;
									out_tail->next=curr;
									out_tail=curr;
									curr->next=NULL;
								}
							}
						}
					}

				}else{
					curr->next=stack;
					stack=curr;
				}
				break;

			case T_LPAREN:
				//If the token is a left parenthesis, then push it onto the stack
				curr->next=stack;
				stack=curr;
				break;

			case T_RPAREN:
				//If the token is a right parenthesis:
				//	Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue.
				//	If the stack runs out without finding a left parenthesis, then there are mismatched parentheses
				temp=stack;
				while(temp && (temp->param.type !=T_LPAREN)){
					stack=stack->next;
					if(out_tail){
						out_tail->next=temp;
						out_tail=temp;
					}else{
						out_tail=out_head=temp;
					}
					temp->next=NULL;
					temp=stack;
				}
				if(stack==NULL || stack->param.type !=T_LPAREN){
					if( stack==NULL && if_operand==2){
						curr->next=in;
						in=curr;
						*new_input=in;
						if( argc_stack) free( argc_stack);
						if( found_stack) free( found_stack);
						if(NULL==out_head){
							*ercode=EXP_ER_INVALEXPR;
							strcpy(error, "Empty clause");
							*error_pos=curr->position;
						}
						return out_head;
					}else{
						*ercode=EXP_ER_INVALEXPR;
						ERROR_OCCURED("Unexpected right parenthesis", curr->position);
						return NULL;
					}
				}
				//	Pop the left parenthesis from the stack, but not onto the output queue.
				temp=stack;
				stack=stack->next;
				temp->next=NULL;
				exp_token_free( temp);
				exp_token_free( curr);
				curr=NULL;
				//If the token at the top of the stack is a function token
				// MOD:Pop stack into f
				// MOD:Pop arg count into a
				// MOD:Pop were values into w
				// MOD:If w is True, increment a
				// MOD:Set the argument count of f to a
				// MOD:Push f onto output queue
				temp=stack;
				if(temp && (temp->param.type==T_FUNCTION)){
					token_t *func=temp, *argc;
					stack=stack->next;

					if( argc_len && found_len){
						if(( argc=calloc( 1, sizeof( token_t)))){
							argc->param.type=T_INTEGER;
							argc->param.value.integer=STACK_POP( argc_stack, argc_len)+
									(STACK_POP(found_stack, found_len)? 1 : 0);
							argc->position=func->position;

							if(out_tail){
								out_tail->next=argc;
								out_tail=argc;
							}else{
								out_tail=out_head=argc;
							}
							argc->next=NULL;

						}else{
							*ercode=EXP_ER_NOMEM;
							ERROR_OCCURED( "Memory error", 0);
							exp_token_free( func);
							return NULL;
						}
					}else{
						*ercode=EXP_ER_NOMEM;
						ERROR_OCCURED( "Algorithm error", 0);
						exp_token_free( func);
						return NULL;
					}

					if(out_tail){
						out_tail->next=func;
						out_tail=func;
					}else{
						out_tail=out_head=func;
					}
					func->next=NULL;
				}
				break;

			default:
				*ercode=EXP_ER_INVALEXPR;
				ERROR_OCCURED("Invalid or unsupported token", curr->position);
				return NULL;
		}
	}


	//When there are no more tokens to read:
	//	While there are still operator tokens in the stack:
	//		If the operator token on the top of the stack is a parenthesis, then there are mismatched parentheses.
	//		Pop the operator onto the output queue

	curr=stack;
	while( curr){
		stack=stack->next;
		curr->next=NULL;
		switch(curr->param.type){
			case T_LPAREN:
				if( if_operand==1){
					*ercode=EXP_ER_INVALEXPR;
					ERROR_OCCURED("Non-closed left parenthesis in conditional expression", curr->position);
				}else{
					*ercode=EXP_ER_INVALEXPR;
					ERROR_OCCURED("Left parenthesis is opened but right parenthesis was not found", curr->position);

				}
				return NULL;
			case T_RPAREN:
				*ercode=EXP_ER_INVALEXPR;
				ERROR_OCCURED("Unexpected right parenthesis", curr->position);
				return NULL;
			default:
				if(out_tail){
					out_tail->next=curr;
					out_tail=curr;
				}else{
					out_tail=out_head=curr;
				}
				break;
		}
		curr=stack;
	}

	if( argc_stack) free( argc_stack);
	if( found_stack) free( found_stack);
	if(NULL==out_head){
		*ercode=EXP_ER_INVALEXPR;
		strcpy(error, "Empty expression was provided");
		*error_pos=0;
		return NULL;
	}else{
		if( if_operand){
			if( colon_found){
				*new_input=in;
			}else{
				*new_input=NULL;
			}
		}
		return out_head;
	}
}
