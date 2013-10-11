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
 * Parse input string into tokens.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "libexpression-private.h"

static char *strdupn( char *s, size_t n){
	char *ret=NULL;
	char *p;

	if((ret=malloc( n+1))){
		p=ret;
		while( *s && n){
			*p++=*s++;
			n--;
		}
		*p=0;
	}
	return ret;
}

static void stripslashes( char *s){
	char c;
	while( *s){
		if( *s=='\\'){
			c=*(s+1);
			switch (c){
				case 'n':
					*s='\n';
					memmove( s+1, s+2, strlen(s+2)+1);
					break;
				case 'r':
					*s='\r';
					memmove( s+1, s+2, strlen(s+2)+1);
					break;
				case 't':
					*s='\t';
					memmove( s+1, s+2, strlen(s+2)+1);
					break;
				case '\\':
					memmove( s+1, s+2, strlen(s+2)+1);
					break;
				case '\'':
					*s='\'';
					memmove( s+1, s+2, strlen(s+2)+1);
					break;
				case '"':
					*s='"';
					memmove( s+1, s+2, strlen(s+2)+1);
					break;
				default:
					s+=2;
					break;
			}

		}else{
			s++;
		}
	}
}

int exp_op_argument_count( operator_t op){
	switch(op)    {
		case O_BOOLNOT:
		case O_BITNOT:
		case O_UMINUS:
		case O_UPLUS:
			return 1;
		case O_EQUALS:
		case O_HAT:
		case O_DIV:
		case O_MOD:
		case O_MUL:
		case O_PLUS:
		case O_MINUS:
		case O_SHIFTLEFT:
		case O_SHIFTRIGHT:
		case O_GT:
		case O_LT:
		case O_GE:
		case O_LE:
		case O_NOTEQUALS:
		case O_BOOLEQUALS:
		case O_BITAND:
		case O_BITOR:
		case O_BOOLAND:
		case O_BOOLOR:
		case O_IFTHEN:
		case O_ELSE:
			return 2;
		default:
			fprintf(stderr, "Invalid operator %d (%c)\n", op, op);
			return 0;
    }
}

/*
 * Returns 1 if the operator is  left-to-right associative, otherwise returns 0
 */
int exp_op_is_lefttoright( operator_t op){
	switch(op)    {
		case O_BOOLNOT:
		case O_BITNOT:
		case O_EQUALS:
		case O_HAT:
		case O_UMINUS:
		case O_UPLUS:
		case O_IFTHEN:
		case O_ELSE:
			return 0;
		case O_DIV:
		case O_MOD:
		case O_MUL:
		case O_PLUS:
		case O_MINUS:
		case O_SHIFTLEFT:
		case O_SHIFTRIGHT:
		case O_GT:
		case O_LT:
		case O_GE:
		case O_LE:
		case O_NOTEQUALS:
		case O_BOOLEQUALS:
		case O_BITAND:
		case O_BITOR:
		case O_BOOLAND:
		case O_BOOLOR:
			return 1;
		default:
			fprintf(stderr, "Invalid operator %d (%c)\n", op, op);
			return 1;
    }
}




token_t *exp_token_free( token_t *head){
	token_t *t;
	while(head){
		t=head;
		head=head->next;

		if(t->param.type==T_STRING && t->param.value.string){
			free(t->param.value.string);
		}
		if(t->param.type==T_PARAMETER && t->param.value.parameter){
			free(t->param.value.parameter);
		}
		if(t->param.type==T_FUNCTION && t->param.value.function){
			free(t->param.value.function);
		}
		if(t->param.type==T_IFSTATEMENT){
			exp_token_free( t->children);
		}
		free(t);
	}
	return NULL;
}



token_t *exp_token_dup( token_t *tok){
	token_t *ret=NULL, *ret_tail=NULL, *c;

	while( tok){
		if( NULL==(c=calloc(1, sizeof( token_t)))){
			break;
		}

		if(tok->param.type==T_STRING){
			c->param.type=tok->param.type;
			c->position=tok->position;
			if( tok->param.value.string){
				c->param.value.string=strdup(tok->param.value.string);
			}
		}else if(tok->param.type==T_PARAMETER){
			c->param.type=tok->param.type;
			c->position=tok->position;
			if( tok->param.value.parameter){
				c->param.value.parameter=strdup(tok->param.value.parameter);
			}
		}else if(tok->param.type==T_FUNCTION){
			c->param.type=tok->param.type;
			c->position=tok->position;
			if( tok->param.value.function){
				c->param.value.function=strdup(tok->param.value.function);
			}
		}else if( tok->param.type==T_IFSTATEMENT){
			c->param.type=tok->param.type;
			c->position=tok->position;
			c->children=exp_token_dup( tok->children);

		}else{
			memcpy( c, tok, sizeof( token_t));
		}
		c->next=NULL;

		if(ret_tail){
			ret_tail->next=c;
			ret_tail=c;
		}else{
			ret=ret_tail=c;
		}
		tok=tok->next;
	}
	return ret;
}


#ifdef EXP_DEBUG


static char *operator_to_string( operator_t op, char *out){
	switch(op){
		case O_IFTHEN:         strcpy(out, "?");  return out;
		case O_ELSE:       strcpy(out, ":");  return out;

		case O_BOOLNOT:    strcpy(out, "!");  return out;
		case O_BITNOT:     strcpy(out, "~");  return out;
		case O_DIV:        strcpy(out, "/");  return out;
		case O_MOD:        strcpy(out, "%");  return out;
		case O_MUL:        strcpy(out, "*");  return out;
		case O_UPLUS:      strcpy(out, "+-");  return out;
		case O_PLUS:       strcpy(out, "+");  return out;
		case O_UMINUS:     strcpy(out, "+-");  return out;
		case O_MINUS:      strcpy(out, "-");  return out;
		case O_SHIFTLEFT:  strcpy(out, "<<"); return out;
		case O_SHIFTRIGHT: strcpy(out, ">>"); return out;
		case O_GT:         strcpy(out, ">");  return out;
		case O_LT:         strcpy(out, "<");  return out;
		case O_GE:         strcpy(out, ">="); return out;
		case O_LE:         strcpy(out, "<="); return out;
		case O_NOTEQUALS:  strcpy(out, "!="); return out;
		case O_BOOLEQUALS: strcpy(out, "=="); return out;
		case O_BITAND:     strcpy(out, "&");  return out;
		case O_HAT:        strcpy(out, "^");  return out;
		case O_BITOR:      strcpy(out, "|");  return out;
		case O_BOOLAND:    strcpy(out, "&&"); return out;
		case O_BOOLOR:     strcpy(out, "||"); return out;
		case O_EQUALS:     strcpy(out, "=");  return out;
		default:           return NULL;
	}
}

//#define VERBOSE_PRINT
#ifdef VERBOSE_PRINT
void token_print( char *msg, token_t *token){
	token_t *t=token;
	char op[16];

	if(msg){
		printf("%s", msg);
	}
	while(t){
		if(t->param.type==T_REAL){
			printf("[real]%f ", t->param.value.real);
		}else if(t->param.type==T_INTEGER){
			printf("[int]%lld ", t->param.value.integer);
		}else if(t->param.type==T_BOOLEAN){
			printf("[bool]%s ", t->param.value.boolean? "true" : "false");
		}else if(t->param.type==T_PARAMETER){
			printf("[param]%s ", t->param.value.parameter? t->param.value.parameter : "NULL");
		}else if(t->param.type==T_OPERATOR){
			printf("[op]%s ", operator_to_string( t->param.value.operator, op));
		}else if(t->param.type==T_LPAREN){
			printf("( ");
		}else if(t->param.type==T_RPAREN){
			printf(") ");
		}else if(t->param.type==T_COMMA){
			printf(", ");
		}else if(t->param.type==T_FUNCTION){
			printf("[func]%s() ", t->param.value.function);
		}else{
			printf("[unknown token %d] ", t->param.type);
		}
		t=t->next;
	}
	printf("\n");
}
#else
void token_print( char *msg, token_t *token, int recursion){
	token_t *t=token;
	char op[16];

	if(msg){
		printf("%s", msg);
	}
	while(t){
		if(t->param.type==T_REAL){
			printf("%f ", t->param.value.real);
		}else if(t->param.type==T_INTEGER){
			printf("%lld ", t->param.value.integer);
		}else if(t->param.type==T_BOOLEAN){
			printf("%s ", t->param.value.boolean? "true" : "false");
		}else if(t->param.type==T_STRING){
			printf("'%s' ", t->param.value.string);
		}else if(t->param.type==T_PARAMETER){
			printf("%s ", t->param.value.parameter? t->param.value.parameter : "NULL");
		}else if(t->param.type==T_OPERATOR){
			printf("%s ", operator_to_string( t->param.value.operator, op));
		}else if(t->param.type==T_LPAREN){
			printf("( ");
		}else if(t->param.type==T_RPAREN){
			printf(") ");
		}else if(t->param.type==T_COMMA){
			printf(", ");
		}else if(t->param.type==T_FUNCTION){
			printf("%s() ", t->param.value.function);

		}else if( t->param.type==T_IFCONDITION){
			printf("?");
		}else if( t->param.type==T_IFSTATEMENT){
			printf("{");
			token_print( "", t->children, recursion+1);
			printf("}");

		}else{
			printf("[unknown token %d] ", t->param.type);
		}

		t=t->next;
	}
	if( 0==recursion) printf("\n");
}
#endif
#endif//EXP_DEBUG






static void strtolower( char *str){
	while( *str){
		*str=tolower(*str);
		str++;
	}
}

static token_t *is_parenthesis( char *s, size_t slen, int *toklen){
	if(slen){
		token_type_t type;
		token_t *ret;

		switch( *s){
			case '(': type=T_LPAREN; break;
			case ')': type=T_RPAREN; break;
			default: return NULL;
		}
		*toklen=1;
		if((ret=calloc(1, sizeof( token_t)))){
			ret->param.type=type;
		}
		return ret;
	}
	return NULL;
}

static token_t *is_comma( char *s, size_t slen, int *toklen){
	if(slen){
		token_type_t type;
		token_t *ret;

		switch( *s){
			case ',': type=T_COMMA; break;
			default: return NULL;
		}
		*toklen=1;
		if((ret=calloc(1, sizeof( token_t)))){
			ret->param.type=type;
		}
		return ret;
	}
	return NULL;
}

static token_t *is_operator( char *s, size_t slen, int *toklen){
	if(slen){
		int rlen;
		token_t *ret;
		operator_t value;

		switch( *s){
			case '+': value=O_PLUS;   rlen=1; break;
			case '-': value=O_MINUS;  rlen=1; break;
			case '/': value=O_DIV;    rlen=1; break;
			case '%': value=O_MOD;    rlen=1; break;
			case '*': value=O_MUL;    rlen=1; break;
			case '^': value=O_HAT;    rlen=1; break;
			case '~': value=O_BITNOT; rlen=1; break;

			case '?': value=O_IFTHEN; rlen=1; break;
			case ':': value=O_ELSE;   rlen=1; break;

			case '>':
				if(slen>1 && (*(s+1)=='=')){
					value=O_GE;
					rlen=2;
				}else if(slen>1 && (*(s+1)=='>')){
					value=O_SHIFTRIGHT;
					rlen=2;
				}else{
					value=O_GT;
					rlen=1;
				}
				break;
			case '<':
				if(slen>1 && (*(s+1)=='=')){
					value=O_LE;
					rlen=2;
				}else if(slen>1 && (*(s+1)=='<')){
					value=O_SHIFTLEFT;
					rlen=2;
				}else{
					value=O_LT;
					rlen=1;
				}
				break;
			case '=':
				if(slen>1 && (*(s+1)=='=')){
					value=O_BOOLEQUALS;
					rlen=2;
				}else{
					value=O_EQUALS;
					rlen=1;
				}
				break;
			case  '!':
				if(slen>1 && (*(s+1)=='=')){
					rlen=2;
					value=O_NOTEQUALS;
				}else{
					rlen=1;
					value=O_BOOLNOT;
				}
				break;
			case  '&':
				if(slen>1 && (*(s+1)=='&')){
					rlen=2;
					value=O_BOOLAND;
				}else{
					rlen=1;
					value=O_BITAND;
				}
				break;
			case '|':
				if(slen>1 && (*(s+1)=='|')){
					rlen=2;
					value=O_BOOLOR;
				}else{
					rlen=1;
					value=O_BITOR;
				}
				break;

			default:
				return NULL;
		}
		if((ret=calloc(1, sizeof( token_t)))){
			*toklen=rlen;
			ret->param.type=T_OPERATOR;
			ret->param.value.operator=value;
		}
		return ret;
	}else{
		return NULL;
	}
}

static token_t *is_number( char *s, size_t slen, int *toklen){
	int rlen;
	token_t *ret;
	int c;

	if(slen>=3 && (0==strncmp("0x", s, 2)) && (-1 !=( c=IS_HEXADEMICAL(s[2])))){
		uint64_t num;

		//hexademical value
		rlen=3;
		num=c;
		s+=3;
		while( -1 !=(c=IS_HEXADEMICAL( *s))){
			num=(num<<4)|c;
			s++;
			rlen++;
		}
		if((ret=calloc(1, sizeof( token_t)))){
			*toklen=rlen;
			ret->param.type=T_INTEGER;
			ret->param.value.integer=num;
		}
		return ret;

	}else if(slen>=3 && (0==strncmp("0o", s, 2)) && (-1 !=( c=IS_OCTAL(s[2])))){
		uint64_t num;

		//octal value
		rlen=3;
		num=c;
		s+=3;
		while( -1 !=(c=IS_OCTAL( *s))){
			num=(num<<3)|c;
			s++;
			rlen++;
		}
		if((ret=calloc(1, sizeof( token_t)))){
			*toklen=rlen;
			ret->param.type=T_INTEGER;
			ret->param.value.integer=num;
		}
		return ret;

	}else if(slen>=3 && (0==strncmp("0b", s, 2)) && (-1 !=( c=IS_BINARY(s[2])))){
		uint64_t num;

		//binary value
		rlen=3;
		num=c;
		s+=3;
		while( -1 !=(c=IS_BINARY( *s))){
			num=(num<<1) | c;
			s++;
			rlen++;
		}
		if((ret=calloc(1, sizeof( token_t)))){
			*toklen=rlen;
			ret->param.type=T_INTEGER;
			ret->param.value.integer=num;
		}
		return ret;

	}else{
		//test if value is real or integer
		char *p=s;
		int is_real=0;
		int efound=0;
		int pointfound=0;

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
			return NULL;

		}else{
			//ok, it's a number
			if((ret=calloc(1, sizeof( token_t)))){
				char buf[rlen+1];

				*toklen=rlen;
				strncpy(buf, s, rlen);
				buf[rlen]=0;
				if(is_real){
					ret->param.type=T_REAL;
					ret->param.value.real=atof( buf);
				}else{
					ret->param.type=T_INTEGER;
					ret->param.value.integer=atoll( buf);
				}
			}
			return ret;
		}
	}
}

static token_t *is_parameter( char *s, size_t slen, int *toklen){
	char *p=s;
	token_t *ret;

	if(IS_ALPHA( *p)){
		p++;
		while( IS_ALPHANUMERIC(*p) || *p=='.') p++;
		if((ret=calloc(1, sizeof( token_t)))){
			if(NULL==(ret->param.value.parameter=strdupn( s, p-s))){
				free(ret);
				ret=NULL;
			}else{
				strtolower( ret->param.value.parameter);
				*toklen=p-s;
				ret->param.type=T_PARAMETER;
			}
		}
		return ret;
	}
	return NULL;
}

static token_t *is_string( char *s, size_t slen, int *toklen){
	char *p=s;
	token_t *ret;
	char c;
	int slash;

	if( '\''==*p || '"'==*p){
		c=*p;

		p++;
		slash=0;
		while( *p){
			if('\\'==*p){
				slash= slash? 0 : 1;
			}else if((c==*p) && (0==slash)){
				//this is the end of the string
				break;
			}else{
				slash=0;
			}
			p++;
		}

		if(c==*p){
			//string found
			if((ret=calloc(1, sizeof( token_t)))){
				if(NULL==(ret->param.value.string=strdupn( s+1, p-s-1))){
					free(ret);
					ret=NULL;
				}else{
					//try removing slashes
					stripslashes(ret->param.value.string);
					*toklen=p-s+1;
					ret->param.type=T_STRING;
				}
			}
			return ret;
		}
	}
	return NULL;
}


int exp_check( token_t *token, exp_error_t *ercode, char *error, int *error_pos){
	token_t *curr, *prev, *next;

	prev=NULL;
	curr=token;

	while(curr){
		next=curr->next;

		//check if parameter is really a function
		if(curr->param.type==T_PARAMETER){
			if(next && next->param.type==T_LPAREN){
				curr->param.type=T_FUNCTION;
				curr->param.value.function=curr->param.value.parameter;
			}
		}

		//check for missing or unexpected parentheses
		if(curr->param.type==T_LPAREN){
			if( !( NULL==prev || prev->param.type==T_LPAREN ||prev->param.type==T_COMMA ||prev->param.type==T_OPERATOR || prev->param.type==T_FUNCTION)){
				*error_pos=curr->position;
				*ercode=EXP_ER_INVALEXPR;
				strcpy(error, "Unexpected left parenthesis");
				return -1;

			}else if( NULL !=curr->next && (curr->next->param.type==T_COMMA || (curr->next->param.type==T_RPAREN && (NULL==prev || (NULL!=prev && prev->param.type !=T_FUNCTION))))){
				*error_pos=curr->position;
				*ercode=EXP_ER_INVALEXPR;
				strcpy(error, "Parenthesis without operand on the right");
				return -1;
			}
		}else if(curr->param.type==T_RPAREN){
			if( !( NULL==curr->next || curr->next->param.type==T_RPAREN|| curr->next->param.type==T_COMMA ||curr->next->param.type==T_OPERATOR )){
				*error_pos=curr->position;
				*ercode=EXP_ER_INVALEXPR;
				strcpy(error, "Parenthesis without operator on the right");
				return -1;
			}else if( NULL !=prev && (prev->param.type==T_COMMA)){
				*error_pos=curr->position;
				*ercode=EXP_ER_INVALEXPR;
				strcpy(error, "Unexpected right parenthesis");
				return -1;
			}
		}

		//check for unexpected operands
		if(curr->param.type==T_BOOLEAN ||curr->param.type==T_INTEGER ||
				curr->param.type==T_REAL ||curr->param.type==T_STRING ||curr->param.type==T_PARAMETER){
			if( !(prev==NULL || prev->param.type==T_OPERATOR ||
					prev->param.type==T_LPAREN || prev->param.type==T_COMMA)){
				*error_pos=curr->position;
				*ercode=EXP_ER_INVALEXPR;
				strcpy(error, "Unexpected operand");
				return -1;
			}else if(!(curr->next==NULL || curr->next->param.type==T_OPERATOR ||
					curr->next->param.type==T_RPAREN || curr->next->param.type==T_COMMA)){
				*error_pos=curr->position;
				*ercode=EXP_ER_INVALEXPR;
				strcpy(error, "Operand without operator on the right");
				return -1;
			}
		}

		//check for unexpected function
		if(curr->param.type==T_FUNCTION){
			if( !(prev==NULL || prev->param.type==T_OPERATOR || prev->param.type==T_LPAREN ||
					prev->param.type==T_COMMA)){
				*error_pos=curr->position;
				*ercode=EXP_ER_INVALEXPR;
				strcpy(error, "Unexpected function");
				return -1;
			}
		}

		//check for unexpected operators
		if( curr->param.type==T_OPERATOR){
			if( NULL==prev || prev->param.type==T_LPAREN
					|| prev->param.type==T_COMMA || prev->param.type==T_OPERATOR){
				if( exp_op_is_lefttoright( curr->param.value.operator)
						|| (exp_op_argument_count( curr->param.value.operator) !=1) ){
					//operator in invalid position
					*error_pos=curr->position;
					*ercode=EXP_ER_INVALEXPR;
					strcpy(error, "Unexpected operator");
					return -1;
				}
			}
			if(NULL==next || next->param.type==T_RPAREN
					|| next->param.type==T_COMMA){
				//operator in invalid position
				*error_pos=curr->position;
				*ercode=EXP_ER_INVALEXPR;
				strcpy(error, "Operator without right operand");
				return -1;
			}
		}

		prev=curr;
		curr=curr->next;
	}
	return 0;
}


/*
 * Parses expr and explodes it on tokens. Function returns linked list of tokens
 * if expr is parsed successfully. Function returns NULL if error occured
 * and sets error to specific error  string.
 */
token_t *exp_parse( char *expression, exp_error_t *ercode, char *error, int *error_pos){
	int elen, l;
	char *exp;
	token_t *token, *head=NULL, *tail=NULL;

	exp=expression;
	//Trim left
	while(IS_BLANK( *exp)) exp++;

	while(IS_BLANK( *exp)) exp++;
	elen=strlen( exp);
	if(0==elen){
		*ercode=EXP_ER_INVALEXPR;
		strcpy(error, "Empty expression was provided");
		*error_pos=0;
		return NULL;

	}else{
		while( elen>0){
			if(NULL==(token=is_parenthesis( exp, elen, &l))){
				if(NULL==(token=is_comma( exp, elen, &l))){
					if(NULL==(token=is_operator( exp, elen, &l))){
						if(NULL==(token=is_number(exp, elen, &l))){
							if(NULL==(token=is_parameter(exp, elen, &l))){
								if(NULL==(token=is_string(exp, elen, &l))){
									if( *exp=='"' || *exp=='\''){
										*ercode=EXP_ER_INVALEXPR;
										strcpy(error, "Missing terminating quote character");
									}else{
										*ercode=EXP_ER_INVALEXPR;
										sprintf(error, "Invalid or unsupported token '%c'", *exp);
									}
									*error_pos=exp-expression;
									head=exp_token_free( head);
									break;
								}
							}
						}
					}
				}
			}
			token->position=exp-expression;

			if(token->param.type==T_OPERATOR && token->param.value.operator==O_MINUS && ((tail==NULL) ||
					(tail && (tail->param.type==T_COMMA || tail->param.type==T_LPAREN || tail->param.type==T_OPERATOR)))){
				token->param.value.operator=O_UMINUS;
			}else if(token->param.type==T_OPERATOR && token->param.value.operator==O_PLUS && ((tail==NULL) ||
					(tail && (tail->param.type==T_COMMA || tail->param.type==T_LPAREN || tail->param.type==T_OPERATOR)))){
				token->param.value.operator=O_UPLUS;
			}

			if(tail){
				tail->next=token;
				tail=token;
			}else{
				head=tail=token;
			}

			exp+=l;
			elen-=l;

			while(IS_BLANK( *exp)){
				exp++;
				elen--;
			}
		}
		return head;
	}
}



