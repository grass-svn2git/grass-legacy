
%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <unistd.h>

#include "mapcalc.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

%}

%union {
	int ival;
	double fval;
	char *sval;
	expression *exp;
	expr_list *list;
}

%token <sval> VARNAME
%token <sval> NAME
%token <sval> VARSTRING
%token <sval> STRING
%token <ival> INTEGER
%token <fval> FLOAT
%token <fval> DOUBLE

%token GT GE LT LE EQ NE AND OR

%type <exp> exp
%type <exp> exp_atom
%type <exp> exp_neg
%type <exp> exp_pow
%type <exp> exp_mul
%type <exp> exp_add
%type <exp> exp_cmp
%type <exp> exp_log
%type <exp> exp_cond
%type <exp> exp_let

%type <exp> atom_var
%type <exp> atom_map
%type <exp> atom_func

%type <exp> def

%type <ival> mapmod
%type <ival> index

%type <sval> name
%type <sval> map

%type <list> expr_list

%type <list> defs
%type <list> program

%{

static expr_list *result;

extern int yylex(void);

int yyparse(void);
void yyerror(char *s);

%}

%%

program		: defs			{ $$ = result = $1;		}
		;

defs		: def			{ $$ = list($1,NULL);		}
		| def ';'		{ $$ = list($1,NULL);		}
		| def ';' defs		{ $$ = list($1,$3);		}
		;

def		: STRING '=' exp	{ $$ = binding($1,$3); define_variable($$);	}
		| NAME '=' exp		{ $$ = binding($1,$3); define_variable($$);	}
		;

map		: STRING
		| NAME
		| name '@' name		{ $$ = composite($1,$3);	}
		;

mapmod		: '@'			{ $$ = '@';			}
		| 'r'			{ $$ = 'r';			}
		| 'g'			{ $$ = 'g';			}
		| 'b'			{ $$ = 'b';			}
		| '#'			{ $$ = '#';			}
		| 'y'			{ $$ = 'y';			}
		| 'i'			{ $$ = 'i';			}
		;

index		: INTEGER
		| '-' INTEGER		{ $$ = -$2;			}
		;

expr_list	: exp			{ $$ = singleton($1);		}
		| exp ',' expr_list	{ $$ = list($1, $3);		}
		;

atom_var	: VARSTRING		{ $$ = variable($1);		}
		| VARNAME		{ $$ = variable($1);		}
		;

atom_map	: map '[' index ']'	{ $$ = mapname($1,'M',$3,0);	}
		| map '[' index ',' index ']'
					{ $$ = mapname($1,'M',$3,$5);	}
		| map			{ $$ = mapname($1,'M',0,0);	}
		| mapmod map '[' index ']'
					{ $$ = mapname($2,$1,$4,0);	}
		| mapmod map '[' index ',' index ']'
					{ $$ = mapname($2,$1,$4,$6);	}
		| mapmod map		{ $$ = mapname($2,$1,0,0);	}
		;

atom_func	: name '(' ')'		{ $$ = function($1, NULL);	}
		| name '(' expr_list ')'
					{ $$ = function($1, $3);	}
		;

exp_atom	: '(' exp ')'		{ $$ = $2;			}
		| atom_var
		| atom_map
		| atom_func
		| INTEGER		{ $$ = constant_int($1);	}
		| FLOAT			{ $$ = constant_float($1);	}
		| DOUBLE		{ $$ = constant_double($1);	}
		;

exp_neg		: exp_atom
		| '-' exp_atom		{ $$ = operator("neg","-",1,singleton($2));	}
		;

exp_pow		: exp_neg
		| exp_neg '^' exp_pow	{ $$ = operator("pow","^",2,pair($1,$3));	}
		;

exp_mul		: exp_pow
		| exp_mul '*' exp_pow	{ $$ = operator("mul","*",3,pair($1,$3));	}
		| exp_mul '/' exp_pow	{ $$ = operator("div","/",3,pair($1,$3));	}
		| exp_mul '%' exp_pow	{ $$ = operator("mod","%",3,pair($1,$3));	}
		;

exp_add		: exp_mul
		| exp_add '+' exp_mul	{ $$ = operator("add","+",4,pair($1,$3));	}
		| exp_add '-' exp_mul	{ $$ = operator("sub","-",4,pair($1,$3));	}
		;

exp_cmp		: exp_add
		| exp_cmp GT exp_add	{ $$ = operator("gt",">", 5,pair($1,$3));	}
		| exp_cmp GE exp_add	{ $$ = operator("ge",">=",5,pair($1,$3));	}
		| exp_cmp LT exp_add	{ $$ = operator("lt","<", 5,pair($1,$3));	}
		| exp_cmp LE exp_add	{ $$ = operator("le","<=",5,pair($1,$3));	}
		| exp_cmp EQ exp_add	{ $$ = operator("eq","==",5,pair($1,$3));	}
		| exp_cmp NE exp_add	{ $$ = operator("ne","!=",5,pair($1,$3));	}
		;

exp_log		: exp_cmp
		| exp_log OR exp_cmp	{ $$ = operator("or", "||",6,pair($1,$3));	}
		| exp_log AND exp_cmp	{ $$ = operator("and","&&",6,pair($1,$3));	}
		;

exp_cond	: exp_log
		| exp_log '?' exp_cond ':' exp_cond
					{ $$ = operator("if","?:",7,triple($1,$3,$5));	}
		;

exp_let		: exp_cond
		| name '=' exp_let	{ $$ = binding($1,$3); define_variable($$);	}
		;

exp		: exp_let
		;

name		: NAME
		| VARNAME
		;

%%

static int syntax_error_occurred;

void syntax_error(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);

	fprintf(stderr, "\n");

	syntax_error_occurred = 1;
}

void yyerror(char *s)
{
	fprintf(stderr, "%s\n", s);
}

static expr_list *parse(void)
{
#if 0
	yydebug = 1;
#endif
	syntax_error_occurred = 0;

	if (yyparse() != 0)
	{
		fprintf(stderr, "Parse error\n");
		return NULL;
	}

	if (syntax_error_occurred)
	{
		fprintf(stderr, "Syntax error\n");
		return NULL;
	}

	return result;
}

expr_list *parse_string(const char *s)
{
	initialize_scanner_string(s);
	return parse();
}

expr_list *parse_stream(FILE *fp)
{
	expr_list *e;

	initialize_scanner_stream(fp);
	if (isatty(fileno(fp)))
		fputs("Enter expressions, \"end\" when done.\n", stderr);
	e = parse();
	if (isatty(fileno(fp)))
		fputs("\n", stderr);
	return e;
}

