/*****************************************************************************
*
* MODULE:       SQL statement parser library 
*   	    	
* AUTHOR(S):    lex.l and yac.y were originaly taken from unixODBC and
*               probably written by Peter Harvey <pharvey@codebydesigns.com>,
*               modifications and other code by Radim Blazek
*
* PURPOSE:      Parse input string containing SQL statement to 
*               SQLPSTMT structure.
*               SQL parser may be used by simple database drivers. 
*
* COPYRIGHT:    (C) 2000 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/

%{
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sqlp.h"

#define YYDEBUG 1

%}
	
	/* symbolic tokens */

%union {
	int intval;
	double floatval;
	char *strval;
	int subtok;
	Node				*node;
}

	/* operators */
%type <node>	y_comparison
%type <node>	y_condition
%type <node>	y_attr
%type <node>	y_attr2
%type <node>	y_name
%type <node>	y_cnam

	/* literal keyword tokens */
%token <strval> COMPARISON
%token <strval> NAME
%token <strval> STRING
%token <intval> INTNUM 
%token <floatval> FLOATNUM

%token ADD
%token COLUMN
%token EQUAL
%token SELECT FROM WHERE
/* %token SELECT DISTINCT FROM WHERE */
%token DELETE
%token INSERT INTO VALUES
%token UPDATE SET
%token AND
%token OR
%token NOT
%token ALTER TABLE
%token CREATE TABLE
%token DROP TABLE
%token NULL_VALUE
%token VARCHAR
%token INT
%token INTEGER
%token DOUBLE
%token PRECISION
%token DATE
%token ORDER BY

%{
 
extern int yylex(void);

%}

%%

y_sql:	
		y_alter
	|	y_create
	|	y_drop
	|	y_insert
	|	y_select
	|	y_update
	|	y_delete
	;
	
y_alter:
		ALTER TABLE y_table ADD COLUMN y_columndef	{ sqpCommand(SQLP_ADD_COLUMN); }
	|	ALTER TABLE y_table ADD y_columndef		{ sqpCommand(SQLP_ADD_COLUMN); }
	;
	
y_create:
		CREATE TABLE y_table '(' y_columndefs ')'	{ sqpCommand(SQLP_CREATE); }
	;
	
y_drop:
		DROP TABLE y_table				{ sqpCommand(SQLP_DROP); }
	;

y_select:
		SELECT y_columns FROM y_table			{ sqpCommand(SQLP_SELECT); }
	|	SELECT y_columns FROM y_table WHERE y_condition	{ sqpCommand(SQLP_SELECT); }
	|	SELECT y_columns FROM y_table ORDER BY y_order	{ sqpCommand(SQLP_SELECT); }
	|	SELECT y_columns FROM y_table WHERE y_condition ORDER BY y_order { sqpCommand(SQLP_SELECT); }
/*	|	SELECT DISTINCT y_columns FROM y_table		{ sqpCommand(SQLP_SELECT); }
 *	|	SELECT DISTINCT y_columns FROM y_table WHERE y_condition	{ sqpCommand(SQLP_SELECT); }
 */
	;
	
y_delete:
		DELETE FROM y_table				{ sqpCommand(SQLP_DELETE); }
        |	DELETE FROM y_table WHERE y_condition		{ sqpCommand(SQLP_DELETE); }
	;

y_insert:
		INSERT INTO y_table y_values			{ sqpCommand(SQLP_INSERT); }
        |	INSERT INTO y_table '(' y_columns ')' y_values	{ sqpCommand(SQLP_INSERT); }
	;

y_update:
		UPDATE y_table SET y_assignments		{ sqpCommand(SQLP_UPDATE); }
	|	UPDATE y_table SET y_assignments WHERE y_condition	{ sqpCommand(SQLP_UPDATE); }

	;
	
y_columndefs:
		y_columndef
	|	y_columndefs ',' y_columndef
	;

y_columndef:
		NAME VARCHAR '(' INTNUM ')'	{ sqpColumnDef( $1, SQLP_VARCHAR, $4, 0 ); }
	|	NAME INT 			{ sqpColumnDef( $1, SQLP_INTEGER,  0, 0 ); }
	|	NAME INTEGER 			{ sqpColumnDef( $1, SQLP_INTEGER,  0, 0 ); }
	|	NAME DOUBLE			{ sqpColumnDef( $1, SQLP_DOUBLE,   0, 0 ); }
	|	NAME DOUBLE PRECISION		{ sqpColumnDef( $1, SQLP_DOUBLE,   0, 0 ); }
	|	NAME DATE			{ sqpColumnDef( $1, SQLP_DATE,     0, 0 ); }
	;

y_columns:
	'*'
        |	y_column_list
	;
	
y_column_list:
		NAME				{ sqpColumn( $1 ); }
	|	y_column_list ',' NAME		{ sqpColumn( $3 ); }
	;

y_table:
		NAME 				{ sqpTable( $1 ); }
	;
	
y_values:
		VALUES '(' y_value_list ')'
	;

y_value_list:
		NULL_VALUE			{ sqpValue( NULL,  0, 0.0, 1, SQLP_UNKNOWN ); }
	|	STRING				{ sqpValue( $1,    0, 0.0, 0, SQLP_S ); }
        |	INTNUM				{ sqpValue( NULL, $1, 0.0, 0, SQLP_I ); }
	|	FLOATNUM			{ sqpValue( NULL,  0,  $1, 0, SQLP_D ); }
	|	y_value_list ',' NULL_VALUE	{ sqpValue( NULL,  0, 0.0, 1, SQLP_UNKNOWN ); }
	|	y_value_list ',' STRING		{ sqpValue( $3,    0, 0.0, 0, SQLP_S ); }
	|	y_value_list ',' INTNUM		{ sqpValue( NULL, $3, 0.0, 0, SQLP_I ); }
	|	y_value_list ',' FLOATNUM	{ sqpValue( NULL,  0,  $3, 0, SQLP_D ); }
	;

y_assignments:
		y_assignment
	|	y_assignments ',' y_assignment
	;
	
y_assignment:
		NAME EQUAL NULL_VALUE	{ sqpAssignment( $1, NULL,  0, 0.0, 1, SQLP_UNKNOWN ); }
	|	NAME EQUAL STRING	{ sqpAssignment( $1,   $3,  0, 0.0, 0, SQLP_S ); }
        |	NAME EQUAL INTNUM	{ sqpAssignment( $1, NULL, $3, 0.0, 0, SQLP_I ); }
        |	NAME EQUAL FLOATNUM	{ sqpAssignment( $1, NULL,  0,  $3, 0, SQLP_D ); }
	;

y_condition:			
		y_attr		{$$ = $1;}	
	|	y_condition AND y_attr {
			$$ = makeA_Expr(SQLP_AND, 0, $1, $3);
			    sqlpStmt->upperNodeptr = $$; 
		}
	|	y_condition OR y_attr {
			$$ = makeA_Expr(SQLP_OR, 0, $1, $3);
			    sqlpStmt->upperNodeptr = $$; 
		}
	;

y_attr:
		y_attr2		{$$ = $1;}	

	| 	'(' y_condition ')'
				{$$ = $2;}
	;
y_attr2:
		y_comparison	{$$ = $1;}
	|	NOT y_attr {
			$$ = makeA_Expr(SQLP_NOT, 0, NULL, $2); 
			    sqlpStmt->upperNodeptr = $$; 
		}
	;
y_comparison:
		y_name EQUAL y_name {
				$$ = makeA_Expr(SQLP_OP, SQLP_EQ, $1, $3);
					sqlpStmt->upperNodeptr = $$; 
		}
	|	y_name COMPARISON y_name {
				$$ = makeA_Expr(SQLP_OP, translate_Operator($2), $1, $3);
					sqlpStmt->upperNodeptr = $$; 
		}
	;	
y_name:
		y_cnam 				{$$ = makeArithmExpr(SQLP_ADD, $1, NULL);}
	|	y_name '-' y_cnam		{$$ = makeArithmExpr(SQLP_SUBTR, $1, $3);}
	|	y_name '+' y_cnam		{$$ = makeArithmExpr(SQLP_ADD, $1, $3);}
	|	y_name '*' y_cnam		{$$ = makeArithmExpr(SQLP_MLTP, $1, $3);}
	|	y_name '/' y_cnam		{$$ = makeArithmExpr(SQLP_DIV, $1, $3);}
	|	y_name INTNUM			{$$ = makeArithmExpr(SQLP_ADD, $1, makeArithmValue(NULL,$2,0.0,SQLP_I,1));}
	|	y_name FLOATNUM			{$$ = makeArithmExpr(SQLP_ADD, $1, makeArithmValue(NULL,0,$2,SQLP_D,1));}
	;
y_cnam:
		NAME				{$$ = makeArithmValue(  $1, 0,0.0,SQLP_COL,1);}
	|	STRING				{$$ = makeArithmValue(  $1, 0,0.0,  SQLP_S,1);}
	|	INTNUM				{$$ = makeArithmValue(NULL,$1,0.0,  SQLP_I,1);}
	|	FLOATNUM			{$$ = makeArithmValue(NULL, 0, $1,  SQLP_D,1);}
	|	'(' y_name ')'			{$$ = $2;}
	;

y_order:
		NAME 				{ sqpOrderColumn( $1 ); }
	;
%%


